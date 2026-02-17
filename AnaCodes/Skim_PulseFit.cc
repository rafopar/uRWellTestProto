//
// Created by rafopar on 10/21/25.
//
#include <cstdio>
#include <cstdlib>
#include <fstream>

#include <TMath.h>
#include <TFile.h>

#include <TSystem.h>
#include <TGraphErrors.h>

// ===== Hipo headers =====
#include <reader.h>
#include <writer.h>
#include <dictionary.h>

// ===== Custom headers =====
#include <uRwellTools.h>


using namespace std;

int main(int argc, char **argv) {
    char outputFile[256];
    char inputFile[256];

    int run = 0;
    int fnum = -1;
    if (argc > 2) {
        run = atoi(argv[1]);
        fnum = atoi(argv[2]);

        sprintf(inputFile, "Data/decoded_%d_%d.hipo", run, fnum);
        sprintf(outputFile, "Skim_PulseFit_%d_%d.hipo", run, fnum);
        std::string outFileName_Str = outputFile;

        if (uRwellTools::fileExists(outputFile)) {
            cout << "The file " << outFileName_Str.c_str() << " exists. The program will not run" << endl;
            cout << "Exiting" << endl;
            exit(1);
        }
    } else {
        std::cout << " *** please provide a run number and the file index..." << std::endl;
        exit(0);
    }

    gSystem->RedirectOutput("/dev/null", "a"); // hides ROOT's own prints

    /**
     * Creating schema for uRwell::Hit
     * This is more of experimental study, that is why I want to keep many details, while later for established
     * analysis, just time (e.g. MPV) would be enough.
     */
    hipo::schema sch("uRwell::Pulse", 90, 1);
    sch.parse(
        "sec/S,layer/S,strip/S,stripLocal/S,adc/F,adcRel/F,ts/S,slot/S,ped_rms/F,pulse_p0/F,pulse_A0/F,pulse_MPV/F,pulse_Sigma/F,pulse_Chi2/F,pulse_NDF/S,pulse_Integral/F,pulse_ADC0/F,pulse_ADC1/F,pulse_ADC2/F,pulse_ADC3/F,pulse_ADC4/F,pulse_ADC5/F,pulse_ADC6/F,pulse_ADC7/F,pulse_ADC8/F,pulse_ADC9/F,pulse_ADC10/F,pulse_ADC11/F,pulse_ADC12/F,pulse_ADC13/F,pulse_ADC14/F");
    sch.show();

    auto *f_bgrPlusLandau = new TF1("f_bgrPlusLandau", "[0] + [1]*TMath::Landau(x,[2],[3])", -10, 10.);
    f_bgrPlusLandau->SetNpx(4500);

    hipo::reader reader;
    reader.open(inputFile);

    hipo::dictionary factory;

    reader.readDictionary(factory);

    factory.show();
    hipo::event event;
    int evCounter = 0;

    hipo::bank buRWellADC(factory.getSchema("URWELL::adc"));
    hipo::bank bRAWADc(factory.getSchema("RAW::adc"));
    hipo::bank bRunConf(factory.getSchema("RUN::config"));
    hipo::bank bXYHodo(factory.getSchema("XYHODO::tdc"));

    hipo::writer writer;
    writer.getDictionary().addSchema(sch);
    writer.getDictionary().addSchema(factory.getSchema("RAW::adc"));
    writer.getDictionary().addSchema(factory.getSchema("RUN::config"));
    writer.getDictionary().addSchema(factory.getSchema("XYHODO::tdc"));


    writer.open(outputFile);

    int __bank_Sec_INDEX_ = buRWellADC.getSchema().getEntryOrder("sector");
    int __bank_Layer_INDEX_ = buRWellADC.getSchema().getEntryOrder("layer");
    int __bank_Component_INDEX_ = buRWellADC.getSchema().getEntryOrder("component");
    int __bank_Order_INDEX_ = buRWellADC.getSchema().getEntryOrder("order");
    int __bank_ADC_INDEX_ = buRWellADC.getSchema().getEntryOrder("ADC");
    int __bank_Time_INDEX_ = buRWellADC.getSchema().getEntryOrder("time");
    int __bank_Ped_INDEX_ = buRWellADC.getSchema().getEntryOrder("ped");

    const double sigm_threshold = 3.; // Represents the threshold of the ADC in units of the sigma.

    const int nGEMChannels = 256;
    const int n_ts = 15;
    const int sec_GEM = 8; // GEM is in Sec 8
    const int sec_uRWell = 6; // uRWELL is in Sec 8

    /**
 * Reading the pedestal file and and fill a map for pedestals and RMSs
 */

    std::map<int, double> m_ped_mean; // Mean value of the pedestal
    std::map<int, double> m_ped_rms; // rms of the pedestal
    std::map<int, double> m_ped_GEM_mean; // Mean value of the pedestal
    std::map<int, double> m_ped_GEM_rms; // rms of the pedestal

    ifstream inp_ped(Form("PedFiles/Peds_%d", run));

    if (!inp_ped.is_open()) {
        cout << "Can not find the pedestal file \"PedFiles/CosmicPeds.dat\". " << endl;
        cout << "Exiting..." << endl;
        exit(1);
    }

    cout << "Kuku" << endl;
    while (!inp_ped.eof()) {
        int ch;
        double mean, rms;
        inp_ped >> ch;
        inp_ped >> mean;
        inp_ped >> rms;

        m_ped_mean[ch] = mean;
        m_ped_rms[ch] = rms;
    }

    ifstream inp_ped_GEM(Form("PedFiles/GEM_Peds_%d", run));

    if (!inp_ped_GEM.is_open()) {
        cout << "Can not find the pedestal file. " << endl;
        cout << "Exiting..." << endl;
        exit(1);
    }

    while (!inp_ped_GEM.eof()) {
        int ch;
        double mean, rms;
        inp_ped_GEM >> ch;
        inp_ped_GEM >> mean;
        inp_ped_GEM >> rms;

        m_ped_GEM_mean[ch] = mean;
        m_ped_GEM_rms[ch] = rms;
    }

    cout << "The pedestal map is loaded." << endl;

    try {
        while (reader.next() == true) {
            reader.read(event);

            evCounter = evCounter + 1;

            //if (evCounter > 1500) { break; }
            if (evCounter % 1000 == 0) {
                gSystem->RedirectOutput(0);
                cout.flush() << "Processed " << evCounter << " events \r";
                gSystem->RedirectOutput("/dev/null", "a"); // hides ROOT's own prints
            }

            event.getStructure(buRWellADC);
            event.getStructure(bRAWADc);
            event.getStructure(bRunConf);

            int n_uRwellADC = buRWellADC.getRows();
            if (n_uRwellADC == 0) {
                continue;
            }

            std::map<int, int> m_ts_uRWELL;
            // This represents the time sample that has the highest ADC for the given strip
            std::map<int, double> m_MaxADC_uRWELL;
            // This represents the maximum ADC from all time samples of the given strip
            std::map<int, double> m_ADC_uRWELL;
            std::map<int, double> m_ADCRel_uRWELL;
            std::map<int, int> m_ts_GEM; // This represents the time sample that has the highest ADC for the given strip
            std::map<int, double> m_MaxADC_GEM;
            // This represents the maximum ADC from all time samples of the given strip
            std::map<int, double> m_ADC_GEM;
            std::map<int, double> m_ADCRel_GEM;

            TGraphErrors gr_ADC_Waveforms_uRwell[uRwellTools::nMaxUniqueChan + 1];
            TGraphErrors gr_ADC_Waveforms_GEM[uRwellTools::nMaxUniqueChan + 1];

            double uRWell_grMax[uRwellTools::nMaxUniqueChan + 1] = {0.};
            double uRWell_ts_grMax[uRwellTools::nMaxUniqueChan + 1] = {0.};

            for (int i = 0; i < n_uRwellADC; i++) {
                int sector = buRWellADC.getInt(__bank_Sec_INDEX_, i);
                int layer = buRWellADC.getInt(__bank_Layer_INDEX_, i);
                int channel = buRWellADC.getInt(__bank_Component_INDEX_, i);
                int ADC = buRWellADC.getInt(__bank_ADC_INDEX_, i);
                int uniqueChan = int(buRWellADC.getFloat(__bank_Time_INDEX_, i));
                int ts = buRWellADC.getInt(__bank_Ped_INDEX_, i);

                int slot = layer;

                if (sector == sec_GEM) {
                    m_ADC_GEM[uniqueChan] += double(ADC);

                    gr_ADC_Waveforms_GEM[uniqueChan].AddPointError(ts, m_ped_GEM_mean[uniqueChan] - ADC, 0,
                                                                   m_ped_GEM_rms[uniqueChan]);

                    if (m_ped_GEM_mean[uniqueChan] - ADC > m_MaxADC_GEM[uniqueChan]) {
                        m_MaxADC_GEM[uniqueChan] = m_ped_GEM_mean[uniqueChan] - ADC;
                        m_ts_GEM[uniqueChan] = ts;
                    }
                } else if (sector == sec_uRWell) {
                    m_ADC_uRWELL[uniqueChan] = m_ADC_uRWELL[uniqueChan] + double(ADC);

                    gr_ADC_Waveforms_uRwell[uniqueChan].AddPointError(ts, m_ped_mean[uniqueChan] - ADC, 0,
                                                                      m_ped_rms[uniqueChan]);

                    if ( m_ped_mean[uniqueChan] - ADC > uRWell_grMax[uniqueChan]  ) {
                        uRWell_grMax[uniqueChan] = m_ped_mean[uniqueChan] - ADC;
                        uRWell_ts_grMax[uniqueChan] = ts;
                    }

                    if (m_ped_mean[uniqueChan] - ADC > m_MaxADC_uRWELL[uniqueChan]) {
                        m_MaxADC_uRWELL[uniqueChan] = m_ped_mean[uniqueChan] - ADC;
                        m_ts_uRWELL[uniqueChan] = ts;
                    }
                }
            }

            vector<uRwellTools::APV25Pulse> v_GEM_Pulses;
            for (auto it = m_ADC_GEM.begin(); it != m_ADC_GEM.end(); ++it) {
                int ch = it->first;

                m_ADC_GEM[ch] = m_ped_GEM_mean[ch] - m_ADC_GEM[ch] / double(n_ts);
                m_ADCRel_GEM[ch] = m_ADC_GEM[ch] / m_ped_GEM_rms[ch];

                if (m_ADCRel_GEM[ch] > sigm_threshold) {
                    uRwellTools::uRwellHit curHit;
                    curHit.adc = m_ADC_GEM[ch];
                    curHit.adcRel = m_ADCRel_GEM[ch];
                    curHit.sector = sec_GEM;
                    curHit.layer = 1 + ch / 1000;
                    curHit.slot = uRwellTools::getGEMSlot(ch);
                    curHit.strip = ch % 1000;
                    curHit.stripLocal = ch - uRwellTools::slot_Offset[curHit.slot];
                    curHit.ts = m_ts_GEM[ch];

                    uRwellTools::APV25Pulse curPulse;

                    f_bgrPlusLandau->FixParameter(0, 0.);
                    f_bgrPlusLandau->SetParameter(
                        1, 4 * (gr_ADC_Waveforms_GEM[ch].GetMaximum() - gr_ADC_Waveforms_GEM[ch].GetMinimum()));
                    f_bgrPlusLandau->SetParLimits(1, 0., 10000.);
                    f_bgrPlusLandau->SetParameter(2, 3);
                    f_bgrPlusLandau->SetParLimits(3, 0.4, 10.);

                    gr_ADC_Waveforms_GEM[ch].Fit(f_bgrPlusLandau, "MeQ", "", -1.1, double(n_ts) + 0.1);
                    //gr_ADC_Waveforms[ch].Fit(f_bgrPlusWaveform1, "MeV", "", -1.1, 9.1);

                    curPulse.hit = curHit;
                    curPulse.ped_rms = m_ped_GEM_rms[ch];
                    curPulse.pulse_p0 = f_bgrPlusLandau->GetParameter(0);
                    curPulse.pulse_A0 = f_bgrPlusLandau->GetParameter(1);
                    curPulse.pulse_MPV = f_bgrPlusLandau->GetParameter(2);
                    curPulse.pulse_Sigma = f_bgrPlusLandau->GetParameter(3);
                    curPulse.pulse_Chi2 = f_bgrPlusLandau->GetChisquare();
                    curPulse.pulse_NDF = f_bgrPlusLandau->GetNDF();
                    curPulse.pulse_Integral = f_bgrPlusLandau->Integral(curPulse.pulse_MPV - 5 * curPulse.pulse_Sigma,
                                                                        curPulse.pulse_MPV + 50. * curPulse.
                                                                        pulse_Sigma);

                    for (auto ts = 0; ts < n_ts; ++ts) {
                        curPulse.pulse_ADC[int(gr_ADC_Waveforms_GEM[ch].GetPointX(ts))] = gr_ADC_Waveforms_GEM[ch].
                                GetPointY(ts);
                    }

                    v_GEM_Pulses.push_back(curPulse);
                }
            }

            vector<uRwellTools::APV25Pulse> v_uRwell_Pulses;

            for (auto it = m_ADC_uRWELL.begin(); it != m_ADC_uRWELL.end(); ++it) {
                int ch = it->first;
                m_ADC_uRWELL[ch] = m_ped_mean[ch] - m_ADC_uRWELL[ch] / double(n_ts);
                m_ADCRel_uRWELL[ch] = m_ADC_uRWELL[ch] / m_ped_rms[ch];

                if (m_ADCRel_uRWELL[ch] > sigm_threshold) {
                    uRwellTools::uRwellHit curHit;
                    curHit.adc = m_ADC_uRWELL[ch];
                    curHit.adcRel = m_ADCRel_uRWELL[ch];
                    curHit.sector = sec_uRWell;
                    curHit.layer = 1 + ch / 1000;
                    curHit.slot = uRwellTools::getURwellSlot(ch);
                    curHit.strip = ch % 1000;
                    curHit.stripLocal = ch - uRwellTools::slot_Offset[curHit.slot];
                    curHit.ts = m_ts_uRWELL[ch];

                    uRwellTools::APV25Pulse curPulse;

                    f_bgrPlusLandau->FixParameter(0, 0.);
                    f_bgrPlusLandau->SetParameter(
                        1, 4 * (gr_ADC_Waveforms_uRwell[ch].GetMaximum() - gr_ADC_Waveforms_uRwell[ch].GetMinimum()));
                    f_bgrPlusLandau->SetParLimits(1, 0., 10000.);
                    //f_bgrPlusLandau->SetParameter(2, 3);
                    f_bgrPlusLandau->SetParameter(2, uRWell_ts_grMax[ch]);
                    f_bgrPlusLandau->SetParLimits(3, 0.4, 10.);

                    //gr_ADC_Waveforms_uRwell[ch].Fit(f_bgrPlusLandau, "MeQ", "", -1.1, 9.1);
                    gr_ADC_Waveforms_uRwell[ch].Fit(f_bgrPlusLandau, "MeQ", "", -1.1, double(n_ts) + 0.1);

                    curPulse.hit = curHit;
                    curPulse.ped_rms = m_ped_rms[ch];
                    curPulse.pulse_p0 = f_bgrPlusLandau->GetParameter(0);
                    curPulse.pulse_A0 = f_bgrPlusLandau->GetParameter(1);
                    curPulse.pulse_MPV = f_bgrPlusLandau->GetParameter(2);
                    curPulse.pulse_Sigma = f_bgrPlusLandau->GetParameter(3);
                    curPulse.pulse_Chi2 = f_bgrPlusLandau->GetChisquare();
                    curPulse.pulse_NDF = f_bgrPlusLandau->GetNDF();
                    curPulse.pulse_Integral = f_bgrPlusLandau->Integral(curPulse.pulse_MPV - 5 * curPulse.pulse_Sigma,
                                                                        curPulse.pulse_MPV + 50. * curPulse.
                                                                        pulse_Sigma);

                    for (auto ts = 0; ts < n_ts; ++ts) {
                        curPulse.pulse_ADC[int(gr_ADC_Waveforms_uRwell[ch].GetPointX(ts))] = gr_ADC_Waveforms_uRwell[ch]
                                .GetPointY(ts);
                    }


                    v_uRwell_Pulses.push_back(curPulse);
                }
            }

            int n_TotHits = v_GEM_Pulses.size() + v_uRwell_Pulses.size();

            if (n_TotHits == 0) {
                continue;
            }

            hipo::bank buRwellPulses(sch, n_TotHits);
            hipo::event outEvent;

            int col = 0;
            //     *********** Writing uRwell pulses above the threshold **********
            // "sec/S,layer/S,strip/S,stripLocal/S,adc/F,adcRel/F,ts/S,slot/S,
            // ped_rms/F,pulse_p0/F,pulse_A0/F,pulse_MPV/F,pulse_Sigma/F,pulse_Chi2/F,pulse_NDF/S,pulse_ADC0/F,pulse_ADC1/F,pulse_ADC2/F,pulse_ADC3/F,pulse_ADC4/F,
            // pulse_ADC5/F,pulse_ADC6/F,pulse_ADC7/F,pulse_ADC8/F,pulse_ADC9/F,pulse_ADC10/F,pulse_ADC11/F,pulse_ADC12/F,pulse_ADC13/F,pulse_ADC14/F");
            for (auto curPulse: v_uRwell_Pulses) {
                buRwellPulses.putShort("sec", col, short(curPulse.hit.sector));
                buRwellPulses.putShort("layer", col, short(curPulse.hit.layer));
                buRwellPulses.putShort("strip", col, short(curPulse.hit.strip));
                buRwellPulses.putShort("stripLocal", col, short(curPulse.hit.stripLocal));
                buRwellPulses.putFloat("adc", col, float(curPulse.hit.adc));
                buRwellPulses.putFloat("adcRel", col, float(curPulse.hit.adcRel));
                buRwellPulses.putShort("ts", col, short(curPulse.hit.ts));
                buRwellPulses.putShort("slot", col, short(curPulse.hit.slot));
                buRwellPulses.putFloat("ped_rms", col, float(curPulse.ped_rms));
                buRwellPulses.putFloat("pulse_p0", col, float(curPulse.pulse_p0));
                buRwellPulses.putFloat("pulse_A0", col, float(curPulse.pulse_A0));
                buRwellPulses.putFloat("pulse_MPV", col, float(curPulse.pulse_MPV));
                buRwellPulses.putFloat("pulse_Sigma", col, float(curPulse.pulse_Sigma));
                buRwellPulses.putFloat("pulse_Chi2", col, float(curPulse.pulse_Chi2));
                buRwellPulses.putShort("pulse_NDF", col, short(curPulse.pulse_NDF));
                buRwellPulses.putFloat("pulse_Integral", col, float(curPulse.pulse_Integral));

                for (int ts = 0; ts < n_ts; ++ts) {
                    buRwellPulses.putFloat(Form("pulse_ADC%d", ts), col, float(curPulse.pulse_ADC[ts]));
                }

                col = col + 1;
            }

            for (auto curPulse: v_GEM_Pulses) {
                buRwellPulses.putShort("sec", col, short(curPulse.hit.sector));
                buRwellPulses.putShort("layer", col, short(curPulse.hit.layer));
                buRwellPulses.putShort("strip", col, short(curPulse.hit.strip));
                buRwellPulses.putShort("stripLocal", col, short(curPulse.hit.stripLocal));
                buRwellPulses.putFloat("adc", col, float(curPulse.hit.adc));
                buRwellPulses.putFloat("adcRel", col, float(curPulse.hit.adcRel));
                buRwellPulses.putShort("ts", col, short(curPulse.hit.ts));
                buRwellPulses.putShort("slot", col, short(curPulse.hit.slot));
                buRwellPulses.putFloat("ped_rms", col, float(curPulse.ped_rms));
                buRwellPulses.putFloat("pulse_p0", col, float(curPulse.pulse_p0));
                buRwellPulses.putFloat("pulse_A0", col, float(curPulse.pulse_A0));
                buRwellPulses.putFloat("pulse_MPV", col, float(curPulse.pulse_MPV));
                buRwellPulses.putFloat("pulse_Sigma", col, float(curPulse.pulse_Sigma));
                buRwellPulses.putFloat("pulse_Chi2", col, float(curPulse.pulse_Chi2));
                buRwellPulses.putShort("pulse_NDF", col, short(curPulse.pulse_NDF));
                buRwellPulses.putFloat("pulse_Integral", col, float(curPulse.pulse_Integral));

                for (int ts = 0; ts < n_ts; ++ts) {
                    buRwellPulses.putFloat(Form("pulse_ADC%d", ts), col, float(curPulse.pulse_ADC[ts]));
                }

                col = col + 1;
            }

            event.getStructure(bXYHodo);
            outEvent.addStructure(bRAWADc);
            outEvent.addStructure(bRunConf);
            outEvent.addStructure(buRwellPulses);
            outEvent.addStructure(bXYHodo);
            // outEvent.addStructure(bVMM3ADC);
            writer.addEvent(outEvent);
        }
        gSystem->RedirectOutput(0);
    } catch (exception &e) {
        cerr << e.what() << endl;
    }


    writer.close();
    writer.showSummary();

    return 0;
}
