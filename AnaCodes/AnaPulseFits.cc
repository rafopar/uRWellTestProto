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

    sprintf(inputFile, "Skim_PulseFit_%d_%d.hipo", run, fnum);

    const int layer_U_uRwell = 1;
    const int layer_V_uRwell = 2;
    const int layer_X_GEM = 1;
    const int layer_Y_GEM = 2;
    const int sec_uRwell = 6;
    const int sec_GEM = 8;
    const double PulseSigmaMin = 1.2;

    const int minHits = 3;

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
    TH2D h_Pulse_Integral_vs_StartTime_uRwell1("h_Pulse_Integral_vs_StartTime_uRwell1", "", 200, 0., 20000., 200, -5., 15);
    TH2D h_Pulse_Integral_vs_MPV_uRwell2("h_Pulse_Integral_vs_MPV_uRwell2", "", 200, 0., 20000., 200, -5., 15);
    TH2D h_Pulse_Integral_vs_StartTime_uRwell2("h_Pulse_Integral_vs_StartTime_uRwell2", "", 200, 0., 20000., 200, -5., 15);
    TH2D h_Pulse_Integral_vs_Chi2NDF_uRwell1("h_Pulse_Integral_vs_Chi2NDF_uRwell1", "", 200, 0., 20000., 200, 0., 100.);

    TH2D h_N_V_vs_U_Pulses1("h_N_V_vs_U_Pulses1", "", 21, -0.5, 20.5, 21, -0.5, 20.5);
    TH2D h_N_V_vs_U_PulsClustrs1("h_N_V_vs_U_PulsClustrs1", "", 7, -0.5, 6.5, 7, -0.5, 6.5);

    TH2D h_Cross_YXc_MaxIntegral1("h_Cross_YXc_MaxIntegral1", "", 1000, -900., 900., 200, -500., 500.);

    TH2D h_Cross_YXc_WeightedClusterStartTimeDiff1("h_Cross_YXc_WeightedClusterStartTimeDiff1", "", 1000, -900., 900., 200, -500., 500.);

    TH2D h_Cross_V_vs_U_ClusterMPV1("h_Cross_V_vs_U_ClusterMPV1", "", 200, -5, 15., 200, -5., 15.);
    TH2D h_Cross_V_vs_U_SeedMPV1("h_Cross_V_vs_U_SeedMPV1", "", 200, -5, 15., 200, -5., 15.);
    TH2D h_Cross_V_vs_U_ClusterStartTime1("h_Cross_V_vs_U_ClusterStartTime1", "", 200, -5., 15., 200, -5., 15.);
    TH2D h_Cross_V_vs_U_SeedStartTime1("h_Cross_V_vs_U_SeedStartTime1", "", 200, -5., 15., 200, -5., 15.);

    TH1D h_Cross_ClusterTimeDiff1("h_Cross_ClusterTimeDiff1", "", 200, -5, 5);
    TH1D h_Cross_SeedTimeDiff1("h_Cross_SeedTimeDiff1", "", 200, -5, 5);
    TH1D h_Cross_ClusterStartTimeDiff1("h_Cross_ClusterStartTimeDiff1", "", 200, -5, 5);
    TH1D h_Cross_SeedStartTimeDiff1("h_Cross_SeedStartTimeDiff1", "", 200, -5, 5);

    TH2D h_Cross_V_vs_U_ClusterPulseIntegral1("h_Cross_V_vs_U_ClusterPulseIntegral1", "", 200, 0, 50000., 200, 0., 50000.);
    TH2D h_Cross_V_vs_U_SeedPulseIntegral("h_Cross_V_vs_U_SeedPulseIntegral", "", 200, 0, 50000., 200, 0., 50000.);

    TH2D h_Cross_ClusterTimeDiff_vs_U_clPulseIntegral1("h_Cross_ClusterTimeDiff_vs_U_clPulseIntegral1", "", 200, 0, 50000., 200, -5., 5.);
    TH2D h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral1("h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral1", "", 200, 0, 50000., 200, -5., 5.);
    TH2D h_Cross_SeedTimeDiff_vs_U_clPulseIntegral1("h_Cross_SeedTimeDiff_vs_U_clPulseIntegral1", "", 200, 0, 50000., 200, -5., 5.);
    TH2D h_Cross_SeedStartTimeDiff_vs_U_cl_PulseIntegral1("h_Cross_SeedStartTimeDiff_vs_U_cl_PulseIntegral1", "", 200, 0, 50000., 200, -5., 5.);
    TH2D h_Cross_SeedTimeDiff_vs_U_clPulseIntegral_GoodWidth1("h_Cross_SeedTimeDiff_vs_U_clPulseIntegral_GoodWidth1", "", 200, 0, 50000., 200, -5., 5.);
    TH2D h_Cross_SeedStartTimeDiff_vs_U_cl_PulseIntegral_GoodWidth1("h_Cross_SeedStartTimeDiff_vs_U_cl_PulseIntegral_GoodWidth1", "", 200, 0, 50000., 200, -5., 5.);

    TH2D h_Cross_UCluster_Sigms_vs_Slot1("h_Cross_UCluster_Sigms_vs_Slot1", "", 13, -0.5, 12.5, 200, 0., 5.);
    TH2D h_Cross_VCluster_Sigms_vs_Slot1("h_Cross_VCluster_Sigms_vs_Slot1", "", 13, -0.5, 12.5, 200, 0., 5.);

    TH2D h_Cross_UCluster_Sgima_vs_Strip1("h_Cross_UCluster_Sgima_vs_Strip1", "", 706, -0.5, 705.5, 200, 0., 5.);
    TH2D h_Cross_VCluster_Sigma_vs_Strip1("h_Cross_VCluster_Sgima_vs_Strip1", "", 706, -0.5, 705.5, 200, 0., 5.);

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

            std::vector<uRwellTools::APV25Pulse> v_U_Pulses;
            std::vector<uRwellTools::APV25Pulse> v_V_Pulses;

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
                    h_Pulse_Integral_vs_StartTime_uRwell1.Fill(curPulse.pulse_Integral, curPulse.pulse_MPV - curPulse.pulse_Sigma);
                    h_Pulse_Integral_vs_Chi2NDF_uRwell1.Fill(curPulse.pulse_Integral, curPulse.pulse_Chi2/curPulse.pulse_NDF);

                    /*
                     * THE CHI2 CUT IS STILL IN UNDER STUDY
                     */

                    // if ( curPulse.pulse_Chi2/curPulse.pulse_NDF > 4 ) {
                    //     continue;
                    // }

                    if ( curPulse.hit.layer == layer_U_uRwell ) {
                        v_U_Pulses.push_back(curPulse);
                    }else if ( curPulse.hit.layer == layer_V_uRwell ) {
                        v_V_Pulses.push_back(curPulse);
                    }

                    if ( curPulse.pulse_Chi2/curPulse.pulse_NDF < chi2NDF_cut) {
                        h_Pulse_Integral_vs_MPV_uRwell2.Fill(curPulse.pulse_Integral, curPulse.pulse_MPV);
                        h_Pulse_Integral_vs_StartTime_uRwell2.Fill(curPulse.pulse_Integral, curPulse.pulse_MPV - curPulse.pulse_Sigma);
                    }
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

            h_N_V_vs_U_Pulses1.Fill(n_U_Pulses, n_V_Pulses);
            h_N_V_vs_U_PulsClustrs1.Fill(n_U_PulseClusters, n_V_PulseClusters);

            uRwellTools::PulseCluster Max_U_PulseCluster = uRwellTools::getMaxIntegralPulseCluster(v_U_PulseClusters, minHits);
            uRwellTools::PulseCluster Max_V_Pulsecluster = uRwellTools::getMaxIntegralPulseCluster(v_V_PulseClusters, minHits);

            bool has_U_AND_V_clusters =  !Max_U_PulseCluster.getPulses()->empty() && !Max_V_Pulsecluster.getPulses()->empty();

            uRwellCross crs_Max_Integral;

            if ( has_U_AND_V_clusters ) {
                crs_Max_Integral = uRwellCross(Max_U_PulseCluster.getClusterCenter(), Max_V_Pulsecluster.getClusterCenter());

                double crs_X = crs_Max_Integral.getX();
                double crs_Y = crs_Max_Integral.getY();

                double U_ClusterMPV = Max_U_PulseCluster.getClusterMPV();
                double V_ClusterMPV = Max_V_Pulsecluster.getClusterMPV();
                double U_SeedMPV = Max_U_PulseCluster.getSeedMPV();
                double V_SeedMPV = Max_V_Pulsecluster.getSeedMPV();
                double U_SeedSigma = Max_U_PulseCluster.getSeedSigma();
                double V_SeedSigma = Max_V_Pulsecluster.getSeedSigma();
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

                int U_slot = Max_U_PulseCluster.getPulses()->at(0).hit.slot;
                int V_slot = Max_V_Pulsecluster.getPulses()->at(0).hit.slot;

                h_Cross_YXc_MaxIntegral1.Fill(crs_X, crs_Y);
                h_Cross_YXc_WeightedClusterStartTimeDiff1.Fill(crs_X, crs_Y, dtCluster_StartTime_UV);
                h_Cross_V_vs_U_ClusterMPV1.Fill( U_ClusterMPV, V_ClusterMPV );
                h_Cross_V_vs_U_SeedMPV1.Fill(U_SeedMPV, V_SeedMPV);
                h_Cross_V_vs_U_ClusterStartTime1.Fill( U_ClusterStatTime, V_ClusterStartTime );
                h_Cross_V_vs_U_SeedStartTime1.Fill(U_SeedStartTime, V_SeedStartTime );

                h_Cross_ClusterTimeDiff1.Fill( dtCluster_UV );
                h_Cross_SeedTimeDiff1.Fill(dtSeed_UV);
                h_Cross_ClusterStartTimeDiff1.Fill( dtCluster_StartTime_UV );
                h_Cross_SeedStartTimeDiff1.Fill( dtSeed_StartTime_UV );

                h_Cross_V_vs_U_ClusterPulseIntegral1.Fill( U_ClusterPulseIntegral, V_ClusterPulseIntegral );
                h_Cross_V_vs_U_SeedPulseIntegral.Fill(U_SeedPulseIntegral, V_SeedPulseIntegral );

                h_Cross_ClusterTimeDiff_vs_U_clPulseIntegral1.Fill( U_ClusterPulseIntegral, dtCluster_UV );
                h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral1.Fill(U_ClusterPulseIntegral, dtCluster_StartTime_UV);

                h_Cross_SeedTimeDiff_vs_U_clPulseIntegral1.Fill(U_ClusterPulseIntegral, dtSeed_UV);
                h_Cross_SeedStartTimeDiff_vs_U_cl_PulseIntegral1.Fill(U_ClusterPulseIntegral, dtSeed_StartTime_UV);

                h_Cross_UCluster_Sigms_vs_Slot1.Fill(U_slot, U_ClusterSigma);
                h_Cross_VCluster_Sigms_vs_Slot1.Fill(V_slot, U_ClusterSigma);

                h_Cross_UCluster_Sgima_vs_Strip1.Fill(Max_U_PulseCluster.getClusterCenter(), U_ClusterSigma);
                h_Cross_VCluster_Sigma_vs_Strip1.Fill(Max_V_Pulsecluster.getClusterCenter(), V_ClusterSigma);

                bool goodpulseWidth = U_SeedSigma > PulseSigmaMin && V_SeedSigma > PulseSigmaMin;

                if (goodpulseWidth) {
                    h_Cross_SeedTimeDiff_vs_U_clPulseIntegral_GoodWidth1.Fill(U_ClusterPulseIntegral, dtSeed_UV);
                    h_Cross_SeedStartTimeDiff_vs_U_cl_PulseIntegral_GoodWidth1.Fill(U_ClusterPulseIntegral, dtSeed_StartTime_UV);
                }
            }
        }

    }catch (exception &e) {
        cerr<<e.what()<<endl;
    }

    gDirectory->Write();
    file_out->Close();


}