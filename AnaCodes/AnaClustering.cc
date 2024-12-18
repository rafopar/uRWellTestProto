/* 
 * File:   TestClustering.cc
 * Author: rafopar
 *
 * Created on May 5, 2023, 4:53 PM
 */

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

int main(int argc, char** argv) {

    char outputFile[256];
    char inputFile[256];


    cxxopts::Options options("AnaClustering", "Performs clustering and also does analysis on cosmic data");

    options.add_options()
            ("r,Run", "Run number", cxxopts::value<int>())
            ("f,FileNo", "File number", cxxopts::value<int>())
            ("t,Threshold", "Hit Threshold in terms of sigma", cxxopts::value<double>()->default_value("5"))
            ("m,MinHits", "Number of minimum hits in the cluster", cxxopts::value<int>()->default_value("1"))
            //("f,File", "File number of the given run", cxxopts::value<int>())
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

    const double HitThr = parsed_options["Threshold"].as<double>();
    if (!parsed_options.count("Threshold")) {
        cout << "* You didn't provide the hit threshold, will use the default value " << HitThr << endl;
    }

    const int MinClSize = parsed_options["MinHits"].as<int>();
    if (!parsed_options.count("MinHits")) {
        cout << "* You didn't provide the Minimum hits int the cluster, so will use the default value " << MinClSize << endl;
    }

    cout << "The hit threshold is " << HitThr << "\\sigma" << endl;
    cout << "The Minimum cluster size is " << MinClSize << "hits" << endl;

    
    /*
     *  Here is the input file
     */
    sprintf(inputFile, "Skim_ZeroSuppr_%d_%d.hipo", run, fnum);
    
    hipo::reader reader;
    reader.open(inputFile);

    hipo::dictionary factory;

    reader.readDictionary(factory);

    factory.show();
    hipo::event event;
    int evCounter = 0;

    const double GEM_LargeSignal_Thr = 8.;
    const int nMaxGroup = 11;
    const int n_ts = 9;
    const int n_MaxClusters = 5; // If the number of clusters is more than this number, then the entire event will be dropped
    const double highADCCut = 800; //

    const int layer_U_uRwell = 1;
    const int layer_V_uRwell = 2;
    const int layer_X_GEM = 1;
    const int layer_Y_GEM = 2;
    const int sec_uRwell = 6;
    const int sec_GEM = 8;
    const double GEMHighThreshold = 10.; // 10 Sigma
    const double slot11_StripMin = 577.;
    const double GEM_strip2Coord = 10. / 256;

    hipo::bank buRwellHit(factory.getSchema("uRwell::Hit"));
    hipo::bank bRAWADc(factory.getSchema("RAW::adc"));
    hipo::bank bRunConf(factory.getSchema("RUN::config"));

    TFile *file_out = new TFile(Form("AnaClustering_%d_Thr_%1.1f_MinHits_%d_File_%d.root", run, HitThr, MinClSize, fnum), "Recreate");
    TH2D h_n_GEM_vs_uRwellHits("h_n_GEM_vs_uRwellHits", "", 31, -0.5, 30, 31, -0.5, 30);
    TH2D h_n_GEM_Y_vs_X_Clusters1("h_n_GEM_Y_vs_X_Clusters1", "", 11, -0.5, 10.5, 11, -0.5, 10.5);
    TH2D h_n_V_vs_U_hits1("h_n_V_vs_U_hits1", "", 26, -0.5, 25.5, 26, -0.5, 25.5);

    TH1D h_U_HitStrip1("h_U_HitStrip1", "", 706, -0.5, 705.5);
    TH1D h_U_HitStrip2("h_U_HitStrip2", "", 706, -0.5, 705.5);
    TH1D h_V_HitStrip1("h_V_HitStrip1", "", 706, -0.5, 705.5);
    TH1D h_V_HitStrip2("h_V_HitStrip2", "", 706, -0.5, 705.5);

    TH2D h_n_V_vs_U_Clusters1("h_n_V_vs_U_Clusters1", "", 26, -0.5, 25.5, 26, -0.5, 25.5);
    TH1D h_n_UclHits1("h_n_UclHits1", "", 21, -0.5, 20.5);
    TH1D h_n_VclHits1("h_n_VclHits1", "", 21, -0.5, 20.5);

    TH2D h_n_GEM_Y_vs_X_Hits1("h_n_GEM_Y_vs_X_Hits1", "", 21, -0.5, 20.5, 21, -0.5, 20.5);
    TH2D h_n_uRwell_V_vs_U_MultiHitCl("h_n_uRwell_V_vs_U_MultiHitCl", "", 21, -0.5, 20.5, 21, -0.5, 20.5);
    TH1D h_U_Coord_MultrCl1("h_U_Coord_MultrCl1", "", 200, 0., 750);
    TH1D h_V_Coord_MultrCl1("h_V_Coord_MultrCl1", "", 200, 0., 750);
    TH1D h_U_Coord_MultrCl2("h_U_Coord_MultrCl2", "", 200, 0., 750);
    TH1D h_V_Coord_MultrCl2("h_V_Coord_MultrCl2", "", 200, 0., 750);
    TH1D h_U_PeakADC_MultiCl1("h_U_PeakADC_MultiCl1", "", 200, 0., 1000.);
    TH1D h_V_PeakADC_MultiCl1("h_V_PeakADC_MultiCl1", "", 200, 0., 1000.);
    TH1D h_U_Coord_SingleHitCl1("h_U_Coord_SingleHitCl1", "", 200, 0., 750);
    TH1D h_V_Coord_SingleHitCl1("h_V_Coord_SingleHitCl1", "", 200, 0., 750);

    TH2D h_Cross_YXc1("h_Cross_YXc1", "", 1000, -900., 900., 200, -500., 500.);
    TH2D h_Cross_YXc2("h_Cross_YXc2", "", 200, -900., 900., 200, -500., 500.);
    TH2D h_Cross_YXc3("h_Cross_YXc3", "", 1000, -900., 900., 200, -500., 500.);

    TH2D h_Cross_YXc_Max1("h_Cross_YXc_Max1", "", 1000, -900., 900., 200, -500., 500.);
    TH2D h_Cross_YXc_Max2("h_Cross_YXc_Max2", "", 1000, -900., 900., 200, -500., 500.);
    TH2D h_Cross_YXc_Max3("h_Cross_YXc_Max3", "", 1000, -900., 900., 200, -500., 500.);
    TH2D h_Cross_YXc_Max4("h_Cross_YXc_Max4", "", 1000, -900., 900., 200, -500., 500.);
    TH2D h_Cross_YXc_Max5("h_Cross_YXc_Max5", "", 1000, -900., 900., 200, -500., 500.);
    TH2D h_Cross_YXc_Max_InsideDet1("h_Cross_YXc_Max_InsideDet1", "", 1000, -900., 900., 200, -500., 500.);

    TH2D h_Cross_YXc_Max_Weighted1("h_Cross_YXc_Max_Weighted1", "", 1000, -900., 900., 200, -500., 500.);
    TH2D h_Cross_YXc_Max_Weighted_ADC_U1("h_Cross_YXc_Max_Weighted_ADC_U1", "", 1000, -900., 900., 200, -500., 500.);
    TH2D h_Cross_YXc_Max_Weighted_ADC_V1("h_Cross_YXc_Max_Weighted_ADC_V1", "", 1000, -900., 900., 200, -500., 500.);
    TH2D h_Cross_YXc_Max_Weighted_tU1("h_Cross_YXc_Max_Weighted_tU1", "", 1000, -900., 900., 200, -500., 500.);
    TH2D h_Cross_YXc_Max_Weighted_tV1("h_Cross_YXc_Max_Weighted_tV1", "", 1000, -900., 900., 200, -500., 500.);
    TH2D h_Cross_YXc_Max_Weighted_clUSize1("h_Cross_YXc_Max_Weighted_clUSize1", "", 1000, -900., 900., 200, -500., 500.);
    TH2D h_Cross_YXc_Max_Weighted_clVSize1("h_Cross_YXc_Max_Weighted_clVSize1", "", 1000, -900., 900., 200, -500., 500.);

    TH2D h_Cross_YXc_Max_Weighted_TotEnergy1("h_Cross_YXc_Max_Weighted_TotEnergy1", "", 1000, -900., 900., 200, -500., 500.);
    TH2D h_Cross_YXc_Max_Weighted_Energy_U1("h_Cross_YXc_Max_Weighted_Energy_U1", "", 1000, -900., 900., 200, -500., 500.);
    TH2D h_Cross_YXc_Max_Weighted_Energy_V1("h_Cross_YXc_Max_Weighted_Energy_V1", "", 1000, -900., 900., 200, -500., 500.);

    TH1D h_ADC_U_Max2("h_ADC_U_Max2", "", 200, 0., 1500.);
    TH1D h_ADC_V_Max2("h_ADC_V_Max2", "", 200, 0., 1500.);

    /* 
     * Noise related histograms
     */

    TH2D h_clV_clU_size_NOISE_1("h_clV_clU_size_NOISE_1", "", 15, -0.5, 14.5, 15, -0.5, 14.5);
    TH2D h_clV_clU_time_NOISE_1("h_clV_clU_time_NOISE_1", "", n_ts + 1, -0.5, double(n_ts) + 0.5, n_ts + 1, -0.5, double(n_ts) + 0.5);
    TH2D h_clV_clU_ADC_NOISE_1("h_clV_clU_ADC_NOISE_1", "", 200, 0., 1600, 200, 0., 1600);
    TH2D h_N_Cl_V_U_NOISE_1("h_N_Cl_V_U_NOISE_1", "", 25, -0.5, 24.5, 25, -0.5, 24.5);

    TH2D h_clV_clU_size1("h_clV_clU_size1", "", 15, -0.5, 14.5, 15, -0.5, 14.5);

    TH2D h_time_ADC_U_Max1("h_time_ADC_U_Max1", "", 200, 0., 1500., n_ts + 1, -0.5, double(n_ts) + 0.5);
    TH2D h_time_ADC_V_Max1("h_time_ADC_V_Max1", "", 200, 0., 1500., n_ts + 1, -0.5, double(n_ts) + 0.5);
    TH2D h_t_V_vs_U_Max1("h_t_V_vs_U_Max1", "", n_ts + 1, -0.5, double(n_ts) + 0.5, n_ts + 1, -0.5, double(n_ts) + 0.5);

    TH2D h_cl_V_U_Energy_Max1("h_cl_V_U_Energy_Max1", "", 200, 0., 3500, 200, 0., 3500);

    TH2D h_Cross_YXc_Weighted1("h_Cross_YXc_Weighted1", "", 1000, -900., 900., 200, -500., 500.);
    TH2D h_Cross_YXc_Weighted2("h_Cross_YXc_Weighted2", "", 1000, -900., 900., 200, -500., 500.);
    TH2D h_Cross_YXc_Weighted3("h_Cross_YXc_Weighted3", "", 1000, -900., 900., 200, -500., 500.);

    TH2D h_GEM_XY1("h_GEM_XY1", "", 129, -0.5, 128.5, 129, -0.5, 128.5);
    TH2D h_GEM_cl_YXC1("h_GEM_cl_YXC1", "", 100, -0.5, 10.5, 200, -0.5, 10.5);
    TH2D h_GEM_cl_YXC_MAX1("h_GEM_cl_YXC1_MAX1", "", 100, -0.5, 10.5, 200, -0.5, 10.5);
    TH2D h_GEM_cl_YXC_MAX_Weighted_ADC_Tot1("h_GEM_cl_YXC_MAX_Weighted_ADC_Tot1", "", 100, -0.5, 10.5, 200, -0.5, 10.5);
    TH2D h_GEM_cl_YXC_MAX_Weighted_ADC_Y1("h_GEM_cl_YXC_MAX_Weighted_ADC_Y1", "", 100, -0.5, 10.5, 200, -0.5, 10.5);
    TH2D h_GEM_cl_YXC_MAX_Weighted_ADC_X1("h_GEM_cl_YXC_MAX_Weighted_ADC_X1", "", 100, -0.5, 10.5, 200, -0.5, 10.5);

    TH2D h_GEM_tY_vs_tX_Max1("h_GEM_tY_vs_tX_Max1", "", n_ts + 1, -0.5, double(n_ts) + 0.5, n_ts + 1, -0.5, double(n_ts) + 0.5);
    TH2D h_GEM_ADCY_vs_ADCX_Max1("h_GEM_ADCY_vs_ADCX_Max1", "", 200, 0., 1500, 200, 0., 1500);
    TH2D h_GEM_clSize_Y_vs_X_Max1("h_GEM_clSize_Y_vs_X_Max1", "", 11, -0.5, 10.5, 11, -0.5, 10.5);

    TH1D h_ADC_GEM_Y1("h_ADC_GEM_Y1", "", 200, 0., 1000.);
    TH1D h_ADC_GEM_X1("h_ADC_GEM_X1", "", 200, 0., 1000.);

    TH1D h_MaxADC_GEM_Y1("h_MaxADC_GEM_Y1", "", 200, 0., 1000.);
    TH1D h_MaxADC_GEM_X1("h_MaxADC_GEM_X1", "", 200, 0., 1000.);

    TH2D h_nVcl_vs_Ucoord1("h_nVcl_vs_Ucoord1", "", 200, 0., 710., 15, -0.5, 14.5);
    TH2D h_nUcl_vs_Vcoord1("h_nUcl_vs_Vcoord1", "", 200, 0., 710., 15, -0.5, 14.5);

    TH2D h_ADC_vs_U_Max1("h_ADC_vs_U_Max1", "", 705, -0.5, 704.5, 200, 0., 1600);
    TH2D h_ADC_vs_V_Max1("h_ADC_vs_V_Max1", "", 705, -0.5, 704.5, 200, 0., 1600);
    TH2D h_ADC_vs_U_Max2("h_ADC_vs_U_Max2", "", 705, -0.5, 704.5, 200, 0., 1600);
    TH2D h_ADC_vs_V_Max2("h_ADC_vs_V_Max2", "", 705, -0.5, 704.5, 200, 0., 1600);

    TH2D h_Cross_YXx1_[nMaxGroup];
    TH2D h_ADC_V_U1_[nMaxGroup];
    TH2D h_nV_U1_[nMaxGroup];

    TH2D h_Cross_YXx_Max1_[nMaxGroup];
    TH2D h_ADC_V_U_Max1_[nMaxGroup];
    TH2D h_nV_U_Max1_[nMaxGroup];

    for (int i = 0; i < nMaxGroup; i++) {
        h_Cross_YXx1_[i] = TH2D(Form("h_Cross_YXx1_%d", i), "", 1000, -900., 900., 200, -500., 500.);
        h_ADC_V_U1_[i] = TH2D(Form("h_ADC_V_U1_%d", i), "", 200, 0., 1500., 200., 0., 1500);
        h_nV_U1_[i] = TH2D(Form("h_nV_U1_%d", i), "", 15, -0.5, 14.5, 15, -0.5, 14.5);

        h_Cross_YXx_Max1_[i] = TH2D(Form("h_Cross_YXx_Max1_%d", i), "", 1000, -900., 900., 200, -500., 500.);
        h_ADC_V_U_Max1_[i] = TH2D(Form("h_ADC_V_U_Max1_%d", i), "", 200, 0., 1500., 200., 0., 1500);
        h_nV_U_Max1_[i] = TH2D(Form("h_nV_U_Max1_%d", i), "", 15, -0.5, 14.5, 15, -0.5, 14.5);
    }


    try {

        while (reader.next() == true) {
            reader.read(event);

            evCounter = evCounter + 1;

            //if( evCounter > 2000 ){break;}
            if (evCounter % 1000 == 0) {
                cout.flush() << "Processed " << evCounter << " events \r";
            }

            event.getStructure(buRwellHit);
            event.getStructure(bRAWADc);
            event.getStructure(bRunConf);

            int n_TrkHits = buRwellHit.getRows();

            vector<uRwellHit> v_uRwellHits;
            vector<uRwellHit> v_U_Hits_uRwell;
            vector<uRwellHit> v_V_Hits_uRwell;
            vector<uRwellHit> v_GEMHits;
            vector<uRwellHit> v_X_GEMHits;
            vector<uRwellHit> v_Y_GEMHits;

            for (int ihit = 0; ihit < n_TrkHits; ihit++) {
                uRwellHit curHit;
                curHit.sector = buRwellHit.getInt("sec", ihit);
                curHit.layer = buRwellHit.getInt("layer", ihit);
                curHit.strip = buRwellHit.getInt("strip", ihit);
                curHit.stripLocal = buRwellHit.getInt("stripLocal", ihit);
                curHit.adc = double(buRwellHit.getFloat("adc", ihit));
                curHit.adcRel = double(buRwellHit.getFloat("adcRel", ihit));
                curHit.slot = buRwellHit.getInt("slot", ihit);
                curHit.ts = buRwellHit.getInt("ts", ihit);

                if (curHit.sector == sec_uRwell) {

                    if (curHit.adcRel < HitThr) {
                        continue;
                    }

                    v_uRwellHits.push_back(curHit);

                    if (curHit.layer == layer_U_uRwell) {
                        v_U_Hits_uRwell.push_back(curHit);
                    } else if (curHit.layer == layer_V_uRwell) {
                        v_V_Hits_uRwell.push_back(curHit);
                    }

                } else if (curHit.sector == sec_GEM) {

                    if (curHit.adcRel < GEM_LargeSignal_Thr) {
                        continue;
                    }

                    v_GEMHits.push_back(curHit);

                    if (curHit.layer == layer_X_GEM) {
                        v_X_GEMHits.push_back(curHit);
                    } else if (curHit.layer == layer_Y_GEM) {
                        v_Y_GEMHits.push_back(curHit);
                    }
                }
            }

            h_n_GEM_vs_uRwellHits.Fill(v_uRwellHits.size(), v_GEMHits.size());
            h_n_V_vs_U_hits1.Fill(v_U_Hits_uRwell.size(), v_V_Hits_uRwell.size());

            vector<uRwellCluster> v_U_Clusters = uRwellTools::getGlusters(v_U_Hits_uRwell);
            vector<uRwellCluster> v_V_Clusters = uRwellTools::getGlusters(v_V_Hits_uRwell);

            vector<uRwellCluster> v_Y_GEM_Clusters = uRwellTools::getGlusters(v_Y_GEMHits);
            vector<uRwellCluster> v_X_GEM_Clusters = uRwellTools::getGlusters(v_X_GEMHits);

            uRwellCluster GEM_Max_Y_Cluster = uRwellTools::getMaxAdcCluster(v_Y_GEM_Clusters, 2);
            uRwellCluster GEM_Max_X_Cluster = uRwellTools::getMaxAdcCluster(v_X_GEM_Clusters, 2);

            if (GEM_Max_Y_Cluster.getHits()->size() > 0 && GEM_Max_X_Cluster.getHits()->size() > 0) {
                double GEM_X = GEM_Max_X_Cluster.getAvgStrip() * GEM_strip2Coord;
                double GEM_Y = GEM_Max_Y_Cluster.getAvgStrip() * GEM_strip2Coord;

                int t_X = GEM_Max_X_Cluster.getPeakTime();
                int t_Y = GEM_Max_Y_Cluster.getPeakTime();

                int cl_size_X = GEM_Max_X_Cluster.getHits()->size();
                int cl_size_Y = GEM_Max_Y_Cluster.getHits()->size();

                double ADC_X = GEM_Max_X_Cluster.getPeakADC();
                double ADC_Y = GEM_Max_Y_Cluster.getPeakADC();

                h_GEM_cl_YXC_MAX1.Fill(GEM_X, GEM_Y);
                h_GEM_tY_vs_tX_Max1.Fill(t_X, t_Y);
                h_GEM_ADCY_vs_ADCX_Max1.Fill(ADC_X, ADC_Y);
                h_GEM_clSize_Y_vs_X_Max1.Fill(cl_size_X, cl_size_Y);

                h_GEM_cl_YXC_MAX_Weighted_ADC_Tot1.Fill(GEM_X, GEM_Y, ADC_X + ADC_Y);
                h_GEM_cl_YXC_MAX_Weighted_ADC_X1.Fill(GEM_X, GEM_Y, ADC_X);
                h_GEM_cl_YXC_MAX_Weighted_ADC_Y1.Fill(GEM_X, GEM_Y, ADC_Y);
            }


            for (auto curHit : v_U_Hits_uRwell) {

                h_U_HitStrip1.Fill(curHit.strip);

                if (curHit.adcRel > 5) {
                    h_U_HitStrip2.Fill(curHit.strip);
                }
            }

            for (auto curHit : v_V_Hits_uRwell) {

                h_V_HitStrip1.Fill(curHit.strip);

                if (curHit.adcRel > 5) {
                    h_V_HitStrip2.Fill(curHit.strip);
                }
            }

            int n_U_cl = v_U_Clusters.size();
            int n_V_cl = v_V_Clusters.size();
            h_n_V_vs_U_Clusters1.Fill(n_U_cl, n_V_cl);

            int n_U_MultiHit_clusters = 0;
            int n_V_MultiHit_clusters = 0;

            uRwellCluster Max_UCluster = uRwellTools::getMaxAdcCluster(v_U_Clusters, MinClSize);
            uRwellCluster Max_VCluster = uRwellTools::getMaxAdcCluster(v_V_Clusters, MinClSize);
            uRwellCross curCrs_MaxADC;

            double U_Cl_ADC_Max = 0;
            double V_Cl_ADC_Max = 0;
            int max_Ucl_ind = 0;
            int max_Vcl_ind = 0;

            for (int iUcl = 0; iUcl < v_U_Clusters.size(); iUcl++) {
                int nhits = v_U_Clusters.at(iUcl).getHits()->size();
                h_n_UclHits1.Fill(nhits);

                if (nhits < MinClSize) {
                    continue;
                }

                if (nhits >= MinClSize) {
                    h_U_Coord_MultrCl1.Fill(v_U_Clusters.at(iUcl).getAvgStrip());
                    h_U_PeakADC_MultiCl1.Fill(v_U_Clusters.at(iUcl).getPeakADC());
                    n_U_MultiHit_clusters++;
                } else {
                    h_U_Coord_SingleHitCl1.Fill(v_U_Clusters.at(iUcl).getAvgStrip());
                }
            }

            for (int iVcl = 0; iVcl < v_V_Clusters.size(); iVcl++) {
                int nhits = v_V_Clusters.at(iVcl).getHits()->size();
                h_n_VclHits1.Fill(nhits);

                if (nhits < MinClSize) {
                    continue;
                }


                if (nhits >= MinClSize) {
                    h_V_Coord_MultrCl1.Fill(v_V_Clusters.at(iVcl).getAvgStrip());
                    h_V_PeakADC_MultiCl1.Fill(v_V_Clusters.at(iVcl).getPeakADC());
                    n_V_MultiHit_clusters++;
                } else {
                    h_V_Coord_SingleHitCl1.Fill(v_V_Clusters.at(iVcl).getAvgStrip());
                }
            }


            if (Max_UCluster.getHits()->size() > 0) {
                h_ADC_vs_U_Max1.Fill(Max_UCluster.getAvgStrip(), Max_UCluster.getPeakADC());
            }
            if (Max_VCluster.getHits()->size() > 0) {
                h_ADC_vs_V_Max1.Fill(Max_VCluster.getAvgStrip(), Max_VCluster.getPeakADC());
            }

            if (Max_UCluster.getHits()->size() > 0 && Max_VCluster.getHits()->size() > 0) {
                // This cross represents the cross with Maximum U and Maximum V cluster
                curCrs_MaxADC = uRwellCross(Max_UCluster.getAvgStrip(), Max_VCluster.getAvgStrip());
                h_Cross_YXc_Max1.Fill(curCrs_MaxADC.getX(), curCrs_MaxADC.getY());
                
                if( uRwellTools::IsInsideDetector(curCrs_MaxADC.getX(), curCrs_MaxADC.getY()) ){
                    h_Cross_YXc_Max_InsideDet1.Fill(curCrs_MaxADC.getX(), curCrs_MaxADC.getY());
                }

                if (n_U_cl > n_MaxClusters || n_V_cl > n_MaxClusters) {
                    h_Cross_YXc_Max4.Fill(curCrs_MaxADC.getX(), curCrs_MaxADC.getY());
                }

                int time_Max_U = Max_UCluster.getPeakTime();
                int time_Max_V = Max_VCluster.getPeakTime();
                h_t_V_vs_U_Max1.Fill(time_Max_U, time_Max_V);

                int size_clU_U = Max_UCluster.getHits()->size();
                int size_clU_V = Max_VCluster.getHits()->size();

                double cl_ADC_U = Max_UCluster.getPeakADC();
                double cl_ADC_V = Max_VCluster.getPeakADC();
                h_time_ADC_U_Max1.Fill(cl_ADC_U, time_Max_U);
                h_time_ADC_V_Max1.Fill(cl_ADC_V, time_Max_V);

                double cl_Energy_U = Max_UCluster.getEnergy();
                double cl_Energy_V = Max_VCluster.getEnergy();
                double cl_Energy_Tot = cl_Energy_U + cl_Energy_V;

                h_cl_V_U_Energy_Max1.Fill(cl_Energy_U, cl_Energy_V);

                h_ADC_vs_U_Max2.Fill(Max_UCluster.getAvgStrip(), Max_UCluster.getPeakADC());
                h_ADC_vs_V_Max2.Fill(Max_VCluster.getAvgStrip(), Max_VCluster.getPeakADC());


                if (cl_ADC_U > highADCCut && cl_ADC_V > highADCCut) {
                    h_Cross_YXc_Max5.Fill(curCrs_MaxADC.getX(), curCrs_MaxADC.getY());
                }

                int group_ID = curCrs_MaxADC.getGroupID();

                double crsX = curCrs_MaxADC.getX();
                double crsY = curCrs_MaxADC.getY();

                /* 
                 * Some rough geometric cut that selects right side crosses that
                 * are apparently out of the detector active area. Just want to check
                 * whether those events have something unique e.g.
                 */
                if (crsY < -1400. + 2.2 * crsX) {
                    h_clV_clU_size_NOISE_1.Fill(size_clU_U, size_clU_V);
                    h_clV_clU_time_NOISE_1.Fill(time_Max_U, time_Max_V);

                    h_clV_clU_ADC_NOISE_1.Fill(cl_ADC_U, cl_ADC_V);
                    h_N_Cl_V_U_NOISE_1.Fill(n_U_cl, n_V_cl);

                }

                h_Cross_YXc_Max_Weighted1.Fill(crsX, crsY, cl_ADC_U + cl_ADC_V);
                h_Cross_YXc_Max_Weighted_ADC_U1.Fill(crsX, crsY, cl_ADC_U);
                h_Cross_YXc_Max_Weighted_ADC_V1.Fill(crsX, crsY, cl_ADC_V);
                h_Cross_YXc_Max_Weighted_tU1.Fill(crsX, crsY, time_Max_U);
                h_Cross_YXc_Max_Weighted_tV1.Fill(crsX, crsY, time_Max_V);
                h_Cross_YXc_Max_Weighted_clUSize1.Fill(crsX, crsY, size_clU_U);
                h_Cross_YXc_Max_Weighted_clVSize1.Fill(crsX, crsY, size_clU_V);
                h_Cross_YXc_Max_Weighted_TotEnergy1.Fill(crsX, crsY, cl_Energy_Tot);
                h_Cross_YXc_Max_Weighted_Energy_U1.Fill(crsX, crsY, cl_Energy_U);
                h_Cross_YXc_Max_Weighted_Energy_V1.Fill(crsX, crsY, cl_Energy_V);

                h_clV_clU_size1.Fill(size_clU_U, size_clU_V);


                if (group_ID >= 0) {
                    h_Cross_YXx_Max1_[group_ID].Fill(crsX, crsY);
                    h_ADC_V_U_Max1_[group_ID].Fill(cl_ADC_U, cl_ADC_V);
                    h_nV_U_Max1_[group_ID].Fill(Max_UCluster.getHits()->size(), Max_VCluster.getHits()->size());
                }

            }

            int n_GEMHits_X = 0;
            int n_GEMHits_Y = 0;

            int n_GEM_Y_Cl = v_Y_GEM_Clusters.size();
            int n_GEM_X_Cl = v_X_GEM_Clusters.size();

            h_n_GEM_Y_vs_X_Clusters1.Fill(n_GEM_X_Cl, n_GEM_Y_Cl);

            for (auto cur_GEM_X_Cluster : v_X_GEM_Clusters) {
                for (auto cur_GEM_Y_Cluster : v_Y_GEM_Clusters) {
                    h_GEM_cl_YXC1.Fill(cur_GEM_X_Cluster.getAvgStrip() * GEM_strip2Coord, cur_GEM_Y_Cluster.getAvgStrip() * GEM_strip2Coord);
                }
            }

            for (int iGEMHit = 0; iGEMHit < v_GEMHits.size(); iGEMHit++) {

                if (v_GEMHits.at(iGEMHit).strip < 128) {
                    h_ADC_GEM_X1.Fill(v_GEMHits.at(iGEMHit).adc);
                } else {
                    h_ADC_GEM_Y1.Fill(v_GEMHits.at(iGEMHit).adc);
                }

                if (v_GEMHits.at(iGEMHit).adcRel > GEMHighThreshold) {
                    if (v_GEMHits.at(iGEMHit).strip < 128) {
                        n_GEMHits_X++;
                    } else {
                        n_GEMHits_Y++;
                    }
                }

            }


            double maxGEM_Y_ADC = 0;
            double maxGEM_X_ADC = 0;

            for (auto curXGEMHit : v_X_GEMHits) {

                if (curXGEMHit.adc > maxGEM_Y_ADC) {
                    maxGEM_X_ADC = curXGEMHit.adc;
                }

                if (curXGEMHit.adcRel < GEMHighThreshold) {
                    continue;
                }

                for (auto curYGEMHit : v_Y_GEMHits) {
                    if (curYGEMHit.adcRel < GEMHighThreshold) {
                        continue;
                    }
                    h_GEM_XY1.Fill(curXGEMHit.stripLocal, curYGEMHit.stripLocal);

                }
            }

            for (auto curYGEMHit : v_Y_GEMHits) {
                if (curYGEMHit.adc > maxGEM_Y_ADC) {
                    maxGEM_Y_ADC = curYGEMHit.adc;
                }
            }

            if (maxGEM_X_ADC > 0) {
                h_MaxADC_GEM_X1.Fill(maxGEM_X_ADC);
            }

            if (maxGEM_Y_ADC > 0) {
                h_MaxADC_GEM_Y1.Fill(maxGEM_Y_ADC);
            }

            h_n_GEM_Y_vs_X_Hits1.Fill(n_GEMHits_X, n_GEMHits_Y);

            if (n_GEM_Y_Cl == 1 && n_GEM_X_Cl == 1) {
                h_n_uRwell_V_vs_U_MultiHitCl.Fill(n_U_MultiHit_clusters, n_V_MultiHit_clusters);

                if (Max_UCluster.getHits()->size() > 0) {
                    h_ADC_U_Max2.Fill(Max_UCluster.getPeakADC());
                }
                if (Max_VCluster.getHits()->size() > 0) {
                    h_ADC_V_Max2.Fill(Max_VCluster.getPeakADC());
                }

                for (int iUcl = 0; iUcl < v_U_Clusters.size(); iUcl++) {
                    int nhits = v_U_Clusters.at(iUcl).getHits()->size();
                    if (nhits > 1) {
                        h_U_Coord_MultrCl2.Fill(v_U_Clusters.at(iUcl).getAvgStrip());
                    }
                }
                for (int iVcl = 0; iVcl < v_V_Clusters.size(); iVcl++) {
                    int nhits = v_V_Clusters.at(iVcl).getHits()->size();
                    if (nhits > 1) {
                        h_V_Coord_MultrCl2.Fill(v_V_Clusters.at(iVcl).getAvgStrip());
                    }
                }


            }

            for (auto cur_Vcl : v_V_Clusters) {
                double vAvgStrip = cur_Vcl.getAvgStrip();

                //cout<<cur_Vcl.getHits()->size()<<endl;

                if (cur_Vcl.getHits()->size() < MinClSize) {
                    continue;
                }
                //cout<<"Kuku"<<endl;
                h_nVcl_vs_Ucoord1.Fill(vAvgStrip, n_U_MultiHit_clusters);

            }
            if (n_GEM_Y_Cl >= 1 && n_GEM_X_Cl >= 1) {
                h_Cross_YXc_Max2.Fill(curCrs_MaxADC.getX(), curCrs_MaxADC.getY());

                if (GEM_Max_Y_Cluster.getHits()->size() > 0 && GEM_Max_X_Cluster.getHits()->size() > 0
                        && GEM_Max_Y_Cluster.getAvgStrip() * GEM_strip2Coord > 5) {
                    h_Cross_YXc_Max3.Fill(curCrs_MaxADC.getX(), curCrs_MaxADC.getY());
                }
            }

            for (auto cur_Ucl : v_U_Clusters) {

                double uAvgStrip = cur_Ucl.getAvgStrip();

                if (cur_Ucl.getHits()->size() < MinClSize /*|| uAvgStrip >= slot11_StripMin*/) {
                    continue;
                }

                h_nVcl_vs_Ucoord1.Fill(uAvgStrip, n_V_MultiHit_clusters);
                for (auto cur_Vcl : v_V_Clusters) {
                    if (cur_Vcl.getHits()->size() < MinClSize) {
                        continue;
                    }

                    double cl_Strip_U = cur_Ucl.getAvgStrip();
                    double cl_Strip_V = cur_Vcl.getAvgStrip();

                    uRwellCross curCrs = uRwellCross(cl_Strip_U, cl_Strip_V);
                    double crsX = curCrs.getX();
                    double crsY = curCrs.getY();
                    int group_ID = curCrs.getGroupID();

                    //curCrs.PrintCross();

                    double cl_ADC_U = cur_Ucl.getPeakADC();
                    double cl_ADC_V = cur_Vcl.getPeakADC();

                    if (group_ID >= 0) {
                        h_Cross_YXx1_[group_ID].Fill(crsX, crsY);
                        h_ADC_V_U1_[group_ID].Fill(cl_ADC_U, cl_ADC_V);
                        h_nV_U1_[group_ID].Fill(cur_Ucl.getHits()->size(), cur_Vcl.getHits()->size());
                    }

                    double cl_ADC_Tot = cl_ADC_U + cl_ADC_V;

                    h_Cross_YXc1.Fill(crsX, crsY);
                    h_Cross_YXc_Weighted1.Fill(crsX, crsY, cl_ADC_Tot);


                    if (uRwellTools::IsInsideDetector(crsX, crsY)) {
                        h_Cross_YXc3.Fill(crsX, crsY);
                        h_Cross_YXc_Weighted3.Fill(crsX, crsY, cl_ADC_Tot);
                    }

                    if (n_GEM_Y_Cl == 1 && n_GEM_X_Cl == 1) {
                        h_Cross_YXc2.Fill(crsX, crsY);
                        h_Cross_YXc_Weighted2.Fill(crsX, crsY, cl_ADC_Tot);
                    }
                }

            }


        }
    } catch (const char msg) {
        cerr << msg << endl;
    }

    gDirectory->Write();
    file_out->Close();

    return 0;
}