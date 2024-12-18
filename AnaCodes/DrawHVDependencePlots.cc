/* 
 * File:   DrawHVDependecePlots.cc
 * Author: rafopar
 *
 * Created on December 1, 2023, 8:59 PM
 */

#include <vector>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include <cxxopts.hpp>

#include <TF1.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TFile.h>
#include <TGraph.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TEfficiency.h>
#include <TMultiGraph.h>
#include <TGraphErrors.h>
#include <TGraphAsymmErrors.h>

#include <uRwellTools.h>

using namespace std;

/*
 * 
 */
int main(int argc, char **argv) {

    cxxopts::Options options("DrawHVDependencePlots", "Draws plots of efficiencies (or others staff) as a function of HV");

    options.add_options()
            ("s,Series", "HV Scan series", cxxopts::value<int>())
            ("t,Threshold", "Hit Threshold in terms of sigma", cxxopts::value<double>())
            ("m,MinHits", "Number of minimum hits in the cluster", cxxopts::value<int>())
            ;

    auto parsed_options = options.parse(argc, argv);

    if (!parsed_options.count("Series")) {
        cout << "The Series of runs is nor provided. Exiting..." << endl;
        exit(1);
    }
    const int series = parsed_options["Series"].as<int>();


    if (!parsed_options.count("Threshold")) {
        cout << "* You didn't provide the hit threshold. Exiting" << endl;
        exit(1);
    }
    const double threshold = parsed_options["Threshold"].as<double>();

    if (!parsed_options.count("MinHits")) {
        cout << "* You didn't provide the Minimum hits int the cluster. Exiting " << endl;
        exit(1);
    }
    const int MinHits = parsed_options["MinHits"].as<int>();

    cout << "The hit threshold is " << threshold << "\\sigma" << endl;
    cout << "The Minimum cluster size is " << MinHits << "hits" << endl;


    std::map<int, std::vector<int> > mv_runs;
    mv_runs[1] = {1731, 1745, 1750, 1753, 1790, 1761, 1779};
    mv_runs[2] = {1824, 1826, 1828, 1833, 1835, 1837, 1840, 1848, 1852};
    mv_runs[3] = {1854, 1855, 1857, 1852, 1859, 1861, 1866};
    mv_runs[4] = {18660, 18661, 18662, 18663, 18664, 18665, 18666, 18667, 18668, 18669, 186610, 186611, 186612, 186613, 186614, 186615, 186616, 186617};
    mv_runs[5] = {18720, 18721, 18722, 18723, 18724, 18725, 18726, 18727, 18728, 18729, 187210, 187211, 187212, 187213, 187214, 187215, 187216, 187217, 187218, 187219};
    mv_runs[6] = {18770, 18771, 18772, 18773, 18774, 18775, 18776, 18777, 18778, 18779, 187710, 187711, 187712, 187713, 187714, 187715, 187716};
    mv_runs[7] = {18800, 18801, 18802};
    mv_runs[8] = {18830, 18831, 18832, 18833, 18834, 18835, 18836, 18837, 18838, 18839, 188310, 188311, 188312, 188313, 188314, 188315, 188316, 188317, 188318, 188319,
        188320, 188321, 188322, 188323, 188324, 188325, 188326, 188327, 188328, 188329,
        188330, 188331, 188332, 188333};
    mv_runs[9] = {18590, 18591, 18592, 18593, 18594, 18595, 18596, 18597, 18598, 18599, 185910, 185911, 185912, 185913};
    mv_runs[10] = {18880, 18881, 18882, 18883, 18884, 18885, 18886, 18887, 18888, 18889, 188810, 188811, 188812, 188813, 188814, 188815, 188816, 188817, 188818, 188819,
        188820, 188821, 188822, 188823, 188824, 188825, 188826, 188827, 188828, 188829, 188830, 188831, 188832, 188833, 188834, 188835, 188836, 188837, 188838, 188839,
        188840, 188841, 188842, 188843, 188844, 188845, 188846, 188847, 188848, 188849, 188850, 188851, 188852, 188853, 188854, 188855, 188856, 188857, 188858, 188859,
        188860, 188861, 188862, 188863, 188864, 188865, 188866, 188867, 188868, 188869, 188870, 188871, 188872, 188873, 188874, 188875, 188876, 188877, 188878, 188879,
        188880, 188881, 188882, 188883, 188884, 188885, 188886, 188887, 188888, 188889, 188890, 188891, 188892, 188893, 188894, 188895, 188896, 188897, 188898, 188899,
        1888100, 1888101, 1888102, 1888103, 1888104};
    mv_runs[11] = {19210, 19211, 19212, 19213, 19214, 19215, 19216};
    mv_runs[12] = {19340, 19341, 19342, 19343, 19344, 19345, 19346, 19347, 19348, 19349, 193410, 193411, 193412, 193413, 193414, 193415};
    mv_runs[13] = {19360, 19361, 19362, 19363, 19364, 19365, 19366};
    mv_runs[14] = {19460, 19461, 19462, 19463, 19464, 19465, 19466, 19467, 19468, 19469,         194611, 194612, 194613, 194614, 194615, 194616, 194617, 194618, 194619,
    194621, 194622, 194623, 194624, 194625, 194626, 194627, 194628, 194629, 194630, 194631, 194632};
    mv_runs[15] = {19730, 19731, 19732, 19733, 19734, 19735, 19736, 19737, 19738, 19739, 197310, 197311, 197312, 197313, 197314, 197315, 197316, 197317, 197318};
    mv_runs[16] = {19860, 19861, 19862, 19863, 19864, 19865, 19866, 19867, 19868, 19869, 198610, 198611, 198612, 198613, 198614, 198615, 198616, 198617, 198618, 198619,
                  198620, 198621, 198622, 198623, 198624, 198625, 198626};
    mv_runs[18] = {2091, 2090, 2089, 2085, 2087};
    mv_runs[19] = {2092, 2095, 2096, 2099};
    mv_runs[20] = {2100, 2101, 2103, 2104, 2105, 2106, 2108, 2110, 2111, 2112, 2119};
    mv_runs[21] = {2119, 2130, 2139, 2145};
    mv_runs[22] = {/*2191,*/ 2193, 2199, 2201, 2203, 2205, 2207, 2219, 2232, 2234, 2236};
    std::map<int, double> m_MESH_HV; // The key is the run number, the value is the MESH_HV
    std::map<int, double> m_Cathode_HV; // The key is the run number, the value is the Cathode_HV
    std::map<int, double> m_Drift_HV; // The key is the run number, the value is the Drift_HV = Hathode_HV - MESH_HV

    std::map<int, std::string> m_HVType;
    m_HVType[1] = "MESH";
    m_HVType[2] = "MESH";
    m_HVType[3] = "DRIFT";
    m_HVType[4] = "FILE_IND";
    m_HVType[5] = "FILE_IND";
    m_HVType[6] = "FILE_IND";
    m_HVType[7] = "FILE_IND";
    m_HVType[8] = "FILE_IND";
    m_HVType[9] = "FILE_IND";
    m_HVType[10] = "FILE_IND";
    m_HVType[11] = "FILE_IND";
    m_HVType[12] = "FILE_IND";
    m_HVType[13] = "FILE_IND";
    m_HVType[14] = "FILE_IND";
    m_HVType[15] = "FILE_IND";
    m_HVType[16] = "FILE_IND";
    m_HVType[18] = "MESH";
    m_HVType[19] = "DRIFT";
    m_HVType[20] = "DRIFT";
    m_HVType[21] = "MESH";
    m_HVType[22] = "MESH";

    std::map<int, std::string> m_XTitle;

    m_XTitle[1] = "MESH HV [V]";
    m_XTitle[2] = "MESH HV [V]";
    m_XTitle[3] = "DRIFT HV [V]";
    m_XTitle[4] = "File number";
    m_XTitle[5] = "File number";
    m_XTitle[6] = "File number";
    m_XTitle[7] = "File number";
    m_XTitle[8] = "File number";
    m_XTitle[9] = "File number";
    m_XTitle[10] = "File number";
    m_XTitle[11] = "File number";
    m_XTitle[13] = "File number";
    m_XTitle[14] = "File number";
    m_XTitle[16] = "File number";
    m_XTitle[18] = "MESH HV [V]";
    m_XTitle[19] = "DRIFT HV [V]";
    m_XTitle[20] = "DRIFT HV [V]";
    m_XTitle[21] = "MESH HV [V]";
    m_XTitle[22] = "MESH HV [V]";
    
    TCanvas *c1 = new TCanvas("c1", "", 950, 950);
    c1->SetTopMargin(0.02);
    c1->SetRightMargin(0.02);
    c1->SetGridy();
    c1->SetGridx();

    TGraphAsymmErrors *gr_Eff_U = new TGraphAsymmErrors();
    gr_Eff_U->SetMarkerStyle(20);
    gr_Eff_U->SetMarkerColor(2);
    gr_Eff_U->SetMarkerSize(2);
    TGraphAsymmErrors *gr_Eff_V = new TGraphAsymmErrors();
    gr_Eff_V->SetMarkerStyle(21);
    gr_Eff_V->SetMarkerColor(4);
    gr_Eff_V->SetMarkerSize(2);
    TGraphAsymmErrors *gr_Eff_OR = new TGraphAsymmErrors();
    gr_Eff_OR->SetMarkerStyle(22);
    gr_Eff_OR->SetMarkerColor(6);
    gr_Eff_OR->SetMarkerSize(2);
    TGraphAsymmErrors *gr_Eff_AND = new TGraphAsymmErrors();
    gr_Eff_AND->SetMarkerStyle(23);
    gr_Eff_AND->SetMarkerColor(7);
    gr_Eff_AND->SetMarkerSize(2);
    TGraphAsymmErrors *gr_Eff_CrsBgrSubtr = new TGraphAsymmErrors();
    gr_Eff_CrsBgrSubtr->SetMarkerStyle(24);
    gr_Eff_CrsBgrSubtr->SetMarkerColor(8);
    gr_Eff_CrsBgrSubtr->SetMarkerSize(2);

    TGraphAsymmErrors *gr_Eff_Ratio_V_Over_U = new TGraphAsymmErrors();
    gr_Eff_Ratio_V_Over_U->SetMarkerStyle(20);
    gr_Eff_Ratio_V_Over_U->SetMarkerColor(2);
    gr_Eff_Ratio_V_Over_U->SetMarkerSize(2);

    TGraphErrors *gr_ADC_MPV_U = new TGraphErrors();
    gr_ADC_MPV_U->SetMarkerStyle(20);
    gr_ADC_MPV_U->SetMarkerColor(2);
    gr_ADC_MPV_U->SetMarkerSize(2);

    TGraphErrors *gr_ADC_MPV_V = new TGraphErrors();
    gr_ADC_MPV_V->SetMarkerStyle(21);
    gr_ADC_MPV_V->SetMarkerColor(4);
    gr_ADC_MPV_V->SetMarkerSize(2);

    TGraphErrors *gr_ADC_GEM_X = new TGraphErrors();
    gr_ADC_GEM_X->SetMarkerStyle(22);
    gr_ADC_GEM_X->SetMarkerColor(6);
    gr_ADC_GEM_X->SetMarkerSize(2);

    TGraphErrors *gr_ADC_GEM_Y = new TGraphErrors();
    gr_ADC_GEM_Y->SetMarkerStyle(23);
    gr_ADC_GEM_Y->SetMarkerColor(7);
    gr_ADC_GEM_Y->SetMarkerSize(2);

    TGraphErrors *gr_tU_RMS = new TGraphErrors();
    gr_tU_RMS->SetMarkerStyle(20);
    gr_tU_RMS->SetMarkerColor(2);
    gr_tU_RMS->SetMarkerSize(2);
    TGraphErrors *gr_tV_RMS = new TGraphErrors();
    gr_tV_RMS->SetMarkerStyle(21);
    gr_tV_RMS->SetMarkerColor(4);
    gr_tV_RMS->SetMarkerSize(2);
    
    TGraphErrors *gr_clU_Size = new TGraphErrors();
    gr_clU_Size->SetMarkerStyle(20);
    gr_clU_Size->SetMarkerColor(2);
    gr_clU_Size->SetMarkerSize(2);
    TGraphErrors *gr_clV_Size = new TGraphErrors();
    gr_clV_Size->SetMarkerStyle(21);
    gr_clV_Size->SetMarkerColor(4);
    gr_clV_Size->SetMarkerSize(2);

    std::string hvTablefileName = Form("HV_Table_%d.dat", series);
    ifstream inp_HVTable(hvTablefileName.c_str());

    if (inp_HVTable.is_open()) {

        while (!inp_HVTable.eof()) {
            int run, HV_MESH, HV_Cathode, HV_GEM, HV_Drift;

            inp_HVTable >> run >> HV_MESH >> HV_Cathode>>HV_GEM;

            cout << run << "  " << HV_MESH << "   " << HV_Cathode << "   " << HV_GEM << endl;
            m_MESH_HV[run] = HV_MESH;
            m_Cathode_HV[run] = HV_Cathode;
            m_Drift_HV[run] = HV_Cathode - HV_MESH;
        }

    } else {
        cout << "Can not open the file" << hvTablefileName.c_str() << endl;
    }

    for (int i = 0; i < mv_runs[series].size(); i++) {

        int run = mv_runs[series].at(i);

        double eff_U, eff_V, eff_OR, eff_AND, eff_BgrSubtr;
        uRwellTools::uRwellEff eff;

        TFile *file_in = new TFile(Form("AnaClustering_%d_Thr_%1.1f_MinHits_%d.root", run, threshold, MinHits));

        TH2D *h_n_uRwell_V_vs_U_MultiHitCl = (TH2D*) file_in->Get("h_n_uRwell_V_vs_U_MultiHitCl");
        double All_GoodEventts = h_n_uRwell_V_vs_U_MultiHitCl->Integral( 0, h_n_uRwell_V_vs_U_MultiHitCl->GetNbinsX() + 1, 0, h_n_uRwell_V_vs_U_MultiHitCl->GetNbinsY() + 1 );

        uRwellTools::CalcEfficiencies(h_n_uRwell_V_vs_U_MultiHitCl, eff);

        TH2D *h_Cross_YXc2 = (TH2D*) file_in->Get("h_Cross_YXc2");

        double n_CrossBgrSubtracted = uRwellTools::getNofBgrSbtrCrosses(h_Cross_YXc2);

        eff.eff_crs_BgrSubtr = 100. * n_CrossBgrSubtracted / All_GoodEventts;

        double HV_Value;

        if (strcmp(m_HVType[series].c_str(), "MESH") == 0) {
            HV_Value = m_MESH_HV[run];
        } else if (strcmp(m_HVType[series].c_str(), "DRIFT") == 0) {
            HV_Value = m_Drift_HV[run];
        } else if (strcmp(m_HVType[series].c_str(), "CATHODE") == 0) {
            HV_Value = m_Cathode_HV[run];
        } else if (strcmp(m_HVType[series].c_str(), "FILE_IND") == 0) {
            HV_Value = m_Cathode_HV[run];
        }

        gr_Eff_U->SetPoint(i, HV_Value, eff.eff_U);
        gr_Eff_U->SetPointError(i, 0., 0., eff.errLow_eff_U, eff.errUp_eff_U);
        gr_Eff_V->SetPoint(i, HV_Value, eff.eff_V);
        gr_Eff_U->SetPointError(i, 0., 0., eff.errLow_eff_V, eff.errUp_eff_V);
        gr_Eff_OR->SetPoint(i, HV_Value, eff.eff_OR);
        gr_Eff_U->SetPointError(i, 0., 0., eff.errLow_eff_OR, eff.errUp_eff_OR);
        gr_Eff_AND->SetPoint(i, HV_Value, eff.eff_AND);
        gr_Eff_U->SetPointError(i, 0., 0., eff.errLow_eff_AND, eff.errUp_eff_AND);


        double err_eff_U_over_V_Low = (eff.eff_V / eff.eff_U) * sqrt((eff.errLow_eff_U / eff.eff_U)*(eff.errLow_eff_U / eff.eff_U) + (eff.errUp_eff_V / eff.eff_V)*(eff.errUp_eff_V / eff.eff_V));
        double err_eff_U_over_V_Up = (eff.eff_V / eff.eff_U) * sqrt((eff.errUp_eff_U / eff.eff_U)*(eff.errUp_eff_U / eff.eff_U) + (eff.errLow_eff_V / eff.eff_V)*(eff.errLow_eff_V / eff.eff_V));
        gr_Eff_Ratio_V_Over_U->SetPoint(i, HV_Value, eff.eff_V / eff.eff_U);
        gr_Eff_Ratio_V_Over_U->SetPointError(i, 0., 0., err_eff_U_over_V_Low, err_eff_U_over_V_Up);


        eff.errUp_eff_crs_BgrSubtr = 100 * TEfficiency::ClopperPearson(All_GoodEventts, n_CrossBgrSubtracted, uRwellTools::OneSigma, true) - eff.eff_crs_BgrSubtr;
        eff.errLow_eff_crs_BgrSubtr = eff.eff_crs_BgrSubtr - 100 * TEfficiency::ClopperPearson(All_GoodEventts, n_CrossBgrSubtracted, uRwellTools::OneSigma, false);
        gr_Eff_CrsBgrSubtr->SetPoint(i, HV_Value, eff.eff_crs_BgrSubtr);
        gr_Eff_CrsBgrSubtr->SetPointError(i, 0., 0., eff.errLow_eff_crs_BgrSubtr, eff.errUp_eff_crs_BgrSubtr);

        //cout<<eff.errLow_eff_U<<"  "<<eff.errUp_eff_U<<endl;

        TH1D *h_U_PeakADC_MultiCl1 = (TH1D*) file_in->Get("h_U_PeakADC_MultiCl1");
        h_U_PeakADC_MultiCl1->SetAxisRange(0., 500., "X");
        TH1D *h_V_PeakADC_MultiCl1 = (TH1D*) file_in->Get("h_V_PeakADC_MultiCl1");
        h_V_PeakADC_MultiCl1->SetAxisRange(0., 500., "X");
        TH1D *h_MaxADC_GEM_X1 = (TH1D*) file_in->Get("h_MaxADC_GEM_X1");
        TH1D *h_MaxADC_GEM_Y1 = (TH1D*) file_in->Get("h_MaxADC_GEM_Y1");
        
        TH2D *h_time_ADC_U_Max1 = (TH2D*)file_in->Get("h_time_ADC_U_Max1");
        TH1D *h_ADC_U_Max1 = (TH1D*)h_time_ADC_U_Max1->ProjectionX("h_ADC_U_Max1", 1, h_time_ADC_U_Max1->GetNbinsY() );
        h_ADC_U_Max1->SetAxisRange(0., 500., "X");

        TH2D *h_time_ADC_V_Max1 = (TH2D*)file_in->Get("h_time_ADC_V_Max1");
        TH1D *h_ADC_V_Max1 = (TH1D*)h_time_ADC_V_Max1->ProjectionX("h_ADC_V_Max1", 1, h_time_ADC_V_Max1->GetNbinsY() );
        h_ADC_V_Max1->SetAxisRange(0., 500., "X");
        
        TH1D *h_ADC_U_Max2 = (TH1D*)file_in->Get("h_ADC_U_Max2");
        h_ADC_U_Max2->SetAxisRange(0., 500., "X");
        TH1D *h_ADC_V_Max2 = (TH1D*)file_in->Get("h_ADC_V_Max2");
        h_ADC_V_Max2->SetAxisRange(0., 500., "X");
        
        uRwellTools::ADC_Distribution distr_U = uRwellTools::CalcMPVandMean(h_ADC_U_Max2);
        uRwellTools::ADC_Distribution distr_V = uRwellTools::CalcMPVandMean(h_ADC_V_Max2);
//        uRwellTools::ADC_Distribution distr_U = uRwellTools::CalcMPVandMean(h_U_PeakADC_MultiCl1);
//        uRwellTools::ADC_Distribution distr_V = uRwellTools::CalcMPVandMean(h_V_PeakADC_MultiCl1);
        uRwellTools::ADC_Distribution distr_GEM_X = uRwellTools::CalcMPVandMean(h_MaxADC_GEM_X1);
        uRwellTools::ADC_Distribution distr_GEM_Y = uRwellTools::CalcMPVandMean(h_MaxADC_GEM_Y1);

        TH2D *h_t_V_vs_U_Max1 = (TH2D*)file_in->Get("h_t_V_vs_U_Max1");
        double tU_RMS = h_t_V_vs_U_Max1->GetRMS(1);
        double tU_RMS_Err = h_t_V_vs_U_Max1->GetRMSError(1);
        double tV_RMS = h_t_V_vs_U_Max1->GetRMS(2);
        double tV_RMS_Err = h_t_V_vs_U_Max1->GetRMSError(2);
        
        TH2D *h_clV_clU_size1 = (TH2D*)file_in->Get("h_clV_clU_size1");
        double cl_U_Size = h_clV_clU_size1->GetMean(1);
        double cl_U_SizeErr = h_clV_clU_size1->GetMeanError(1);
        double cl_V_Size = h_clV_clU_size1->GetMean(2);
        double cl_V_SizeErr = h_clV_clU_size1->GetMeanError(2);
        
        gr_ADC_MPV_U->SetPoint(i, HV_Value, distr_U.MPV);
        gr_ADC_MPV_U->SetPointError(i, 0, distr_U.errMPV);
        gr_ADC_MPV_V->SetPoint(i, HV_Value, distr_V.MPV);
        gr_ADC_MPV_V->SetPointError(i, 0, distr_V.errMPV);
        gr_ADC_GEM_X->SetPoint(i, HV_Value, distr_GEM_X.MPV);
        gr_ADC_GEM_X->SetPointError(i, 0, distr_GEM_X.errMPV);
        gr_ADC_GEM_Y->SetPoint(i, HV_Value, distr_GEM_Y.MPV);
        gr_ADC_GEM_Y->SetPointError(i, 0, distr_GEM_Y.errMPV);

        gr_tU_RMS->SetPoint(i, HV_Value, tU_RMS);
        gr_tU_RMS->SetPointError(i, 0, tU_RMS_Err);
        gr_tV_RMS->SetPoint(i, HV_Value, tV_RMS);
        gr_tV_RMS->SetPointError(i, 0, tV_RMS_Err);

        gr_clU_Size->SetPoint(i, HV_Value, cl_U_Size);
        gr_clU_Size->SetPointError(i, 0, cl_U_SizeErr);
        gr_clV_Size->SetPoint(i, HV_Value, cl_V_Size);
        gr_clV_Size->SetPointError(i, 0, cl_V_SizeErr);
        
        delete h_Cross_YXc2;
        delete h_n_uRwell_V_vs_U_MultiHitCl;
        delete h_U_PeakADC_MultiCl1;
        delete h_V_PeakADC_MultiCl1;
        delete h_MaxADC_GEM_X1;
        delete h_MaxADC_GEM_Y1;
        delete h_t_V_vs_U_Max1;
        delete h_clV_clU_size1;
        delete file_in;
    }

    //TLegend *leg1 = new TLegend(0.12, 0.85, 0.3, 0.97);
    TLegend *leg1 = new TLegend(0.55, 0.25, 0.93, 0.55);
    leg1->SetBorderSize(0);
    leg1->AddEntry(gr_Eff_U, "U Cluster");
    leg1->AddEntry(gr_Eff_V, "V Cluster");
    leg1->AddEntry(gr_Eff_OR, "Any Cluster");
    leg1->AddEntry(gr_Eff_AND, "U and V Cluster");
    //leg1->AddEntry(gr_Eff_CrsBgrSubtr, "Bgr Subtracted");

    TMultiGraph *mtgr1 = new TMultiGraph();
    mtgr1->Add(gr_Eff_U);
    mtgr1->Add(gr_Eff_V);
    mtgr1->Add(gr_Eff_OR);
    mtgr1->Add(gr_Eff_AND);
    //mtgr1->Add(gr_Eff_CrsBgrSubtr);

    mtgr1->Draw("APL");
    mtgr1->SetTitle(Form("; %s; Efficiency", m_XTitle[series].c_str()));
    mtgr1->SetMaximum(100.);
    mtgr1->SetMinimum(0.);
    leg1->Draw();
    c1->Print(Form("Figs/HV_Eff_Dependence_Thr_%1.1f_MinHits_%d_Series_%d.pdf", threshold, MinHits, series));
    c1->Print(Form("Figs/HV_Eff_Dependence_Thr_%1.1f_MinHits_%d_Series_%d.png", threshold, MinHits, series));
    c1->Print(Form("Figs/HV_Eff_Dependence_Thr_%1.1f_MinHits_%d_Series_%d.root", threshold, MinHits, series));

    c1->Clear();
    gr_Eff_Ratio_V_Over_U->GetYaxis()->SetTitleOffset(1.4);
    gr_Eff_Ratio_V_Over_U->Draw("APL");
    gr_Eff_Ratio_V_Over_U->SetTitle(Form("; %s; Eff_V/Eff_U", m_XTitle[series].c_str()));

    c1->Print(Form("Figs/HV_EffRatio_U_Over_V_Dep_Thr_%1.1f_MinHits_%d_Series_%d.pdf", threshold, MinHits, series));
    c1->Print(Form("Figs/HV_EffRatio_U_Over_V_Dep_Thr_%1.1f_MinHits_%d_Series_%d.png", threshold, MinHits, series));
    c1->Print(Form("Figs/HV_EffRatio_U_Over_V_Dep_Thr_%1.1f_MinHits_%d_Series_%d.root", threshold, MinHits, series));

    TLegend *leg2 = new TLegend(0.12, 0.85, 0.35, 0.97);
    leg2->SetBorderSize(0);
    leg2->AddEntry(gr_ADC_MPV_U, "U Strips");
    leg2->AddEntry(gr_ADC_MPV_V, "V Strips");
//    leg2->AddEntry(gr_ADC_GEM_X, "GEM X");
//    leg2->AddEntry(gr_ADC_GEM_Y, "GEM Y");

    TMultiGraph *mtgr_ADC_MPV = new TMultiGraph();
    mtgr_ADC_MPV->Add(gr_ADC_MPV_U);
    mtgr_ADC_MPV->Add(gr_ADC_MPV_V);
//    mtgr_ADC_MPV->Add(gr_ADC_GEM_X);
//    mtgr_ADC_MPV->Add(gr_ADC_GEM_Y);
    mtgr_ADC_MPV->Draw("APL");
    mtgr_ADC_MPV->SetTitle(Form("; %s; MPV [ADC]", m_XTitle[series].c_str()));
    //mtgr_ADC_MPV->SetMaximum(100);
    mtgr_ADC_MPV->SetMinimum(30);
    leg2->Draw();
    c1->Print(Form("Figs/HV_MPV_Dependence_Thr_%1.1f_MinHits_%d_Series_%d.pdf", threshold, MinHits, series));
    c1->Print(Form("Figs/HV_MPV_Dependence_Thr_%1.1f_MinHits_%d_Series_%d.png", threshold, MinHits, series));
    c1->Print(Form("Figs/HV_MPV_Dependence_Thr_%1.1f_MinHits_%d_Series_%d.root", threshold, MinHits, series));

    TLegend *leg3 = new TLegend(0.55, 0.85, 0.92, 0.97);
    leg3->SetBorderSize(0);
    leg3->AddEntry(gr_tU_RMS, "U strip time");
    leg3->AddEntry(gr_tV_RMS, "V strip time");
    TMultiGraph *mtgr_timeRMS = new TMultiGraph();
    mtgr_timeRMS->Add(gr_tU_RMS);
    mtgr_timeRMS->Add(gr_tV_RMS);
    mtgr_timeRMS->Draw("AP");
    mtgr_timeRMS->SetTitle("; MESH HV [V]; peak time sample RMS");
    leg3->Draw();
    c1->Print(Form("Figs/HV_peakTime_RMS_Thr_%1.1f_MinHits_%d_Series_%d.pdf", threshold, MinHits, series));
    c1->Print(Form("Figs/HV_peakTime_RMS_Thr_%1.1f_MinHits_%d_Series_%d.png", threshold, MinHits, series));
    c1->Print(Form("Figs/HV_peakTime_RMS_Thr_%1.1f_MinHits_%d_Series_%d.root", threshold, MinHits, series));
  
    TLegend *leg4 = new TLegend(0.15, 0.85, 0.42, 0.97);
    leg4->SetBorderSize(0);
    leg4->AddEntry(gr_clU_Size, "U cluster size");
    leg4->AddEntry(gr_clU_Size, "V cluster size");
    TMultiGraph *mtgr_clSize = new TMultiGraph();
    mtgr_clSize->Add(gr_clU_Size);
    mtgr_clSize->Add(gr_clV_Size);
    mtgr_clSize->Draw("AP");
    mtgr_clSize->SetTitle("; MESH HV [V]; Cluster Size");
    leg3->Draw();
    c1->Print(Form("Figs/HV_clSize_Thr_%1.1f_MinHits_%d_Series_%d.pdf", threshold, MinHits, series));
    c1->Print(Form("Figs/HV_clSize_Thr_%1.1f_MinHits_%d_Series_%d.png", threshold, MinHits, series));
    c1->Print(Form("Figs/HV_clSize_Thr_%1.1f_MinHits_%d_Series_%d.root", threshold, MinHits, series));
    
    return 0;
}