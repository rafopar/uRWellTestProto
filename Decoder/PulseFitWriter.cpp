#include "PulseFitWriter.h"

#include <uRwellTools.h>      // getURwellSlot/getGEMSlot/slot_Offset, structs, nMaxUniqueChan

#include <TF1.h>
#include <TMath.h>
#include <TGraphErrors.h>
#include <TError.h>

#include <cstdio>
#include <fstream>
#include <map>
#include <vector>

// Constants copied verbatim from AnaCodes/Skim_PulseFit.cc so the behaviour is
// bit-for-bit the same.
static const double sigm_threshold = 3.;   // ADC threshold in units of sigma
static const int    n_ts          = 15;    // number of time samples
static const int    sec_GEM       = 8;     // GEM lives in sector 8
static const int    sec_uRWell    = 6;     // uRwell lives in sector 6

bool PulseFitWriter::loadPed(const std::string &path,
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

bool PulseFitWriter::open(const std::string &filename, int run,
                          const std::string &pedDir)
{
    // Pedestals: pedDir/Peds_<run> (uRwell) and pedDir/GEM_Peds_<run> (GEM),
    // exactly the two files Skim_PulseFit reads.
    char p1[512], p2[512];
    snprintf(p1, sizeof(p1), "%s/Peds_%d", pedDir.c_str(), run);
    snprintf(p2, sizeof(p2), "%s/GEM_Peds_%d", pedDir.c_str(), run);
    if (!loadPed(p1, fPedMean, fPedRms))       return false;
    if (!loadPed(p2, fPedGEMMean, fPedGEMRms)) return false;

    gErrorIgnoreLevel = kError;   // hush ROOT fit warnings (does not affect results)

    // uRwell::Pulse schema, identical to Skim_PulseFit.cc:58.
    fSchPulse = hipo::schema("uRwell::Pulse", 90, 1);
    fSchPulse.parse(
        "sec/S,layer/S,strip/S,stripLocal/S,adc/F,adcRel/F,ts/S,slot/S,ped_rms/F,"
        "pulse_p0/F,pulse_A0/F,pulse_MPV/F,pulse_Sigma/F,pulse_Chi2/F,pulse_NDF/S,"
        "pulse_Integral/F,pulse_ADC0/F,pulse_ADC1/F,pulse_ADC2/F,pulse_ADC3/F,"
        "pulse_ADC4/F,pulse_ADC5/F,pulse_ADC6/F,pulse_ADC7/F,pulse_ADC8/F,"
        "pulse_ADC9/F,pulse_ADC10/F,pulse_ADC11/F,pulse_ADC12/F,pulse_ADC13/F,pulse_ADC14/F");

    // Pass-through banks (RAW::adc is always empty, just like in the skim where
    // the decoded input carries an empty RAW::adc).
    fSchRaw = hipo::schema("RAW::adc", 20000, 11);
    fSchRaw.parse("crate/B,slot/B,channel/S,order/B,ADC/I,time/F,ped/S");

    fSchCfg = hipo::schema("RUN::config", 10000, 11);
    fSchCfg.parse("run/I,event/I,unixtime/I,trigger/L,timestamp/L,type/B,mode/B,torus/F,solenoid/F");

    fSchHodo = hipo::schema("XYHODO::tdc", 21850, 13);
    fSchHodo.parse("sector/B,layer/B,component/S,order/B,TDC/I");

    // Same fit function as Skim_PulseFit.cc:63 (background fixed to 0).
    fFit = new TF1("f_bgrPlusLandau", "[0] + [1]*TMath::Landau(x,[2],[3])", -10., 10.);
    fFit->SetNpx(4500);
    fFit->FixParameter(0, 0.);

    fWriter.getDictionary().addSchema(fSchPulse);
    fWriter.getDictionary().addSchema(fSchRaw);
    fWriter.getDictionary().addSchema(fSchCfg);
    fWriter.getDictionary().addSchema(fSchHodo);
    fWriter.open(filename.c_str());
    return true;
}

void PulseFitWriter::writeEvent(int run, int event, int unixtime,
                                const std::vector<AdcRow> &adc,
                                const std::vector<TdcRow> &tdc)
{
    // Skim_PulseFit skips events with no uRwell ADC rows.
    if (adc.empty()) return;

    const int N = uRwellTools::nMaxUniqueChan + 1;

    std::map<int, double> m_ADC_uRWELL;
    std::map<int, double> m_ADC_GEM;
    std::map<int, int>    m_ts_GEM;
    std::map<int, double> m_MaxADC_GEM;

    std::vector<TGraphErrors> gr_uRwell(N);
    std::vector<TGraphErrors> gr_GEM(N);

    std::vector<double> uRWell_grMax(N, 0.);
    std::vector<double> uRWell_ts_grMax(N, 0.);

    // ---- Build the per-strip waveforms (Skim_PulseFit.cc:195) ---------------
    for (const AdcRow &r : adc) {
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

    // ---- GEM pulses (Skim_PulseFit.cc:242) ----------------------------------
    std::vector<uRwellTools::APV25Pulse> v_GEM_Pulses;
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

            v_GEM_Pulses.push_back(curPulse);
        }
    }

    // ---- uRwell pulses (Skim_PulseFit.cc:297) -------------------------------
    std::vector<uRwellTools::APV25Pulse> v_uRwell_Pulses;
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

            v_uRwell_Pulses.push_back(curPulse);
        }
    }

    int n_TotHits = int(v_GEM_Pulses.size() + v_uRwell_Pulses.size());
    if (n_TotHits == 0) return;   // Skim_PulseFit skips events with no pulses

    // ---- write the event ----------------------------------------------------
    hipo::bank bPulse(fSchPulse, n_TotHits);
    int col = 0;
    auto fill = [&](const uRwellTools::APV25Pulse &p) {
        bPulse.putShort("sec",        col, short(p.hit.sector));
        bPulse.putShort("layer",      col, short(p.hit.layer));
        bPulse.putShort("strip",      col, short(p.hit.strip));
        bPulse.putShort("stripLocal", col, short(p.hit.stripLocal));
        bPulse.putFloat("adc",        col, float(p.hit.adc));
        bPulse.putFloat("adcRel",     col, float(p.hit.adcRel));
        bPulse.putShort("ts",         col, short(p.hit.ts));
        bPulse.putShort("slot",       col, short(p.hit.slot));
        bPulse.putFloat("ped_rms",        col, float(p.ped_rms));
        bPulse.putFloat("pulse_p0",       col, float(p.pulse_p0));
        bPulse.putFloat("pulse_A0",       col, float(p.pulse_A0));
        bPulse.putFloat("pulse_MPV",      col, float(p.pulse_MPV));
        bPulse.putFloat("pulse_Sigma",    col, float(p.pulse_Sigma));
        bPulse.putFloat("pulse_Chi2",     col, float(p.pulse_Chi2));
        bPulse.putShort("pulse_NDF",      col, short(p.pulse_NDF));
        bPulse.putFloat("pulse_Integral", col, float(p.pulse_Integral));
        for (int ts = 0; ts < n_ts; ++ts) {
            char nm[16];
            snprintf(nm, sizeof(nm), "pulse_ADC%d", ts);
            bPulse.putFloat(nm, col, float(p.pulse_ADC[ts]));
        }
        col++;
    };
    for (const auto &p : v_uRwell_Pulses) fill(p);   // uRwell first, then GEM (as in skim)
    for (const auto &p : v_GEM_Pulses)    fill(p);

    hipo::bank bRaw(fSchRaw, 0);   // empty pass-through

    hipo::bank bCfg(fSchCfg, 1);
    bCfg.putInt("run", 0, run);
    bCfg.putInt("event", 0, event);
    bCfg.putInt("unixtime", 0, unixtime);
    bCfg.putLong("trigger", 0, 0);
    bCfg.putLong("timestamp", 0, 0);
    bCfg.putByte("type", 0, 0);
    bCfg.putByte("mode", 0, 0);
    bCfg.putFloat("torus", 0, 0.f);
    bCfg.putFloat("solenoid", 0, 0.f);

    hipo::bank bHodo(fSchHodo, int(tdc.size()));
    for (int i = 0; i < int(tdc.size()); ++i) {
        const TdcRow &r = tdc[i];
        bHodo.putByte("sector", i, int8_t(r.sector));
        bHodo.putByte("layer", i, int8_t(r.layer));
        bHodo.putShort("component", i, int16_t(r.component));
        bHodo.putByte("order", i, int8_t(r.order));
        bHodo.putInt("TDC", i, r.tdc);
    }

    hipo::event out;
    out.addStructure(bRaw);     // same order as Skim_PulseFit.cc:416
    out.addStructure(bCfg);
    out.addStructure(bPulse);
    out.addStructure(bHodo);
    fWriter.addEvent(out);

    fPulses += n_TotHits;
    fEvents++;
}
