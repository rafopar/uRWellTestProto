//
// Created by rafopar on 11/14/25.
//

#include <iostream>
#include <utility>
#include <set>

// ===== Hipo headers =====
#include <reader.h>
#include <writer.h>
#include <dictionary.h>

#include "uRwellTools.h"
#include <cxxopts.hpp>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TMultiGraph.h>
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
    const int sec_uRwell = 6;
    const int layer_U_uRwell = 1;
    const int layer_V_uRwell = 2;
    const int minHits = 3;

    std::set<std::pair<int, int> > set_selectedChannels;
    set_selectedChannels.insert(std::pair<int, int>(1, 206));
    set_selectedChannels.insert(std::pair<int, int>(1, 314));
    set_selectedChannels.insert(std::pair<int, int>(1, 657));
    set_selectedChannels.insert(std::pair<int, int>(1, 658));
    set_selectedChannels.insert(std::pair<int, int>(2, 7));
    set_selectedChannels.insert(std::pair<int, int>(2, 8));
    set_selectedChannels.insert(std::pair<int, int>(2, 289));
    set_selectedChannels.insert(std::pair<int, int>(2, 351));
    set_selectedChannels.insert(std::pair<int, int>(2, 425));


    hipo::reader reader;
    reader.open(inputFile);

    hipo::dictionary factory;

    reader.readDictionary(factory);

    factory.show();
    hipo::event event;
    int evCounter = 0;

    auto *c1 = new TCanvas("c1", "", 950, 950);

    for (auto it = set_selectedChannels.begin(); it != set_selectedChannels.end(); it++) {
        c1->Print(Form("PulseFigs/Selected_Channels_Layer_%d_Strip_%d_Run_%d.pdf[", it->first, it->second, run));
    }

    c1->Print("High_MPV_Pulses.pdf[");
    c1->Print("Low_MPV_Pulses.pdf[");
    c1->Print("Good_MPV_Pulses.pdf[");
    c1->Print("Large_Chi2.pdf[");
    c1->Print("SelectedChannels.pdf[");

    c1->Print("PulseFigs/LargeDeltaStartTime.pdf[");

    auto gr_Pulse = TGraphErrors();
    auto gr_U_SeedPulse = TGraphErrors();
    auto gr_V_SeedPulse = TGraphErrors();

    gr_Pulse.SetMarkerColor(4);
    gr_Pulse.SetMarkerStyle(20);

    gr_U_SeedPulse.SetMarkerColor(4);
    gr_U_SeedPulse.SetMarkerStyle(20);
    gr_V_SeedPulse.SetMarkerColor(2);
    gr_V_SeedPulse.SetMarkerStyle(21);


    auto *f_bgrPlusLandau = new TF1("f_bgrPlusLandau", "[0] + [1]*TMath::Landau(x,[2],[3])", -10, 10.);
    f_bgrPlusLandau->SetNpx(4500);

    auto f_U_SeedFitFunc = new TF1("f_USeedFitFunction", "[0] + [1]*TMath::Landau(x,[2],[3])", -10, 10.);
    f_U_SeedFitFunc->SetNpx(4500);
    f_U_SeedFitFunc->SetLineColor(4);
    auto f_V_SeedFitFunc = new TF1("f_V_SeedFitFunction", "[0] + [1]*TMath::Landau(x,[2],[3])", -10, 10.);
    f_V_SeedFitFunc->SetNpx(4500);
    f_V_SeedFitFunc->SetLineColor(2);


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

            if (evCounter > 2000) {
                break;
            }
            if (evCounter % 1000 == 0) {
                cout.flush() << "Processed " << evCounter << " events \r";
            }

            event.getStructure(buRwellPulses);
            event.getStructure(bRunConfig);

            std::vector<uRwellTools::APV25Pulse> v_U_Pulses;
            std::vector<uRwellTools::APV25Pulse> v_V_Pulses;

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

                if (curPulse.hit.sector != sec_uRwell) {
                    continue;
                }

                double ADC_[n_ts];

                double MaxADC = -10000;

                for (auto its = 0; its < n_ts; its++) {
                    ADC_[its] = static_cast<double>(buRwellPulses.getFloat(Form("pulse_ADC%d", its), iPulse));
                    if (ADC_[its] > MaxADC) {
                        MaxADC = ADC_[its];
                    }
                }

                std::copy(std::begin(ADC_), std::end(ADC_), std::begin(curPulse.pulse_ADC));

                if (MaxADC < 100) {
                    continue;
                }

                if (curPulse.hit.layer == layer_U_uRwell) {
                    v_U_Pulses.push_back(curPulse);
                } else if (curPulse.hit.layer == layer_V_uRwell) {
                    v_V_Pulses.push_back(curPulse);
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
                } else {
                    c1->Print("Good_MPV_Pulses.pdf");
                }

                if (curPulse.pulse_Chi2 / curPulse.pulse_NDF > highChi2Thr) {
                    c1->Print("Large_Chi2.pdf");
                }

                if (set_selectedChannels.find(std::make_pair(curPulse.hit.layer, curPulse.hit.strip)) !=
                    set_selectedChannels.end()) {
                    c1->Print(Form("PulseFigs/Selected_Channels_Layer_%d_Strip_%d_Run_%d.pdf", curPulse.hit.layer,
                                   curPulse.hit.strip, run));
                }
            }

            //       Number of U and V Pulses
            unsigned int n_U_Pulses = v_U_Pulses.size();
            unsigned int n_V_Pulses = v_V_Pulses.size();

            //       Forming U and V Clusters
            std::vector<uRwellTools::PulseCluster> v_U_PulseClusters = uRwellTools::getPulseClusters(v_U_Pulses);
            std::vector<uRwellTools::PulseCluster> v_V_PulseClusters = uRwellTools::getPulseClusters(v_V_Pulses);

            //       Number of U and V Clusters
            unsigned int n_U_PulseClusters = v_U_PulseClusters.size();
            unsigned int n_V_PulseClusters = v_V_PulseClusters.size();

            uRwellTools::PulseCluster Max_U_PulseCluster = uRwellTools::getMaxIntegralPulseCluster(v_U_PulseClusters, minHits);
            uRwellTools::PulseCluster Max_V_Pulsecluster = uRwellTools::getMaxIntegralPulseCluster(v_V_PulseClusters, minHits);

            bool has_U_AND_V_clusters = !Max_U_PulseCluster.getPulses()->empty() && !Max_V_Pulsecluster.getPulses()->empty();

            uRwellCross crs_Max_Integral;

            if (has_U_AND_V_clusters) {
                crs_Max_Integral = uRwellCross(Max_U_PulseCluster.getClusterCenter(), Max_V_Pulsecluster.getClusterCenter());

                double U_ClusterMPV = Max_U_PulseCluster.getClusterMPV();
                double V_ClusterMPV = Max_V_Pulsecluster.getClusterMPV();
                double U_SeedMPV = Max_U_PulseCluster.getSeedMPV();
                double V_SeedMPV = Max_V_Pulsecluster.getSeedMPV();
                double U_ClusterSigma = Max_U_PulseCluster.getClusterSigma();
                double V_ClusterSigma = Max_V_Pulsecluster.getClusterSigma();
                double U_ClusterStatTime = U_ClusterMPV - U_ClusterSigma;
                double V_ClusterStartTime = V_ClusterMPV - V_ClusterSigma;
                double U_SeedStartTime = U_SeedMPV - Max_U_PulseCluster.getSeedSigma();
                double V_SeedStartTime = V_SeedMPV - Max_V_Pulsecluster.getSeedSigma();

                double dtCluster_UV = U_ClusterMPV - V_ClusterMPV;
                double dtSeed_UV = U_SeedMPV - V_SeedMPV;
                double dtCluster_StartTime_UV = U_ClusterStatTime - V_ClusterStartTime;
                double dtSeed_StartTime_UV = U_SeedStartTime - V_SeedStartTime;

                double U_ClusterPulseIntegral = Max_U_PulseCluster.getClusterPulseIntegral();
                double V_ClusterPulseIntegral = Max_V_Pulsecluster.getClusterPulseIntegral();
                double U_SeedPulseIntegral = Max_U_PulseCluster.getSeedPulseIntegral();
                double V_SeedPulseIntegral = Max_V_Pulsecluster.getSeedPulseIntegral();

                auto v_Max_U_Pulses = Max_U_PulseCluster.getPulses();
                auto v_Max_V_Pulses = Max_V_Pulsecluster.getPulses();

                double max_U_Integral = 0;
                double max_V_Integral = 0;
                int ind_Seed_U = -1;
                int ind_Seed_V = -1;

                for ( int ind_U = 0; ind_U < v_Max_U_Pulses->size(); ind_U++ ) {
                    if ( v_Max_U_Pulses->at(ind_U).pulse_Integral > max_U_Integral ) {
                        ind_Seed_U = ind_U;
                        max_U_Integral = v_Max_U_Pulses->at(ind_U).pulse_Integral;
                    }
                }

                for ( int ind_V = 0; ind_V < v_Max_V_Pulses->size(); ind_V++ ) {
                    if ( v_Max_V_Pulses->at(ind_V).pulse_Integral > max_V_Integral ) {
                        ind_Seed_V = ind_V;
                        max_V_Integral = v_Max_V_Pulses->at(ind_V).pulse_Integral;
                    }
                }


                gr_U_SeedPulse.Set(0);
                gr_V_SeedPulse.Set(0);

                auto mtgr_UV = TMultiGraph();

                for ( auto ts = 0; ts < n_ts; ts++ ) {
                    gr_U_SeedPulse.AddPointError(ts, v_Max_U_Pulses->at(ind_Seed_U).pulse_ADC[ts], 0., v_Max_U_Pulses->at(ind_Seed_U).ped_rms );
                    gr_V_SeedPulse.AddPointError(ts, v_Max_V_Pulses->at(ind_Seed_V).pulse_ADC[ts], 0., v_Max_V_Pulses->at(ind_Seed_V).ped_rms );
                }

                if ( TMath::Abs(dtSeed_StartTime_UV) > 3 ) {

                    mtgr_UV.Add(&gr_U_SeedPulse);
                    mtgr_UV.Add(&gr_V_SeedPulse);
                    f_U_SeedFitFunc->SetParameters(v_Max_U_Pulses->at(ind_Seed_U).pulse_p0, v_Max_U_Pulses->at(ind_Seed_U).pulse_A0,  v_Max_U_Pulses->at(ind_Seed_U).pulse_MPV, v_Max_U_Pulses->at(ind_Seed_U).pulse_Sigma);
                    f_V_SeedFitFunc->SetParameters(v_Max_V_Pulses->at(ind_Seed_V).pulse_p0, v_Max_V_Pulses->at(ind_Seed_V).pulse_A0,  v_Max_V_Pulses->at(ind_Seed_V).pulse_MPV, v_Max_V_Pulses->at(ind_Seed_V).pulse_Sigma);
                    mtgr_UV.Draw("AP");
                    f_U_SeedFitFunc->Draw("Same");
                    f_V_SeedFitFunc->Draw("Same");

                    lat1->DrawLatex(0.01, 0.96, Form("MPV_U = %1.2f", v_Max_U_Pulses->at(ind_Seed_U).pulse_MPV));
                    lat1->DrawLatex(0.22, 0.96, Form("#sigma_U = %1.2f", v_Max_U_Pulses->at(ind_Seed_U).pulse_Sigma));
                    lat1->DrawLatex(0.39, 0.96, Form("#chi2_U/ndf_U = %1.1f", v_Max_U_Pulses->at(ind_Seed_U).pulse_Chi2 / v_Max_U_Pulses->at(ind_Seed_U).pulse_NDF));
                    lat1->DrawLatex(0.65, 0.96, Form("Str_U = %d", v_Max_U_Pulses->at(ind_Seed_U).hit.strip));
                    lat1->DrawLatex(0.81, 0.96, Form("File %d", fnum));
                    lat1->DrawLatex(0.9, 0.96, Form("Ev %d", evNum));
                    lat1->DrawLatex(0.01, 0.91, Form("MPV_V = %1.2f", v_Max_V_Pulses->at(ind_Seed_V).pulse_MPV));
                    lat1->DrawLatex(0.22, 0.91, Form("#sigma_V = %1.2f", v_Max_V_Pulses->at(ind_Seed_V).pulse_Sigma));
                    lat1->DrawLatex(0.39, 0.91, Form("#chi2_V/ndf_V = %1.1f", v_Max_V_Pulses->at(ind_Seed_V).pulse_Chi2 / v_Max_V_Pulses->at(ind_Seed_V).pulse_NDF));

                    lat1->DrawLatex(0.65, 0.91, Form("Str_V = %d", v_Max_V_Pulses->at(ind_Seed_V).hit.strip));
                    lat1->DrawLatex(0.82, 0.91, Form("Run %d", run));

                    c1->Print("PulseFigs/LargeDeltaStartTime.pdf");
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
    c1->Print("SelectedChannels.pdf]");
    for (auto it = set_selectedChannels.begin(); it != set_selectedChannels.end(); it++) {
        c1->Print(Form("PulseFigs/Selected_Channels_Layer_%d_Strip_%d_Run_%d.pdf]", it->first, it->second, run));
    }
    c1->Print("PulseFigs/LargeDeltaStartTime.pdf]");

    return 0;
}
