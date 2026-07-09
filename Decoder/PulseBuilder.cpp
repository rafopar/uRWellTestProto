#include "PulseBuilder.h"

#include <TF1.h>
#include <TMath.h>
#include <TGraphErrors.h>
#include <TError.h>

#include <cstdio>
#include <fstream>
#include <map>
#include <vector>

// Constants copied verbatim from PulseFitWriter.cpp / Skim_PulseFit.cc so the
// behaviour is the same as the decoder's --pulse path.
static const double sigm_threshold = 3.;   // ADC threshold in units of sigma
static const int    n_ts           = 15;   // number of time samples
static const int    sec_GEM        = 8;     // GEM lives in sector 8
static const int    sec_uRWell     = 6;     // uRwell lives in sector 6

PulseBuilder::~PulseBuilder() {
    delete fFit;
}

bool PulseBuilder::loadPed(const std::string &path,
                           std::unordered_map<int, double> &mean,
                           std::unordered_map<int, double> &rms)
{
    std::ifstream in(path);
    if (!in.is_open()) {
        fprintf(stderr, "error: cannot open pedestal file \"%s\"\n", path.c_str());
        return false;
    }
    int ch;
    double m, r;
    while (in >> ch >> m >> r) {
        mean[ch] = m;
        rms[ch]  = r;
    }
    return true;
}

void PulseBuilder::ensureFit() {
    if (fFit != nullptr) {
        return;
    }
    gErrorIgnoreLevel = kError;   // hush ROOT fit warnings (does not affect results)
    // Same fit function as PulseFitWriter.cpp / Skim_PulseFit.cc (bgr fixed to 0).
    fFit = new TF1("f_bgrPlusLandau_pb", "[0] + [1]*TMath::Landau(x,[2],[3])", -10., 10.);
    fFit->SetNpx(4500);
    fFit->FixParameter(0, 0.);
}

bool PulseBuilder::loadPedestals(int run, const std::string &pedDir) {
    char p1[512], p2[512];
    snprintf(p1, sizeof(p1), "%s/Peds_%d", pedDir.c_str(), run);
    snprintf(p2, sizeof(p2), "%s/GEM_Peds_%d", pedDir.c_str(), run);
    if (!loadPed(p1, fPedMean, fPedRms))       return false;
    if (!loadPed(p2, fPedGEMMean, fPedGEMRms)) return false;
    ensureFit();
    return true;
}

void PulseBuilder::build(const std::vector<DecodedAdcRow> &adc,
                         std::vector<uRwellTools::APV25Pulse> &uRwellPulses,
                         std::vector<uRwellTools::APV25Pulse> &gemPulses)
{
    uRwellPulses.clear();
    gemPulses.clear();
    if (adc.empty()) {
        return;
    }
    ensureFit();

    const int N = uRwellTools::nMaxUniqueChan + 1;

    std::map<int, double> m_ADC_uRWELL;
    std::map<int, double> m_ADC_GEM;
    std::map<int, int>    m_ts_GEM;
    std::map<int, double> m_MaxADC_GEM;

    std::vector<TGraphErrors> gr_uRwell(N);
    std::vector<TGraphErrors> gr_GEM(N);

    std::vector<double> uRWell_grMax(N, 0.);
    std::vector<double> uRWell_ts_grMax(N, 0.);

    // ---- Build the per-strip waveforms --------------------------------------
    for (const DecodedAdcRow &r : adc) {
        const int sector     = r.sector;
        const int layer      = r.layer;
        const int ADC        = r.adc;
        const int uniqueChan = 1000 * (layer - 1) + r.component;   // = URWELL::adc "time"
        const int ts         = r.ped;                              // = URWELL::adc "ped"

        if (sector == sec_GEM) {
            auto res = m_ADC_GEM.emplace(uniqueChan, 0.0);
            if (res.second) gr_GEM[uniqueChan].Set(n_ts);
            res.first->second += double(ADC);

            gr_GEM[uniqueChan].SetPoint(ts, ts, fPedGEMMean[uniqueChan] - ADC);
            gr_GEM[uniqueChan].SetPointError(ts, 0, fPedGEMRms[uniqueChan]);

            if (fPedGEMMean[uniqueChan] - ADC > m_MaxADC_GEM[uniqueChan]) {
                m_MaxADC_GEM[uniqueChan] = fPedGEMMean[uniqueChan] - ADC;
                m_ts_GEM[uniqueChan]     = ts;
            }
        } else if (sector == sec_uRWell) {
            auto res = m_ADC_uRWELL.emplace(uniqueChan, 0.0);
            if (res.second) gr_uRwell[uniqueChan].Set(n_ts);
            res.first->second += double(ADC);

            gr_uRwell[uniqueChan].SetPoint(ts, ts, fPedMean[uniqueChan] - ADC);
            gr_uRwell[uniqueChan].SetPointError(ts, 0, fPedRms[uniqueChan]);

            if (fPedMean[uniqueChan] - ADC > uRWell_grMax[uniqueChan]) {
                uRWell_grMax[uniqueChan]    = fPedMean[uniqueChan] - ADC;
                uRWell_ts_grMax[uniqueChan] = ts;
            }
        }
    }

    // ---- GEM pulses ---------------------------------------------------------
    for (auto it = m_ADC_GEM.begin(); it != m_ADC_GEM.end(); ++it) {
        int ch = it->first;
        m_ADC_GEM[ch] = fPedGEMMean[ch] - m_ADC_GEM[ch] / double(n_ts);
        double adcRel = m_ADC_GEM[ch] / fPedGEMRms[ch];

        if (adcRel > sigm_threshold) {
            uRwellTools::uRwellHit curHit;
            curHit.adc        = m_ADC_GEM[ch];
            curHit.adcRel     = adcRel;
            curHit.sector     = sec_GEM;
            curHit.layer      = 1 + ch / 1000;
            curHit.slot       = uRwellTools::getGEMSlot(ch);
            curHit.strip      = ch % 1000;
            curHit.stripLocal = ch - uRwellTools::slot_Offset[curHit.slot];
            curHit.ts         = m_ts_GEM[ch];

            uRwellTools::APV25Pulse curPulse;

            fFit->SetParameter(1, 4 * (gr_GEM[ch].GetMaximum() - gr_GEM[ch].GetMinimum()));
            fFit->SetParLimits(1, 0., 10000.);
            fFit->SetParameter(2, 3);
            fFit->SetParLimits(3, 0.4, 10.);

            gr_GEM[ch].Fit(fFit, "MeQ", "", -1.1, double(n_ts) + 0.1);

            curPulse.hit          = curHit;
            curPulse.ped_rms      = fPedGEMRms[ch];
            curPulse.pulse_p0     = fFit->GetParameter(0);
            curPulse.pulse_A0     = fFit->GetParameter(1);
            curPulse.pulse_MPV    = fFit->GetParameter(2);
            curPulse.pulse_Sigma  = fFit->GetParameter(3);
            curPulse.pulse_Chi2   = fFit->GetChisquare();
            curPulse.pulse_NDF    = fFit->GetNDF();
            curPulse.pulse_Integral = fFit->Integral(curPulse.pulse_MPV - 5 * curPulse.pulse_Sigma,
                                                     curPulse.pulse_MPV + 50. * curPulse.pulse_Sigma);

            for (int ts = 0; ts < n_ts; ++ts)
                curPulse.pulse_ADC[int(gr_GEM[ch].GetPointX(ts))] = gr_GEM[ch].GetPointY(ts);

            gemPulses.push_back(curPulse);
        }
    }

    // ---- uRwell pulses ------------------------------------------------------
    for (auto it = m_ADC_uRWELL.begin(); it != m_ADC_uRWELL.end(); ++it) {
        int ch = it->first;
        m_ADC_uRWELL[ch] = fPedMean[ch] - m_ADC_uRWELL[ch] / double(n_ts);
        double adcRel = m_ADC_uRWELL[ch] / fPedRms[ch];

        if (adcRel > sigm_threshold) {
            uRwellTools::uRwellHit curHit;
            curHit.adc        = m_ADC_uRWELL[ch];
            curHit.adcRel     = adcRel;
            curHit.sector     = sec_uRWell;
            curHit.layer      = 1 + ch / 1000;
            curHit.slot       = uRwellTools::getURwellSlot(ch);
            curHit.strip      = ch % 1000;
            curHit.stripLocal = ch - uRwellTools::slot_Offset[curHit.slot];
            curHit.ts         = uRWell_ts_grMax[ch];

            uRwellTools::APV25Pulse curPulse;

            fFit->SetParameter(1, 4 * (gr_uRwell[ch].GetMaximum() - gr_uRwell[ch].GetMinimum()));
            fFit->SetParLimits(1, 0., 10000.);
            fFit->SetParameter(2, uRWell_ts_grMax[ch]);
            fFit->SetParLimits(3, 0.4, 10.);

            gr_uRwell[ch].Fit(fFit, "MEQ", "", -1.1, double(n_ts) + 0.1);

            curPulse.hit          = curHit;
            curPulse.ped_rms      = fPedRms[ch];
            curPulse.pulse_p0     = fFit->GetParameter(0);
            curPulse.pulse_A0     = fFit->GetParameter(1);
            curPulse.pulse_MPV    = fFit->GetParameter(2);
            curPulse.pulse_Sigma  = fFit->GetParameter(3);
            curPulse.pulse_Chi2   = fFit->GetChisquare();
            curPulse.pulse_NDF    = fFit->GetNDF();
            curPulse.pulse_Integral = fFit->Integral(curPulse.pulse_MPV - 5 * curPulse.pulse_Sigma,
                                                     curPulse.pulse_MPV + 50. * curPulse.pulse_Sigma);

            for (int ts = 0; ts < n_ts; ++ts)
                curPulse.pulse_ADC[int(gr_uRwell[ch].GetPointX(ts))] = gr_uRwell[ch].GetPointY(ts);

            uRwellPulses.push_back(curPulse);
        }
    }
}
