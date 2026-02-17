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

// ===== Hodo Tools ======
#include <XYHodoTools.h>
#include "XYHodoAnalyzer.h"

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

    sprintf(inputFile, "Skims/Skim_PulseFit_%d_%d.hipo", run, fnum);

    const int layer_U_uRwell = 1;
    const int layer_V_uRwell = 2;
    const int layer_X_GEM = 1;
    const int layer_Y_GEM = 2;
    const int sec_uRwell = 6;
    const int sec_GEM = 8;
    const double PulseSigmaMax = 5.2;
    const double PulseSigmaMin = 1.2;

    // === Hodoscope analysis cuts ===
    constexpr double T_OverThrCut = 20;
    constexpr double deltaT_Cut_Cross = 20;
    constexpr double deltaT_Cut_PMT12_Match = 20;

    const int minHits = 2;

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
    hipo::bank bXYHodoTDC(factory.getSchema("XYHODO::tdc"));

    auto *file_out = new TFile(Form("AnaPulseFits_%d_File_%d.root", run, fnum), "Recreate");

    TH2D h_Pulse_Integral_vs_MPV_uRwell1("h_Pulse_Integral_vs_MPV_uRwell1", "", 200, 0., 20000., 200, -5., 15);
    TH2D h_Pulse_Integral_vs_StartTime_uRwell1("h_Pulse_Integral_vs_StartTime_uRwell1", "", 200, 0., 20000., 200, -5., 15);
    TH2D h_Pulse_Integral_vs_MPV_uRwell2("h_Pulse_Integral_vs_MPV_uRwell2", "", 200, 0., 20000., 200, -5., 15);
    TH2D h_Pulse_Integral_vs_StartTime_uRwell2("h_Pulse_Integral_vs_StartTime_uRwell2", "", 200, 0., 20000., 200, -5., 15);
    TH2D h_Pulse_Integral_vs_Chi2NDF_uRwell1("h_Pulse_Integral_vs_Chi2NDF_uRwell1", "", 200, 0., 20000., 200, 0., 100.);

    TH2D h_N_V_vs_U_Pulses1("h_N_V_vs_U_Pulses1", "", 21, -0.5, 20.5, 21, -0.5, 20.5);
    TH2D h_N_V_vs_U_PulsClustrs1("h_N_V_vs_U_PulsClustrs1", "", 7, -0.5, 6.5, 7, -0.5, 6.5);

    TH2D h_Cross_YXc_MaxIntegral1("h_Cross_YXc_MaxIntegral1", "", 1000, -900., 900., 200, -500., 500.);
    TH2D h_Cross_YXc_MaxIntegrall_GoodPulseSigma1("h_Cross_YXc_MaxIntegrall_GoodPulseSigma1", "", 1000, -900., 900., 200, -500., 500.);
    TH2D h_Cross_YXc_MaxIntegrall_GoodPulseSigma_Weght_UStartTime1("h_Cross_YXc_MaxIntegrall_GoodPulseSigma_Weght_UStartTime1", "", 1000, -900., 900., 200, -500., 500.);
    TH2D h_Cross_YXc_MaxIntegrall_GoodPulseSigma_Weght_VStartTime1("h_Cross_YXc_MaxIntegrall_GoodPulseSigma_Weght_VStartTime1", "", 1000, -900., 900., 200, -500., 500.);
    TH2D h_Cross_YXc_MaxIntegrall_GoodPulseSigma_Weght_UClPulseIntegral1("h_Cross_YXc_MaxIntegrall_GoodPulseSigma_Weght_UClPulseIntegral1", "", 1000, -900., 900., 200, -500., 500.);
    TH2D h_Cross_YXc_MaxIntegrall_GoodPulseSigma_Weght_VClPulseIntegral1("h_Cross_YXc_MaxIntegrall_GoodPulseSigma_Weght_VClPulseIntegral1", "", 1000, -900., 900., 200, -500., 500.);
    TH2D h_Cross_YXc_MaxIntegrall_GoodPulseSigma_Weght_UClSize1("h_Cross_YXc_MaxIntegrall_GoodPulseSigma_Weght_UClSize1", "", 1000, -900., 900., 200, -500., 500.);
    TH2D h_Cross_YXc_MaxIntegrall_GoodPulseSigma_Weght_VClSize1("h_Cross_YXc_MaxIntegrall_GoodPulseSigma_Weght_VClSize1", "", 1000, -900., 900., 200, -500., 500.);

    TH2D h_Cross_YXc_WeightedClusterStartTimeDiff1("h_Cross_YXc_WeightedClusterStartTimeDiff1", "", 1000, -900., 900., 200, -500., 500.);

    TH2D h_Cross_V_vs_U_ClusterMPV1("h_Cross_V_vs_U_ClusterMPV1", "", 200, -5, 15., 200, -5., 15.);
    TH2D h_Cross_V_vs_U_SeedMPV1("h_Cross_V_vs_U_SeedMPV1", "", 200, -5, 15., 200, -5., 15.);
    TH2D h_Cross_V_vs_U_ClusterStartTime1("h_Cross_V_vs_U_ClusterStartTime1", "", 200, -5., 15., 200, -5., 15.);
    TH2D h_Cross_V_vs_U_SeedStartTime1("h_Cross_V_vs_U_SeedStartTime1", "", 200, -5., 15., 200, -5., 15.);

    TH1D h_Cross_ClusterTimeDiff1("h_Cross_ClusterTimeDiff1", "", 200, -5, 5);
    TH1D h_Cross_SeedTimeDiff1("h_Cross_SeedTimeDiff1", "", 200, -5, 5);
    TH1D h_Cross_ClusterStartTimeDiff1("h_Cross_ClusterStartTimeDiff1", "", 200, -5, 5);
    TH1D h_Cross_ClusterStartTimeTable_Diff1("h_Cross_ClusterStartTimeTable_Diff1", "", 200, -5., 5.);
    TH1D h_Cross_SeedStartTimeDiff1("h_Cross_SeedStartTimeDiff1", "", 200, -5, 5);
    TH1D h_Cross_SeedStartTimeTable_Diff1("h_Cross_SeedStartTimeTable_Diff1", "", 200, -5., 5.);

    TH2D h_Cross_V_vs_U_ClusterPulseIntegral1("h_Cross_V_vs_U_ClusterPulseIntegral1", "", 200, 0, 50000., 200, 0., 50000.);
    TH2D h_Cross_V_vs_U_SeedPulseIntegral("h_Cross_V_vs_U_SeedPulseIntegral", "", 200, 0, 50000., 200, 0., 50000.);

    TH2D h_Cross_ClusterTimeDiff_vs_U_clPulseIntegral1("h_Cross_ClusterTimeDiff_vs_U_clPulseIntegral1", "", 200, 0, 50000., 200, -5., 5.);
    TH2D h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral1("h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral1", "", 200, 0, 50000., 200, -5., 5.);
    TH2D h_Cross_ClusterTimeDiff_vs_U_clPulseIntegral_GoodWidth1("h_Cross_ClusterTimeDiff_vs_U_clPulseIntegral_GoodWidth1", "", 200, 0, 50000., 200, -5., 5.);
    TH2D h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral_GoodWidth1("h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral_GoodWidth1", "", 200, 0, 50000., 200, -5., 5.);
    TH2D h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral_GoodWidth_ActiveArea1("h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral_GoodWidth_ActiveArea1", "", 200, 0, 50000., 200, -5., 5.);
    TH2D h_Cross_SeedTimeDiff_vs_U_clPulseIntegral1("h_Cross_SeedTimeDiff_vs_U_clPulseIntegral1", "", 200, 0, 50000., 200, -5., 5.);
    TH2D h_Cross_SeedStartTimeDiff_vs_U_cl_PulseIntegral1("h_Cross_SeedStartTimeDiff_vs_U_cl_PulseIntegral1", "", 200, 0, 50000., 200, -5., 5.);
    TH2D h_Cross_SeedTimeDiff_vs_U_clPulseIntegral_GoodWidth1("h_Cross_SeedTimeDiff_vs_U_clPulseIntegral_GoodWidth1", "", 200, 0, 50000., 200, -5., 5.);
    TH2D h_Cross_SeedStartTimeDiff_vs_U_cl_PulseIntegral_GoodWidth1("h_Cross_SeedStartTimeDiff_vs_U_cl_PulseIntegral_GoodWidth1", "", 200, 0, 50000., 200, -5., 5.);
    TH1D h_Cross_ClusterStartTimeTable_Diff_GoodWidth1("h_Cross_ClusterStartTimeTable_Diff_GoodWidth1", "", 200, -5., 5.);
    TH1D h_Cross_SeedStartTimeTable_Diff_GoodWidth1("h_Cross_SeedStartTimeTable_Diff_GoodWidth1", "", 200, -5., 5.);
    TH2D h_Cross_ClusterStartTimeTable_Diff_vs_U_cl_PulseIntegral_GoodWidth1("h_Cross_ClusterStartTimeTable_Diff_vs_U_cl_PulseIntegral_GoodWidth1", "", 200, 0, 50000., 200, -5., 5.);
    TH2D h_Cross_SeedStartTimeTable_Diff_vs_U_cl_PulseIntegral1("h_Cross_SeedStartTimeTable_Diff_vs_U_cl_PulseIntegral1", "", 200, 0, 50000., 200, -5., 5.);

    TH2D h_Cross_UCluster_Sigms_vs_Slot1("h_Cross_UCluster_Sigms_vs_Slot1", "", 13, -0.5, 12.5, 200, 0., 5.);
    TH2D h_Cross_VCluster_Sigms_vs_Slot1("h_Cross_VCluster_Sigms_vs_Slot1", "", 13, -0.5, 12.5, 200, 0., 5.);

    TH2D h_Cross_UCluster_Sgima_vs_Strip1("h_Cross_UCluster_Sgima_vs_Strip1", "", 706, -0.5, 705.5, 200, 0., 5.);
    TH2D h_Cross_VCluster_Sigma_vs_Strip1("h_Cross_VCluster_Sgima_vs_Strip1", "", 706, -0.5, 705.5, 200, 0., 5.);

    TH1D h_U_StrpPulseSigma("h_U_StrpPulseSigma", "", uRwellTools::nMaxUStrip + 1, -0.5, uRwellTools::nMaxUStrip + 0.5);
    TH1D h_V_StrpPulseSigma("h_V_StrpPulseSigma", "", uRwellTools::nMaxVStrip + 1, -0.5, uRwellTools::nMaxVStrip + 0.5);


    // === Hodo related histograms ===
    TH1D h_Hodo_N_LR_MatchCrosses("h_Hodo_N_LR_MatchCrosses", "", 21, -0.5, 20.5);
    TH2D h_Hodo_XY_BarID0("h_Hodo_XY_BarID0", "", XYHodoTools::nShortBars + 1, -0.5, double(XYHodoTools::nShortBars) + 0.5, XYHodoTools::nLongBars + 1, -0.5, double(XYHodoTools::nLongBars) + 0.5);
    TH2D h_Hodo_XY_BarID1("h_Hodo_XY_BarID1", "", XYHodoTools::nShortBars + 1, -0.5, double(XYHodoTools::nShortBars) + 0.5, XYHodoTools::nLongBars + 1, -0.5, double(XYHodoTools::nLongBars) + 0.5);

    TH2D h_Hodo_XY_BarID_Tag1("h_Hodo_XY_BarID_Tag1", "", XYHodoTools::nShortBars + 1, -0.5, double(XYHodoTools::nShortBars) + 0.5, XYHodoTools::nLongBars + 1, -0.5, double(XYHodoTools::nLongBars) + 0.5);

    TH2D h_Cross_YXC_Max1_[XYHodoTools::nShortBars][XYHodoTools::nLongBars];
    TH2D h_Cross_Cl_St_Time_Diff_vs_U_cl_PulseInt_GoodWidth_ActiveArea1_[XYHodoTools::nShortBars][XYHodoTools::nLongBars];
    for (int is = 0; is < XYHodoTools::nShortBars; is++) {
        for (int il = 0; il < XYHodoTools::nLongBars; il++) {
            h_Cross_YXC_Max1_[is][il] = TH2D(Form("h_Cross_YXC_Max1_%d_%d", is, il), "", 1000, -900., 900., 200, -500., 500.);
            h_Cross_Cl_St_Time_Diff_vs_U_cl_PulseInt_GoodWidth_ActiveArea1_[is][il] = TH2D(Form("h_Cross_Cl_St_Time_Diff_vs_U_cl_PulseInt_GoodWidth_ActiveArea1_%d_%d", is, il), "", 200, 0, 50000., 200, -5., 5.);
        }
    }

    ifstream inp_stripPulseSigma(Form("Pars/Pulse_Sigmas_%d.dat", run));

    if ( inp_stripPulseSigma.good() ) {
        int ch;
        double sigma;
        while ( !inp_stripPulseSigma.eof() ) {
            inp_stripPulseSigma>>ch>>sigma;

            if ( ch < 1000 ) {
                h_U_StrpPulseSigma.SetBinContent(ch, sigma);
            }else {
                h_V_StrpPulseSigma.SetBinContent(ch - 1000, sigma);
            }

        }
    }


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
            event.getStructure(bXYHodoTDC);

                        /*
             * Analyzing Hodoscope data
             */
            XYHodoTools::XYHodoAnalyzer det0_analyzer(0);
            det0_analyzer.SetTOverThreshold(T_OverThrCut);
            det0_analyzer.SetCrossDeltaT(deltaT_Cut_Cross);
            det0_analyzer.SetPMTMatchTimeCut(deltaT_Cut_PMT12_Match);
            det0_analyzer.SetHitBank(bXYHodoTDC);

            det0_analyzer.AnalyzeEvent();

            int nLR_MatchedCross = det0_analyzer.LR_MatchedCrosses()->size();

            h_Hodo_N_LR_MatchCrosses.Fill(nLR_MatchedCross);

            if (nLR_MatchedCross > 0) {
                h_Hodo_XY_BarID0.Fill( (det0_analyzer.LR_MatchedCrosses()->at(0).first)->ShortBarId(), (det0_analyzer.LR_MatchedCrosses()->at(0).first)->LongBarId() );
            }

            for ( auto iCrs = 0; iCrs < nLR_MatchedCross; iCrs++ ) {
                int shortBarID = (det0_analyzer.LR_MatchedCrosses()->at(iCrs).first)->ShortBarId();
                int longBarID = (det0_analyzer.LR_MatchedCrosses()->at(iCrs).first)->LongBarId();
                h_Hodo_XY_BarID1.Fill( shortBarID, longBarID );
            }

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
            uRwellTools::PulseCluster Max_V_PulseCluster = uRwellTools::getMaxIntegralPulseCluster(v_V_PulseClusters, minHits);

            bool has_U_AND_V_clusters =  !Max_U_PulseCluster.getPulses()->empty() && !Max_V_PulseCluster.getPulses()->empty();

            uRwellCross crs_Max_Integral;

            /*
            * Selecting clean (unumbigous) Hodo hvents
            */

            if (nLR_MatchedCross == 1) {
                int shortBarID = (det0_analyzer.LR_MatchedCrosses()->at(0).first)->ShortBarId();
                int longBarID = (det0_analyzer.LR_MatchedCrosses()->at(0).first)->LongBarId();

                h_Hodo_XY_BarID_Tag1.Fill(shortBarID, longBarID);
            }


            if ( has_U_AND_V_clusters ) {
                crs_Max_Integral = uRwellCross(Max_U_PulseCluster.getClusterCenter(), Max_V_PulseCluster.getClusterCenter());

                double crs_X = crs_Max_Integral.getX();
                double crs_Y = crs_Max_Integral.getY();

                double U_ClusterMPV = Max_U_PulseCluster.getClusterMPV();
                double V_ClusterMPV = Max_V_PulseCluster.getClusterMPV();
                double U_SeedMPV = Max_U_PulseCluster.getSeedMPV();
                double V_SeedMPV = Max_V_PulseCluster.getSeedMPV();
                double U_SeedSigma = Max_U_PulseCluster.getSeedSigma();
                double V_SeedSigma = Max_V_PulseCluster.getSeedSigma();
                double U_ClusterSigma = Max_U_PulseCluster.getClusterSigma();
                double V_ClusterSigma = Max_V_PulseCluster.getClusterSigma();
                double U_ClusterSigma_table = h_U_StrpPulseSigma.GetBinContent( h_U_StrpPulseSigma.FindBin(Max_U_PulseCluster.getClusterCenter()) );
                double V_ClusterSigma_table = h_V_StrpPulseSigma.GetBinContent( h_V_StrpPulseSigma.FindBin(Max_V_PulseCluster.getClusterCenter()) );
                double U_SeedSigma_table = h_U_StrpPulseSigma.GetBinContent( h_U_StrpPulseSigma.FindBin(Max_U_PulseCluster.getSeedStrip() ) );
                double V_SeedSigma_table = h_V_StrpPulseSigma.GetBinContent( h_V_StrpPulseSigma.FindBin(Max_V_PulseCluster.getSeedStrip() ) );

                double U_ClusterStatTime = U_ClusterMPV - U_ClusterSigma;
                double V_ClusterStartTime = V_ClusterMPV - V_ClusterSigma;
                double U_SeedStartTime = U_SeedMPV - Max_U_PulseCluster.getSeedSigma();
                double V_SeedStartTime = V_SeedMPV - Max_V_PulseCluster.getSeedSigma();
                double U_ClusterStartTime_table = U_ClusterMPV - 1.*U_ClusterSigma_table;
                double V_ClusterStartTime_table = V_ClusterMPV - 1.*V_ClusterSigma_table;
                double U_SeedStartTime_table = U_SeedMPV - 1.*U_SeedSigma_table;
                double V_SeedStartTime_table = V_SeedMPV - 1.*V_SeedSigma_table;


                double dtCluster_UV = U_ClusterMPV - V_ClusterMPV;
                double dtSeed_UV = U_SeedMPV - V_SeedMPV;
                double dtCluster_StartTime_UV = U_ClusterStatTime - V_ClusterStartTime;
                double dtSeed_StartTime_UV = U_SeedStartTime - V_SeedStartTime;
                double dtCluster_StartTime_UV_table = U_ClusterStartTime_table - V_ClusterStartTime_table;
                double dtSeed_StartTime_UV_table = U_SeedStartTime_table - V_SeedStartTime_table;

                double U_ClusterPulseIntegral = Max_U_PulseCluster.getClusterPulseIntegral();
                double V_ClusterPulseIntegral = Max_V_PulseCluster.getClusterPulseIntegral();
                double U_SeedPulseIntegral = Max_U_PulseCluster.getSeedPulseIntegral();
                double V_SeedPulseIntegral = Max_V_PulseCluster.getSeedPulseIntegral();

                int U_slot = Max_U_PulseCluster.getPulses()->at(0).hit.slot;
                int V_slot = Max_V_PulseCluster.getPulses()->at(0).hit.slot;

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
                h_Cross_ClusterStartTimeTable_Diff1.Fill(dtCluster_StartTime_UV_table);
                h_Cross_SeedStartTimeTable_Diff1.Fill(dtSeed_StartTime_UV_table);

                h_Cross_V_vs_U_ClusterPulseIntegral1.Fill( U_ClusterPulseIntegral, V_ClusterPulseIntegral );
                h_Cross_V_vs_U_SeedPulseIntegral.Fill(U_SeedPulseIntegral, V_SeedPulseIntegral );

                h_Cross_ClusterTimeDiff_vs_U_clPulseIntegral1.Fill( U_ClusterPulseIntegral, dtCluster_UV );
                h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral1.Fill(U_ClusterPulseIntegral, dtCluster_StartTime_UV);

                h_Cross_SeedTimeDiff_vs_U_clPulseIntegral1.Fill(U_ClusterPulseIntegral, dtSeed_UV);
                h_Cross_SeedStartTimeDiff_vs_U_cl_PulseIntegral1.Fill(U_ClusterPulseIntegral, dtSeed_StartTime_UV);

                h_Cross_UCluster_Sigms_vs_Slot1.Fill(U_slot, U_ClusterSigma);
                h_Cross_VCluster_Sigms_vs_Slot1.Fill(V_slot, U_ClusterSigma);

                h_Cross_UCluster_Sgima_vs_Strip1.Fill(Max_U_PulseCluster.getClusterCenter(), U_ClusterSigma);
                h_Cross_VCluster_Sigma_vs_Strip1.Fill(Max_V_PulseCluster.getClusterCenter(), V_ClusterSigma);



                bool goodpulseWidth = U_SeedSigma > PulseSigmaMin && V_SeedSigma > PulseSigmaMin && U_SeedSigma < PulseSigmaMax && V_SeedSigma < PulseSigmaMax;
                bool IsInActiveArea = uRwellTools::IsInsideDetector(crs_X, crs_Y);

                if (goodpulseWidth) {
                    h_Cross_SeedTimeDiff_vs_U_clPulseIntegral_GoodWidth1.Fill(U_ClusterPulseIntegral, dtSeed_UV);
                    h_Cross_SeedStartTimeDiff_vs_U_cl_PulseIntegral_GoodWidth1.Fill(U_ClusterPulseIntegral, dtSeed_StartTime_UV);

                    h_Cross_ClusterTimeDiff_vs_U_clPulseIntegral_GoodWidth1.Fill( U_ClusterPulseIntegral, dtCluster_UV );
                    h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral_GoodWidth1.Fill(U_ClusterPulseIntegral, dtCluster_StartTime_UV);

                    h_Cross_ClusterStartTimeTable_Diff_GoodWidth1.Fill(dtCluster_StartTime_UV_table);
                    h_Cross_SeedStartTimeTable_Diff_GoodWidth1.Fill(dtSeed_StartTime_UV_table);
                    h_Cross_ClusterStartTimeTable_Diff_vs_U_cl_PulseIntegral_GoodWidth1.Fill(U_ClusterPulseIntegral, dtCluster_StartTime_UV_table);
                    h_Cross_SeedStartTimeTable_Diff_vs_U_cl_PulseIntegral1.Fill(U_SeedPulseIntegral, dtSeed_StartTime_UV_table);
                    h_Cross_YXc_MaxIntegrall_GoodPulseSigma1.Fill(crs_X, crs_Y);

                    h_Cross_YXc_MaxIntegrall_GoodPulseSigma_Weght_UStartTime1.Fill(crs_X, crs_Y, U_ClusterStatTime );
                    h_Cross_YXc_MaxIntegrall_GoodPulseSigma_Weght_VStartTime1.Fill(crs_X, crs_Y, V_ClusterStartTime );
                    h_Cross_YXc_MaxIntegrall_GoodPulseSigma_Weght_UClSize1.Fill(crs_X, crs_Y, Max_U_PulseCluster.getPulses()->size() );
                    h_Cross_YXc_MaxIntegrall_GoodPulseSigma_Weght_VClSize1.Fill(crs_X, crs_Y, Max_V_PulseCluster.getPulses()->size() );
                    h_Cross_YXc_MaxIntegrall_GoodPulseSigma_Weght_UClPulseIntegral1.Fill(crs_X, crs_Y, U_ClusterPulseIntegral );
                    h_Cross_YXc_MaxIntegrall_GoodPulseSigma_Weght_VClPulseIntegral1.Fill(crs_X, crs_Y, V_ClusterPulseIntegral );

                    if (IsInActiveArea) {
                        h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral_GoodWidth_ActiveArea1.Fill(U_ClusterPulseIntegral, dtCluster_StartTime_UV);

                    }
                }


                if ( nLR_MatchedCross == 1 ) {
                    int shortBarID = (det0_analyzer.LR_MatchedCrosses()->at(0).first)->ShortBarId();
                    int longBarID = (det0_analyzer.LR_MatchedCrosses()->at(0).first)->LongBarId();

                    h_Cross_YXC_Max1_[shortBarID][longBarID].Fill(crs_X, crs_Y);
                    if (IsInActiveArea) {
                        h_Cross_Cl_St_Time_Diff_vs_U_cl_PulseInt_GoodWidth_ActiveArea1_[shortBarID][longBarID].Fill(U_ClusterPulseIntegral, dtCluster_StartTime_UV);
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