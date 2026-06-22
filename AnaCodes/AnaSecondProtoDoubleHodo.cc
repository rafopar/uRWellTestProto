//
// Created by rafopar on 6/8/26.
//

// ===== Hipo headers =====
#include <dictionary.h>
#include <reader.h>
#include <writer.h>

#include <cxxopts.hpp>
#include "uRwellTools.h"

// ===== Hodo Tools ======
#include <XYHodoTools.h>
#include "XYHodoAnalyzer.h"

#include <TH1D.h>
#include <TH2D.h>
#include <TFile.h>

using namespace std;
using namespace uRwellTools;

struct HodoPixelHistos {
    TH2D *h_uRwellCross_YXc1;
    TH1D *h_DeltaStartTime_UV1;
    // add new histograms here only — nowhere else
};

void InitHodoPixelHistograms(HodoPixelHistos hCell[XYHodoTools::nShortBars][XYHodoTools::nLongBars]);

bool GoodPulse( uRwellTools::APV25Pulse & Pulse );
bool MonsterEvent( int nUhits, int nVhits); // Checks if number of U or V hits are more than certain threshold. usually those events are discarded

bool IsHodoPixelInsideuRwell(int indShort, int indLong ); // Makes sure the hodo pixel vertical projection intu uRwell is inside the uRwell.

int main(int argc, char **argv) {

    char inputFile[256];

    cxxopts::Options options("AnaPulseFits", "Performs clustering and also does analysis on cosmic data");

    options.add_options()("r,Run", "Run number", cxxopts::value<int>())("f,FileNo", "File number",
                                                                        cxxopts::value<int>());

    auto parsed_options = options.parse(argc, argv);

    int run = 0;
    int fnum = -1;

    if (parsed_options.count("Run")) {
        run = parsed_options["Run"].as<int>();
    } else {
        cout << "The run number is not provided. Exiting..." << endl;
        exit(1);
    }

    if (parsed_options.count("FileNo")) {
        fnum = parsed_options["FileNo"].as<int>();
    } else {
        cout << "The file number is not provided. Exiting..." << endl;
        exit(1);
    }

    sprintf(inputFile, "Skims/Skim_PulseFit_%d_%d.hipo", run, fnum);

    static constexpr double LandauScale = 0.180655;

    const int layer_U_uRwell = 1;
    const int layer_V_uRwell = 2;
    const int sec_uRwell = 6;
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

    auto *file_out = new TFile(Form("AnaSecondHodoDoubleHodo_%d_File_%d.root", run, fnum), "Recreate");

    HodoPixelHistos h_pixelHistos[XYHodoTools::nShortBars][XYHodoTools::nLongBars];
    InitHodoPixelHistograms(h_pixelHistos);


    TH1D h_nLR_Matches_Det0_1("h_nLR_Matches_Det0_1", "", 11, -0.5, 10.5);
    TH1D h_nLR_Matches_Det1_1("h_nLR_Matches_Det1_1", "", 11, -0.5, 10.5);

    TH2D h_ShortBarID_Det01_1("h_ShortBarID_Det01_1", "", XYHodoTools::nShortBars + 1, -0.5, XYHodoTools::nShortBars + 0.5, XYHodoTools::nShortBars + 1, -0.5, XYHodoTools::nShortBars + 0.5);
    TH2D h_LongBarID_Det01_1("h_LongBarID_Det01_1", "", XYHodoTools::nLongBars + 1, -0.5, XYHodoTools::nLongBars + 0.5, XYHodoTools::nLongBars + 1, -0.5, XYHodoTools::nLongBars + 0.5);

    TH2D h_Det0_Occupancy_vertTrk1("h_Det0_Occupancy_vertTrk1", "", XYHodoTools::nShortBars + 1, -0.5, XYHodoTools::nShortBars + 0.5, XYHodoTools::nLongBars + 1, -0.5, XYHodoTools::nLongBars + 0.5);
    TH2D h_Det0_Occupancy_Fiducial1("h_Det0_Occupancy_Fiducial1", "", XYHodoTools::nShortBars + 1, -0.5, XYHodoTools::nShortBars + 0.5, XYHodoTools::nLongBars + 1, -0.5, XYHodoTools::nLongBars + 0.5);

    TH2D h_Det0_Occupancy_vert_uRwell_Ucluster1("h_Det0_Occupancy_vert_uRwell_Ucluster1", "", XYHodoTools::nShortBars + 1, -0.5, XYHodoTools::nShortBars + 0.5, XYHodoTools::nLongBars + 1, -0.5, XYHodoTools::nLongBars + 0.5);
    TH2D h_Det0_Occupancy_vert_uRwell_Vcluster1("h_Det0_Occupancy_vert_uRwell_Vcluster1", "", XYHodoTools::nShortBars + 1, -0.5, XYHodoTools::nShortBars + 0.5, XYHodoTools::nLongBars + 1, -0.5, XYHodoTools::nLongBars + 0.5);
    TH2D h_Det0_Occupancy_vert_uRwell_UVcluster1("h_Det0_Occupancy_vert_uRwell_UVcluster1", "", XYHodoTools::nShortBars + 1, -0.5, XYHodoTools::nShortBars + 0.5, XYHodoTools::nLongBars + 1, -0.5, XYHodoTools::nLongBars + 0.5);

    TH2D h_Cross_YXc_MaxIntegral1("h_Cross_YXc_MaxIntegral1", "", 1000, -900., 900., 200, -500., 500.);
    TH2D h_Cross_YXc_MaxIntegralInsideDet1("h_Cross_YXc_MaxIntegralInsideDet1", "", 1000, -900., 900., 200, -500., 500.);
    TH2D h_Cross_YXc_MaxIntegral_vertTrk11("h_Cross_YXc_MaxIntegral_vertTrk1", "", 1000, -900., 900., 200, -500., 500.);
    TH2D h_Cross_YXc_MaxIntegral_Fiducial1("h_Cross_YXc_MaxIntegral_Fiducial1", "", 1000, -900., 900., 200, -500., 500.);
    TH2D h_Cross_YXC_Weighted_deltaT_StTime1("h_Cross_YXC_Weighted_deltaT_StTime1", "", 1000, -900., 900., 200, -500., 500.);
    TH2D h_Cross_YXC_Weighted_UClSize1("h_Cross_YXC_Weighted_UClSize1", "", 1000, -900., 900., 200, -500., 500.);
    TH2D h_Cross_YXC_Weighted_VClSize1("h_Cross_YXC_Weighted_VClSize1", "", 1000, -900., 900., 200, -500., 500.);
    TH2D h_Cross_YXC_Weighted_U_PulseIntegral1("h_Cross_YXC_Weighted_U_PulseIntegral1", "", 1000, -900., 900., 200, -500., 500.);
    TH2D h_Cross_YXC_Weighted_V_PulseIntegral1("h_Cross_YXC_Weighted_V_PulseIntegral1", "", 1000, -900., 900., 200, -500., 500.);



    // ---------------- U and V cluster property histograms --------------------------------------------
    // ---------------- They don't require cross, U(V) related histograms will be filled ---------------
    // ---------------- id U(V) cluster present --------------------------------------------------------
    TH1D h_UCl_Size1("h_UCl_Size1", "", 11, -0.5, 10.5);
    TH1D h_VCl_Size1("h_VCl_Size1", "", 11, -0.5, 10.5);
    TH1D h_U_PulseIntegral1("h_U_PulseIntegral1", "", 200, 0., 50000);
    TH1D h_V_PulseIntegral1("h_V_PulseIntegral1", "", 200, 0., 50000);
    TH1D h_U_PulseHeight1("h_U_PulseHeight1", "", 200, 0., 2500.);
    TH1D h_V_PulseHeight1("h_V_PulseHeight1", "", 200, 0., 2500.);

    // ----------------- Following histograms will be filled only when there is a cross ---------------
    TH1D h_UCl_Size2("h_UCl_Size2", "", 11, -0.5, 10.5);
    TH1D h_VCl_Size2("h_VCl_Size2", "", 11, -0.5, 10.5);
    TH1D h_U_PulseIntegral2("h_U_PulseIntegral2", "", 200, 0., 50000);
    TH1D h_V_PulseIntegral2("h_V_PulseIntegral2", "", 200, 0., 50000);
    TH1D h_U_PulseHeight2("h_U_PulseHeight2", "", 200, 0., 2500.);
    TH1D h_V_PulseHeight2("h_V_PulseHeight2", "", 200, 0., 2500.);


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

            XYHodoTools::XYHodoAnalyzer det0_analyzer(0);
            det0_analyzer.SetTOverThreshold(T_OverThrCut);
            det0_analyzer.SetCrossDeltaT(deltaT_Cut_Cross);
            det0_analyzer.SetPMTMatchTimeCut(deltaT_Cut_PMT12_Match);
            det0_analyzer.SetHitBank(bXYHodoTDC);

            XYHodoTools::XYHodoAnalyzer det1_analyzer(1);
            det1_analyzer.SetTOverThreshold(T_OverThrCut);
            det1_analyzer.SetCrossDeltaT(deltaT_Cut_Cross);
            det1_analyzer.SetPMTMatchTimeCut(deltaT_Cut_PMT12_Match);
            det1_analyzer.SetHitBank(bXYHodoTDC);

            det0_analyzer.AnalyzeEvent();
            det1_analyzer.AnalyzeEvent();

            int nLR_MatchesDet0 = det0_analyzer.GetNLRMatches();
            int nLR_MatchesDet1 = det1_analyzer.GetNLRMatches();

            h_nLR_Matches_Det0_1.Fill(nLR_MatchesDet0);
            h_nLR_Matches_Det1_1.Fill(nLR_MatchesDet1);

            bool cleanHodoEvent = nLR_MatchesDet0 == 1 && nLR_MatchesDet1 == 1;
            if ( !cleanHodoEvent ) {continue;}

            int det0_ShortBarID = (det0_analyzer.LR_MatchedCrosses()->at(0).first)->ShortBarId();
            int det0_LongBarID = (det0_analyzer.LR_MatchedCrosses()->at(0).first)->LongBarId();
            int det1_ShortBarID = (det1_analyzer.LR_MatchedCrosses()->at(0).first)->ShortBarId();
            int det1_LongBarID = (det1_analyzer.LR_MatchedCrosses()->at(0).first)->LongBarId();

            h_LongBarID_Det01_1.Fill( det0_LongBarID, det1_LongBarID );
            h_ShortBarID_Det01_1.Fill( det0_ShortBarID, det1_ShortBarID );

            bool fiducial_trk = IsHodoPixelInsideuRwell(det0_ShortBarID, det0_LongBarID) && IsHodoPixelInsideuRwell(det1_ShortBarID, det1_LongBarID);

            if (fiducial_trk) {
                h_Det0_Occupancy_Fiducial1.Fill( det0_ShortBarID, det0_LongBarID );
            }

            bool vertical_trk = TMath::Abs(det0_ShortBarID - det1_ShortBarID) <= 2 && TMath::Abs(det0_LongBarID - det1_LongBarID) <= 2;

            //if (!vertical_trk) {continue;}

            // ---- At this point we have a good vertical track, as a denominator for the efficiency we will have
            // ---- number of events in the Det0 pixel

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

                if ( !GoodPulse(curPulse) ){continue;}

                if ( curPulse.hit.sector == sec_uRwell ) {

                    if ( curPulse.hit.layer == layer_U_uRwell ) {
                        v_U_Pulses.push_back(curPulse);
                    }else if ( curPulse.hit.layer == layer_V_uRwell ) {
                        v_V_Pulses.push_back(curPulse);
                    }

                }

            }


            //       Number of U and V Pulses
            unsigned int n_U_Pulses = v_U_Pulses.size();
            unsigned int n_V_Pulses = v_V_Pulses.size();

            // --- We want to avoid these events where there is huge amount of hits in the uRwell.
            // --- Those are mostly caused by a bad noise, which is not direcly caused by the detector performance
            if (MonsterEvent(n_U_Pulses, n_U_Pulses)) {continue;}


            //       Forming U and V Clusters
            std::vector<uRwellTools::PulseCluster> v_U_PulseClusters = uRwellTools::getPulseClusters(v_U_Pulses);
            std::vector<uRwellTools::PulseCluster> v_V_PulseClusters = uRwellTools::getPulseClusters(v_V_Pulses);

            //       Number of U and V Clusters
            unsigned int n_U_PulseClusters = v_U_PulseClusters.size();
            unsigned int n_V_PulseClusters = v_V_PulseClusters.size();

            uRwellTools::PulseCluster Max_U_PulseCluster = uRwellTools::getMaxIntegralPulseCluster(v_U_PulseClusters, minHits);
            uRwellTools::PulseCluster Max_V_PulseCluster = uRwellTools::getMaxIntegralPulseCluster(v_V_PulseClusters, minHits);

            double startTime_U_cl = Max_U_PulseCluster.getClusterMPV() - Max_U_PulseCluster.getClusterSigma();
            double startTime_V_cl = Max_V_PulseCluster.getClusterMPV() - Max_V_PulseCluster.getClusterSigma();

            double deltaT_StartTime = startTime_U_cl - startTime_V_cl;
            int U_ClSize = Max_U_PulseCluster.getPulses()->size();
            int V_ClSize = Max_V_PulseCluster.getPulses()->size();
            double U_ClPulseIntegral = Max_U_PulseCluster.getClusterPulseIntegral();
            double V_ClPulseIntegral = Max_V_PulseCluster.getClusterPulseIntegral();

            double U_ClSeadPeakHeight = Max_U_PulseCluster.getSeedPulse().pulse_A0*LandauScale;
            double V_ClSeadPeakHeight = Max_V_PulseCluster.getSeedPulse().pulse_A0*LandauScale;

            bool has_U_cluster = !Max_U_PulseCluster.getPulses()->empty();
            bool has_V_cluster = !Max_V_PulseCluster.getPulses()->empty();
            bool has_U_AND_V_clusters =  has_U_cluster && has_V_cluster;


            if ( vertical_trk ) {
                h_Det0_Occupancy_vertTrk1.Fill(det0_ShortBarID, det0_LongBarID);

                if ( has_U_cluster ) {
                    h_Det0_Occupancy_vert_uRwell_Ucluster1.Fill(det0_ShortBarID, det0_LongBarID);
                    h_UCl_Size1.Fill(U_ClSize);
                    h_U_PulseIntegral1.Fill(U_ClPulseIntegral);
                    h_U_PulseHeight1.Fill(U_ClSeadPeakHeight);
                }
                if ( has_V_cluster ) {
                    h_Det0_Occupancy_vert_uRwell_Vcluster1.Fill(det0_ShortBarID, det0_LongBarID);
                    h_VCl_Size1.Fill(V_ClSize);
                    h_V_PulseIntegral1.Fill(V_ClPulseIntegral);
                    h_V_PulseHeight1.Fill(V_ClSeadPeakHeight);
                }
            }
            uRwellCross crs_Max_Integral;



            if (has_U_AND_V_clusters) {

                crs_Max_Integral = uRwellCross(Max_U_PulseCluster.getClusterCenter(), Max_V_PulseCluster.getClusterCenter());


                double crs_X = crs_Max_Integral.getX();
                double crs_Y = crs_Max_Integral.getY();

                bool isInsiDeDetector = uRwellTools::IsInsideDetector(crs_X, crs_Y);

                h_Cross_YXc_MaxIntegral1.Fill(crs_X, crs_Y);
                if (!isInsiDeDetector) {continue;}

                h_Cross_YXc_MaxIntegralInsideDet1.Fill(crs_X, crs_Y);
                if ( fiducial_trk ) {
                    h_Cross_YXc_MaxIntegral_Fiducial1.Fill(crs_X, crs_Y);
                }

                if (vertical_trk) {
                    h_Det0_Occupancy_vert_uRwell_UVcluster1.Fill(det0_ShortBarID, det0_LongBarID);

                    h_UCl_Size2.Fill(U_ClSize);
                    h_U_PulseIntegral2.Fill(U_ClPulseIntegral);
                    h_U_PulseHeight2.Fill(U_ClSeadPeakHeight);
                    h_VCl_Size2.Fill(V_ClSize);
                    h_V_PulseIntegral2.Fill(V_ClPulseIntegral);
                    h_V_PulseHeight2.Fill(V_ClSeadPeakHeight);

                    h_Cross_YXc_MaxIntegral_vertTrk11.Fill(crs_X, crs_Y);
                    h_pixelHistos[det0_ShortBarID][det0_LongBarID].h_uRwellCross_YXc1->Fill(crs_X, crs_Y);
                    h_pixelHistos[det0_ShortBarID][det0_LongBarID].h_DeltaStartTime_UV1->Fill(deltaT_StartTime);

                    h_Cross_YXC_Weighted_deltaT_StTime1.Fill(crs_X, crs_Y, deltaT_StartTime);
                    h_Cross_YXC_Weighted_UClSize1.Fill(crs_X, crs_Y, U_ClSize );
                    h_Cross_YXC_Weighted_VClSize1.Fill(crs_X, crs_Y, V_ClSize );
                    h_Cross_YXC_Weighted_U_PulseIntegral1.Fill(crs_X, crs_Y, U_ClPulseIntegral );
                    h_Cross_YXC_Weighted_V_PulseIntegral1.Fill(crs_X, crs_Y, V_ClPulseIntegral );
                }
            }

        }
    }catch (exception &e) {
        cerr << e.what() << endl << endl;
    }

    gDirectory->Write();
    file_out->Close();
}

void InitHodoPixelHistograms(HodoPixelHistos hCell[XYHodoTools::nShortBars][XYHodoTools::nLongBars]) {

    for ( int is = 0; is < XYHodoTools::nShortBars; is++ ) {
        for (int il = 0; il < XYHodoTools::nLongBars; il++ ) {
            std::string s = "_" + std::to_string(is) + "_" + std::to_string(il);
            hCell[is][il].h_uRwellCross_YXc1 = new TH2D( ("h_uRwell_Cross_YXc1" + s).c_str(), "; Cross X [mm]; Cross Y [mm]", 1000, -900., 900., 200, -500., 500.);
            hCell[is][il].h_DeltaStartTime_UV1 = new TH1D(("h_DeltaStartTime_UV1" + s).c_str(), "", 100, -8, 8.);
        }
    }

}


bool GoodPulse( uRwellTools::APV25Pulse & pulse ) {

    static constexpr double MaxSigma = 5.2;
    static constexpr double MinSigma = 1.;
    static constexpr double MPVMax = 15.;
    static constexpr double MPVMin = 0.;

    return pulse.pulse_Sigma > MinSigma && pulse.pulse_Sigma < MaxSigma && pulse.pulse_MPV > MPVMin && pulse.pulse_MPV < MPVMax;

}

bool MonsterEvent( int nUhits, int nVhits) {
    static constexpr int nMaxUHits = 15;
    static constexpr int nMaxVHits = 15;

    return nUhits > nMaxUHits || nVhits > nMaxVHits;
}

bool IsHodoPixelInsideuRwell(int indShort, int indLong ) {
    static constexpr double ilongMax = 9;
    static constexpr double ilongMin = 2;

    if (indLong < ilongMin || indLong > ilongMax) {return false;}

    static const int ishortMax[XYHodoTools::nLongBars] = {0, 0, 26, 27, 27, 28, 28, 29, 29, 30, 0, 0};
    static const int ishortMin[XYHodoTools::nLongBars] = {0,0, 6, 6, 5, 5, 4, 4, 4, 4, 0, 0};

    return indShort >= ishortMin[indLong] && indShort <= ishortMax[indLong];
}
