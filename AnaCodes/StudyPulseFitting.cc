//
// Created by rafopar on 11/14/25.
//

#include <iostream>

// ===== Hipo headers =====
#include <reader.h>
#include <writer.h>
#include <dictionary.h>

#include "uRwellTools.h"
#include <cxxopts.hpp>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TLatex.h>

#include "../include/uRwellTools.h"

using namespace std;
using namespace uRwellTools;

int main(int argc, char *argv[]) {
    char inputFile[256];

    cxxopts::Options options("AnaPulseFits", "Performs clustering and also does analysis on cosmic data");

    options.add_options()
            ("r,Run", "Run number", cxxopts::value<int>())
            ("f,FileNo", "File number", cxxopts::value<int>());

    auto parsed_options = options.parse(argc, argv);

    int run = 0;
    int fnum = -1;


    if (parsed_options.count("Run")) {
        run = parsed_options["Run"].as<int>();
    } else {
        cout << "The run number is nor provided. Exiting..." << endl;
        exit(1);
    }

    if (parsed_options.count("FileNo")) {
        fnum = parsed_options["FileNo"].as<int>();
    } else {
        cout << "The file number is nor provided. Exiting..." << endl;
        exit(1);
    }

    sprintf(inputFile, "Skim_PulseFit_%d_%d.hipo", run, fnum);

    const int n_ts = 9;
    const int High_MPV_thr = 7;
    const int Low_MPV_thr = 2;
    const double highChi2Thr = 200;

    hipo::reader reader;
    reader.open(inputFile);

    hipo::dictionary factory;

    reader.readDictionary(factory);

    factory.show();
    hipo::event event;
    int evCounter = 0;

    auto *c1 = new TCanvas("c1", "", 950, 950);

    c1->Print("High_MPV_Pulses.pdf[");
    c1->Print("Low_MPV_Pulses.pdf[");
    c1->Print("Good_MPV_Pulses.pdf[");
    c1->Print("Large_Chi2.pdf[");

    auto gr_Pulse = TGraphErrors();

    gr_Pulse.SetMarkerColor(4);
    gr_Pulse.SetMarkerStyle(20);

    auto *f_bgrPlusLandau = new TF1("f_bgrPlusLandau", "[0] + [1]*TMath::Landau(x,[2],[3])", -10, 10.);
    f_bgrPlusLandau->SetNpx(4500);

    auto lat1 = new TLatex();
    lat1->SetNDC();
    lat1->SetTextFont(42);
    lat1->SetTextSize(0.03);

    hipo::bank buRwellPulses(factory.getSchema("uRwell::Pulse"));
    hipo::bank bRunConfig(factory.getSchema("RUN::config"));

    try {
        while (reader.next() == true) {
            reader.read(event);
            evCounter = evCounter + 1;

            if (evCounter > 1000) {
                break;
            }
            if (evCounter % 1000 == 0) {
                cout.flush() << "Processed " << evCounter << " events \r";
            }

            event.getStructure(buRwellPulses);
            event.getStructure(bRunConfig);

            int evNum = bRunConfig.getInt("event", 0);

            int nPulses = buRwellPulses.getRows();

            for (int iPulse = 0; iPulse < nPulses; iPulse++) {
                uRwellTools::uRwellHit curHit;
                curHit.sector = buRwellPulses.getInt("sec", iPulse);
                curHit.layer = buRwellPulses.getInt("layer", iPulse);
                curHit.strip = buRwellPulses.getInt("strip", iPulse);
                curHit.stripLocal = buRwellPulses.getInt("stripLocal", iPulse);
                curHit.adc = static_cast<double>(buRwellPulses.getFloat("adc", iPulse));
                curHit.adcRel = static_cast<double>(buRwellPulses.getFloat("adcRel", iPulse));
                curHit.slot = buRwellPulses.getInt("slot", iPulse);
                curHit.ts = buRwellPulses.getInt("ts", iPulse);

                uRwellTools::APV25Pulse curPulse;
                curPulse.hit = curHit;

                curPulse.pulse_p0 = static_cast<double>(buRwellPulses.getFloat("pulse_p0", iPulse));
                curPulse.pulse_A0 = static_cast<double>(buRwellPulses.getFloat("pulse_A0", iPulse));
                curPulse.pulse_MPV = static_cast<double>(buRwellPulses.getFloat("pulse_MPV", iPulse));
                curPulse.pulse_Sigma = static_cast<double>(buRwellPulses.getFloat("pulse_Sigma", iPulse));
                curPulse.pulse_Integral = static_cast<double>(buRwellPulses.getFloat("pulse_Integral", iPulse));
                curPulse.pulse_Chi2 = static_cast<double>(buRwellPulses.getFloat("pulse_Chi2", iPulse));
                curPulse.pulse_NDF = buRwellPulses.getInt("pulse_NDF", iPulse);
                curPulse.ped_rms = static_cast<double>(buRwellPulses.getFloat("ped_rms", iPulse));

                double ADC_[n_ts];

                double MaxADC = -10000;

                for (auto its = 0; its < n_ts; its++) {
                    ADC_[its] = static_cast<double>(buRwellPulses.getFloat(Form("pulse_ADC%d", its), iPulse));
                    if (ADC_[its] > MaxADC) {
                        MaxADC = ADC_[its];
                    }
                }


                if (MaxADC < 300) {
                    continue;
                }


                //c1->Clear();
                gr_Pulse.Set(0);

                for (auto its = 0; its < n_ts; its++) {
                    gr_Pulse.AddPointError(its, ADC_[its], 0., curPulse.ped_rms);
                }
                gr_Pulse.GetXaxis()->SetLimits(-1.0, 9.0);
                gr_Pulse.Draw("AP");
                f_bgrPlusLandau->SetParameters(curPulse.pulse_p0, curPulse.pulse_A0, curPulse.pulse_MPV,
                                               curPulse.pulse_Sigma);
                f_bgrPlusLandau->Draw("Same");

                lat1->DrawLatex(0.1, 0.96, Form("MPV = %1.2f", curPulse.pulse_MPV));
                lat1->DrawLatex(0.35, 0.96, Form("#sigma = %1.2f", curPulse.pulse_Sigma));
                lat1->DrawLatex(0.5, 0.96, Form("Run %d", run));
                lat1->DrawLatex(0.65, 0.96, Form("File %d", fnum));
                lat1->DrawLatex(0.8, 0.96, Form("Ev %d", evNum));
                lat1->DrawLatex(0.1, 0.91, Form("#chi2/ndf = %1.3f", curPulse.pulse_Chi2 / curPulse.pulse_NDF));
                lat1->DrawLatex(0.35, 0.91, Form("Layer = %d", curPulse.hit.layer));
                lat1->DrawLatex(0.5, 0.91, Form("Strip = %d", curPulse.hit.strip));

                if (curPulse.pulse_MPV > High_MPV_thr) {
                    c1->Print("High_MPV_Pulses.pdf");
                } else if (curPulse.pulse_MPV < Low_MPV_thr) {
                    c1->Print("Low_MPV_Pulses.pdf");
                }else {
                    c1->Print("Good_MPV_Pulses.pdf");
                }

                if (curPulse.pulse_Chi2/curPulse.pulse_NDF > highChi2Thr) {
                    c1->Print("Large_Chi2.pdf");
                }

            }
        }
    } catch (const exception &e) {
        cerr << "Error: " << e.what() << endl;
    }

    c1->Print("High_MPV_Pulses.pdf]");
    c1->Print("Low_MPV_Pulses.pdf]");
    c1->Print("Good_MPV_Pulses.pdf]");
    c1->Print("Large_Chi2.pdf]");
    return 0;
}
