//
// Created by rafopar on 10/24/25.
//

#include <cstdlib>

#include <TH2D.h>
#include <TH1D.h>
#include <TMath.h>
#include <TFile.h>

// ===== Hipo headers =====
#include <reader.h>
#include <writer.h>
#include <dictionary.h>

#include "uRwellTools.h"
#include <cxxopts.hpp>

using namespace std;
using namespace uRwellTools;

int main (int argc, char *argv[]) {

    char outputFile[256];
    char inputFile[256];

    cxxopts::Options options("AnaPulseFits", "Performs clustering and also does analysis on cosmic data");

    options.add_options()
            ("r,Run", "Run number", cxxopts::value<int>())
            ("f,FileNo", "File number", cxxopts::value<int>())
            ;

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

    sprintf(inputFile, "Skim_PulseFit_2234_0.hipo", run, fnum);

    const int layer_U_uRwell = 1;
    const int layer_V_uRwell = 2;
    const int layer_X_GEM = 1;
    const int layer_Y_GEM = 2;
    const int sec_uRwell = 6;
    const int sec_GEM = 8;

    const double chi2NDF_cut = 40;

    hipo::reader reader;
    reader.open(inputFile);

    hipo::dictionary factory;

    reader.readDictionary(factory);

    factory.show();
    hipo::event event;
    int evCounter = 0;

    hipo::bank buRwellPulses(factory.getSchema("uRwell::Pulse"));
    hipo::bank bRAWADc(factory.getSchema("RAW::adc"));
    hipo::bank bRunConf(factory.getSchema("RUN::config"));

    auto *file_out = new TFile(Form("AnaPulseFits_%d_File_%d.root", run, fnum), "Recreate");

    TH2D h_Pulse_Integral_vs_MPV_uRwell1("h_Pulse_Integral_vs_MPV_uRwell1", "", 200, 0., 20000., 200, -5., 15);
    TH2D h_Pulse_Integral_vs_MPVMinusSiigm_uRwell1("h_Pulse_Integral_vs_MPVMinusSiigm_uRwell1", "", 200, 0., 20000., 200, -5., 15);
    TH2D h_Pulse_Integral_vs_MPV_uRwell2("h_Pulse_Integral_vs_MPV_uRwell2", "", 200, 0., 20000., 200, -5., 15);
    TH2D h_Pulse_Integral_vs_MPVMinusSiigm_uRwell2("h_Pulse_Integral_vs_MPVMinusSiigm_uRwell2", "", 200, 0., 20000., 200, -5., 15);
    TH2D h_Pulse_Integral_vs_Chi2NDF_uRwell1("h_Pulse_Integral_vs_Chi2NDF_uRwell1", "", 200, 0., 20000., 200, 0., 100.);

    try {

        while (reader.next() == true) {

            reader.read(event);
            evCounter = evCounter + 1;

            if (evCounter % 1000 == 0) {
                cout.flush() << "Processed " << evCounter << " events \r";
            }

            event.getStructure(buRwellPulses);
            event.getStructure(bRAWADc);
            event.getStructure(bRunConf);

            int nPulses = buRwellPulses.getRows();

            for (int iPulse = 0; iPulse < nPulses; iPulse++) {
                uRwellTools::uRwellHit curHit;
                curHit.sector = buRwellPulses.getInt("sec", iPulse);
                curHit.layer = buRwellPulses.getInt("layer", iPulse);
                curHit.strip = buRwellPulses.getInt("strip", iPulse);
                curHit.stripLocal = buRwellPulses.getInt("stripLocal", iPulse);
                curHit.adc = double(buRwellPulses.getFloat("adc", iPulse));
                curHit.adcRel = double(buRwellPulses.getFloat("adcRel", iPulse));
                curHit.slot = buRwellPulses.getInt("slot", iPulse);
                curHit.ts = buRwellPulses.getInt("ts", iPulse);

                uRwellTools::APV25Pulse curPulse;
                curPulse.hit = curHit;

                curPulse.pulse_A0 = double(buRwellPulses.getFloat("pulse_A0", iPulse));
                curPulse.pulse_MPV = double(buRwellPulses.getFloat("pulse_MPV", iPulse));
                curPulse.pulse_Sigma = double(buRwellPulses.getFloat("pulse_Sigma", iPulse));
                curPulse.pulse_Integral = double(buRwellPulses.getFloat("pulse_Integral", iPulse));
                curPulse.pulse_Chi2 = double(buRwellPulses.getFloat("pulse_Chi2", iPulse));
                curPulse.pulse_NDF = buRwellPulses.getInt("pulse_NDF", iPulse);

                if ( curPulse.hit.sector == sec_uRwell ) {
                    h_Pulse_Integral_vs_MPV_uRwell1.Fill(curPulse.pulse_Integral, curPulse.pulse_MPV);
                    h_Pulse_Integral_vs_MPVMinusSiigm_uRwell1.Fill(curPulse.pulse_Integral, curPulse.pulse_MPV - curPulse.pulse_Sigma);
                    h_Pulse_Integral_vs_Chi2NDF_uRwell1.Fill(curPulse.pulse_Integral, curPulse.pulse_Chi2/curPulse.pulse_NDF);

                    if ( curPulse.pulse_Chi2/curPulse.pulse_NDF < chi2NDF_cut) {
                        h_Pulse_Integral_vs_MPV_uRwell2.Fill(curPulse.pulse_Integral, curPulse.pulse_MPV);
                        h_Pulse_Integral_vs_MPVMinusSiigm_uRwell2.Fill(curPulse.pulse_Integral, curPulse.pulse_MPV - curPulse.pulse_Sigma);
                    }
                }

            }

        }

    }catch (exception &e) {
        cerr<<e.what()<<endl;
    }

    gDirectory->Write();
    file_out->Close();


}