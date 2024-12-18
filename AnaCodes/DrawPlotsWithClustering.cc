/* 
 * File:   DrawPlotsWithClustering.cc
 * Author: rafopar
 *
 * Created on June 7, 2023, 5:14 PM
 */

#include <cstdlib>
#include <cxxopts.hpp>

#include <TF1.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TFile.h>
#include <TLine.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TCanvas.h>
#include <TStyle.h>

#include <uRwellTools.h>
using namespace std;


void DrawActiveArea();

/*
 * 
 */
int main(int argc, char **argv) {

    cxxopts::Options options("DrawPlotsWithClustering", "Draws plots from the root file created by analysis");

    options.add_options()
            ("r,Run", "Run number", cxxopts::value<int>())
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


    const double threshold = parsed_options["Threshold"].as<double>();
    if (!parsed_options.count("Threshold")) {
        cout << "* You didn't provide the hit threshold. Exitint" << endl;
        exit(1);
    }

    const int MinClSize = parsed_options["MinHits"].as<int>();
    if (!parsed_options.count("MinHits")) {
        cout << "* You didn't provide the Minimum hits int the cluster. Exiting " << endl;
        exit(1);
    }

    cout << "The hit threshold is " << threshold << "\\sigma" << endl;
    cout << "The Minimum cluster size is " << MinClSize << "hits" << endl;


    gStyle->SetOptStat(0);
    
    TCanvas *c1 = new TCanvas("c1", "", 950, 950);
    c1->SetTopMargin(0.04);
    c1->SetRightMargin(0.04);
    TLatex *lat1 = new TLatex();
    lat1->SetNDC();
    lat1->SetTextFont(42);

    TLine *line1 = new TLine();
    line1->SetLineColor(2);
    line1->SetLineWidth(2);
    TFile *file_in = new TFile(Form("AnaClustering_%d_Thr_%1.1f_MinHits_%d.root", run, threshold, MinClSize), "Read");

    TH1D *h_U_HitStrip1 = (TH1D*) file_in->Get("h_U_HitStrip1");
    h_U_HitStrip1->SetTitle("; U strip number");
    TH1D *h_U_HitStrip2 = (TH1D*) file_in->Get("h_U_HitStrip2");
    h_U_HitStrip2->SetTitle("; U strip number");
    h_U_HitStrip2->SetLineColor(2);
    h_U_HitStrip1->Draw();
    h_U_HitStrip2->Draw("Same");
    lat1->SetTextColor(4);
    lat1->DrawLatex(0.6, 0.8, "3#sigma cut");
    lat1->SetTextColor(2);
    lat1->DrawLatex(0.6, 0.7, "5#sigma cut");
    c1->Print(Form("Figs/U_Strip_Distr1_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c1->Print(Form("Figs/U_Strip_Distr1_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c1->Print(Form("Figs/U_Strip_Distr1_%d_t%1.1f_m%d.root", run, threshold, MinClSize));

    TH1D *h_V_HitStrip1 = (TH1D*) file_in->Get("h_V_HitStrip1");
    h_V_HitStrip1->SetTitle("; V strip number");
    TH1D *h_V_HitStrip2 = (TH1D*) file_in->Get("h_V_HitStrip2");
    h_V_HitStrip2->SetTitle("; V strip number");
    h_V_HitStrip2->SetLineColor(2);
    h_V_HitStrip1->Draw();
    h_V_HitStrip2->Draw("Same");
    lat1->SetTextColor(4);
    lat1->DrawLatex(0.6, 0.8, "3#sigma cut");
    lat1->SetTextColor(2);
    lat1->DrawLatex(0.6, 0.7, "5#sigma cut");
    c1->Print(Form("Figs/V_Strip_Distr1_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c1->Print(Form("Figs/V_Strip_Distr1_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c1->Print(Form("Figs/V_Strip_Distr1_%d_t%1.1f_m%d.root", run, threshold, MinClSize));


    TH1D *h_n_UclHits1 = (TH1D*) file_in->Get("h_n_UclHits1");
    h_n_UclHits1->SetLineWidth(2);
    h_n_UclHits1->SetTitle("; The # of Hits in U clusters");
    int bin_2 = h_n_UclHits1->FindBin(2);
    //h_n_UclHits1->SetMaximum(1.05 * h_n_UclHits1->GetBinContent(bin_2));
    //h_n_UclHits1->SetAxisRange(2, h_n_UclHits1->GetBinCenter(h_n_UclHits1->GetNbinsX()));
    h_n_UclHits1->GetXaxis()->SetNdivisions(716);
    h_n_UclHits1->Draw();
    c1->Print(Form("Figs/N_Ucl_Hits_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c1->Print(Form("Figs/N_Ucl_Hits_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c1->Print(Form("Figs/N_Ucl_Hits_%d_t%1.1f_m%d.root", run, threshold, MinClSize));

    TH1D *h_n_VclHits1 = (TH1D*) file_in->Get("h_n_VclHits1");
    h_n_VclHits1->SetLineWidth(2);
    h_n_VclHits1->SetTitle("; The # of Hits in V clusters");
    bin_2 = h_n_VclHits1->FindBin(2);
    //h_n_VclHits1->SetMaximum(1.05 * h_n_VclHits1->GetBinContent(bin_2));
    //h_n_VclHits1->SetAxisRange(2, h_n_VclHits1->GetBinCenter(h_n_VclHits1->GetNbinsX()));
    h_n_VclHits1->GetXaxis()->SetNdivisions(716);
    h_n_VclHits1->Draw();
    c1->Print(Form("Figs/N_Vcl_Hits_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c1->Print(Form("Figs/N_Vcl_Hits_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c1->Print(Form("Figs/N_Vcl_Hits_%d_t%1.1f_m%d.root", run, threshold, MinClSize));

    TF1 *f_Landau = new TF1("f_Landau", "[0]*TMath::Landau(x, [1], [2])", 0., 1000.);
    f_Landau->SetNpx(4500);

    double adcMaxAxis = 500.;
    TH1D *h_U_PeakADC_MultiCl1 = (TH1D*) file_in->Get("h_U_PeakADC_MultiCl1");
    h_U_PeakADC_MultiCl1->SetLineWidth(2);
    h_U_PeakADC_MultiCl1->SetTitle("; ADC of U cluster peak");
    h_U_PeakADC_MultiCl1->SetAxisRange(0., adcMaxAxis, "X");
    f_Landau->SetParameters(5. * h_U_PeakADC_MultiCl1->GetMaximum(), h_U_PeakADC_MultiCl1->GetBinCenter(h_U_PeakADC_MultiCl1->GetMaximumBin()), 10);
    h_U_PeakADC_MultiCl1->Fit(f_Landau, "MeV", "", 0., adcMaxAxis);
    lat1->DrawLatex(0.5, 0.7, Form("MPV = %1.1f", f_Landau->GetParameter(1)));
    c1->Print(Form("Figs/Peak_ADC_UStrips_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c1->Print(Form("Figs/Peak_ADC_UStrips_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c1->Print(Form("Figs/Peak_ADC_UStrips_%d_t%1.1f_m%d.root", run, threshold, MinClSize));

    TH1D *h_V_PeakADC_MultiCl1 = (TH1D*) file_in->Get("h_V_PeakADC_MultiCl1");
    h_V_PeakADC_MultiCl1->SetLineWidth(2);
    h_V_PeakADC_MultiCl1->SetTitle("; ADC of V cluster peak");
    h_V_PeakADC_MultiCl1->SetAxisRange(0., adcMaxAxis, "X");
    f_Landau->SetParameters(5. * h_V_PeakADC_MultiCl1->GetMaximum(), h_V_PeakADC_MultiCl1->GetBinCenter(h_V_PeakADC_MultiCl1->GetMaximumBin()), 2);
    h_V_PeakADC_MultiCl1->Fit(f_Landau, "MeV", "", 0., adcMaxAxis);
    lat1->DrawLatex(0.5, 0.7, Form("MPV = %1.1f", f_Landau->GetParameter(1)));
    c1->Print(Form("Figs/Peak_ADC_VStrips_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c1->Print(Form("Figs/Peak_ADC_VStrips_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c1->Print(Form("Figs/Peak_ADC_VStrips_%d_t%1.1f_m%d.root", run, threshold, MinClSize));

    TH1D *h_MaxADC_GEM_Y1 = (TH1D*) file_in->Get("h_MaxADC_GEM_Y1");
    h_MaxADC_GEM_Y1->SetLineWidth(2);
    h_MaxADC_GEM_Y1->SetTitle("; ADC of GEM Y highest hit [ADC]");
    h_MaxADC_GEM_Y1->SetAxisRange(0., adcMaxAxis, "X");
    f_Landau->SetParameters(5. * h_MaxADC_GEM_Y1->GetMaximum(), h_MaxADC_GEM_Y1->GetBinCenter(h_MaxADC_GEM_Y1->GetMaximumBin()), 10);
    h_MaxADC_GEM_Y1->Fit(f_Landau, "MeV", "", 0., adcMaxAxis);
    lat1->DrawLatex(0.5, 0.7, Form("MPV = %1.1f", f_Landau->GetParameter(1)));
    c1->Print(Form("Figs/GEM_MAX_ADC_Y_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c1->Print(Form("Figs/GEM_MAX_ADC_Y_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c1->Print(Form("Figs/GEM_MAX_ADC_Y_%d_t%1.1f_m%d.root", run, threshold, MinClSize));

    TH1D *h_MaxADC_GEM_X1 = (TH1D*) file_in->Get("h_MaxADC_GEM_X1");
    h_MaxADC_GEM_X1->SetLineWidth(2);
    h_MaxADC_GEM_X1->SetTitle("; ADC of GEM X highest hit [ADC]");
    h_MaxADC_GEM_X1->SetAxisRange(0., adcMaxAxis, "X");
    f_Landau->SetParameters(5. * h_MaxADC_GEM_X1->GetMaximum(), h_MaxADC_GEM_X1->GetBinCenter(h_MaxADC_GEM_X1->GetMaximumBin()), 10);
    h_MaxADC_GEM_X1->Fit(f_Landau, "MeV", "", 0., adcMaxAxis);
    lat1->DrawLatex(0.5, 0.7, Form("MPV = %1.1f", f_Landau->GetParameter(1)));
    c1->Print(Form("Figs/GEM_MAX_ADC_X_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c1->Print(Form("Figs/GEM_MAX_ADC_X_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c1->Print(Form("Figs/GEM_MAX_ADC_X_%d_t%1.1f_m%d.root", run, threshold, MinClSize));

    TH1D *h_U_Coord_MultrCl1 = (TH1D*) file_in->Get("h_U_Coord_MultrCl1");
    h_U_Coord_MultrCl1->SetTitle("; Cluster U coordinate [strip]");
    h_U_Coord_MultrCl1->SetLineWidth(2);
    h_U_Coord_MultrCl1->Draw();
    c1->Print(Form("Figs/cl_U_coordinate1_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c1->Print(Form("Figs/cl_U_coordinate1_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c1->Print(Form("Figs/cl_U_coordinate1_%d_t%1.1f_m%d.root", run, threshold, MinClSize));

    TH1D *h_V_Coord_MultrCl1 = (TH1D*) file_in->Get("h_V_Coord_MultrCl1");
    h_V_Coord_MultrCl1->SetTitle("; Cluster V coordinate [strip]");
    h_V_Coord_MultrCl1->SetLineWidth(2);
    h_V_Coord_MultrCl1->Draw();
    c1->Print(Form("Figs/cl_V_coordinate1_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c1->Print(Form("Figs/cl_V_coordinate1_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c1->Print(Form("Figs/cl_V_coordinate1_%d_t%1.1f_m%d.root", run, threshold, MinClSize));

    TH1D *h_U_Coord_MultrCl2 = (TH1D*) file_in->Get("h_U_Coord_MultrCl2");
    h_U_Coord_MultrCl2->SetTitle("; Cluster U coordinate [strip]");
    h_U_Coord_MultrCl2->SetLineWidth(2);
    h_U_Coord_MultrCl2->Draw();
    c1->Print(Form("Figs/cl_U_coordinate2_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c1->Print(Form("Figs/cl_U_coordinate2_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c1->Print(Form("Figs/cl_U_coordinate2_%d_t%1.1f_m%d.root", run, threshold, MinClSize));

    TH1D *h_V_Coord_MultrCl2 = (TH1D*) file_in->Get("h_V_Coord_MultrCl2");
    h_V_Coord_MultrCl2->SetTitle("; Cluster V coordinate [strip]");
    h_V_Coord_MultrCl2->SetLineWidth(2);
    h_V_Coord_MultrCl2->Draw();
    c1->Print(Form("Figs/cl_V_coordinate2_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c1->Print(Form("Figs/cl_V_coordinate2_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c1->Print(Form("Figs/cl_V_coordinate2_%d_t%1.1f_m%d.root", run, threshold, MinClSize));


    c1->SetLogz();
    TH2D *h_n_GEM_Y_vs_X_Hits1 = (TH2D*) file_in->Get("h_n_GEM_Y_vs_X_Hits1");
    h_n_GEM_Y_vs_X_Hits1->SetTitle("; The number of X hits in GEM; The number of Y hits in GEM");
    h_n_GEM_Y_vs_X_Hits1->Draw("colz");
    line1->DrawLine(1.5, 1.5, 1.5, 20.);
    line1->DrawLine(1.5, 1.5, 20., 1.5);
    c1->Print(Form("Figs/GEM_N_Y_vs_X_Hits1_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c1->Print(Form("Figs/GEM_N_Y_vs_X_Hits1_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c1->Print(Form("Figs/GEM_N_Y_vs_X_Hits1_%d_t%1.1f_m%d.root", run, threshold, MinClSize));

    TH2D *h_GEM_XY1 = (TH2D*) file_in->Get("h_GEM_XY1");
    h_GEM_XY1->SetTitle("; GEM X Strip; GEM Y Strip");
    h_GEM_XY1->Draw("colz");
    c1->Print(Form("Figs/GEM_Y_vs_X_Strips_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c1->Print(Form("Figs/GEM_Y_vs_X_Strips_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c1->Print(Form("Figs/GEM_Y_vs_X_Strips_%d_t%1.1f_m%d.root", run, threshold, MinClSize));

    TH2D *h_n_uRwell_V_vs_U_MultiHitCl = (TH2D*) file_in->Get("h_n_uRwell_V_vs_U_MultiHitCl");
    h_n_uRwell_V_vs_U_MultiHitCl->SetTitle("; Number of U clusters; umber of V clusters");
    int binx1 = h_n_uRwell_V_vs_U_MultiHitCl->GetXaxis()->FindBin(1);
    int binx2 = h_n_uRwell_V_vs_U_MultiHitCl->GetNbinsX() + 1;
    int biny1 = h_n_uRwell_V_vs_U_MultiHitCl->GetYaxis()->FindBin(1);
    int biny2 = h_n_uRwell_V_vs_U_MultiHitCl->GetNbinsY() + 1;
    double counts_integral = h_n_uRwell_V_vs_U_MultiHitCl->Integral(0, binx2, 0, biny2);
    int counts_has_U_Cluster = h_n_uRwell_V_vs_U_MultiHitCl->Integral(binx1, binx2, 1, biny2);
    int counts_has_V_Cluster = h_n_uRwell_V_vs_U_MultiHitCl->Integral(1, binx2, biny1, biny2);
    int counts_has_AnyCluster = counts_integral - h_n_uRwell_V_vs_U_MultiHitCl->GetBinContent(1, 1);
    int counts_has_U_AND_V_Cluster = h_n_uRwell_V_vs_U_MultiHitCl->Integral(binx1, binx2, biny1, biny2);
    h_n_uRwell_V_vs_U_MultiHitCl->SetAxisRange(0, 10., "Y");
    h_n_uRwell_V_vs_U_MultiHitCl->SetAxisRange(0, 10., "X");
    h_n_uRwell_V_vs_U_MultiHitCl->Draw("colz text");
    lat1->SetTextSize(0.03);
    lat1->DrawLatex(0.15, 0.9, Form("Has U cluster = %d #rightarrow %1.2f %%", counts_has_U_Cluster, 100. * double(counts_has_U_Cluster) / counts_integral));
    lat1->DrawLatex(0.15, 0.85, Form("Has V cluster = %d #rightarrow %1.2f %% ", counts_has_V_Cluster, 100. * double(counts_has_V_Cluster) / counts_integral));
    lat1->DrawLatex(0.15, 0.8, Form("Has any cluster = %d #rightarrow %1.2f %% ", counts_has_AnyCluster, 100. * double(counts_has_AnyCluster) / counts_integral));
    lat1->DrawLatex(0.15, 0.75, Form("Has U and V cluster = %d #rightarrow %1.2f %% ", counts_has_U_AND_V_Cluster, 100. * double(counts_has_U_AND_V_Cluster) / counts_integral));

    TCanvas *c2 = new TCanvas("c2", "", 1800., 1000.);
    c2->SetTopMargin(0.03);
    c2->SetRightMargin(0.11);
    c2->SetBottomMargin(0.13);
    c2->SetLeftMargin(0.12);

    TH2D *h_Cross_YXc2 = (TH2D*) file_in->Get("h_Cross_YXc2");
    h_Cross_YXc2->SetStats(0);
    h_Cross_YXc2->SetTitle("; Cross X coordinate [mm]; Cross Y coordinate [mm]");
    h_Cross_YXc2->SetMinimum(3);
    h_Cross_YXc2->Draw("col");
    DrawActiveArea();
    uRwellTools::DrawGroupStripBiundaries();
    c2->Print(Form("Figs/Cross_YXc2_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc2_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc2_%d_t%1.1f_m%d.root", run, threshold, MinClSize));

    TF1 *f_Gaus = new TF1("f_Gaus", "[0]*TMath::Gaus(x, [1], [2])", -900., 900.);
    f_Gaus->SetNpx(4500);
    TF1 *f_GPol4 = new TF1("f_GPol4", "[0]*TMath::Gaus(x, [1], [2]) + [3] + x*( [4] + x*( [5] + x*( [6] + x*[7] ) )  )", -900., 900.);
    f_GPol4->SetNpx(4500);
    TF1 *f_Pol4 = new TF1("f_Pol4", "[0] + x*( [1] + x*( [2] + x*( [3] + x*[4] ) )  )", -900., 900.);
    f_Pol4->SetNpx(4500);
    f_Pol4->SetLineColor(4);

    TH1D *h_Cross_X2 = (TH1D*) h_Cross_YXc2->ProjectionX("h_Cross_X2", 1, h_Cross_YXc2->GetNbinsY());
    h_Cross_X2->Draw();
    f_GPol4->SetParameters(h_Cross_X2->GetMaximum(), h_Cross_X2->GetBinCenter(h_Cross_X2->GetMaximumBin()), 75.);
    h_Cross_X2->Fit(f_GPol4, "MeV", "", -800, 800.);
    double pars[8];
    f_GPol4->GetParameters(pars);
    f_Gaus->SetParameters(pars);
    f_Pol4->SetParameters(&pars[3]);
    f_Pol4->Draw("Same");
    double N_Cross_BgrSubtr = f_Gaus->Integral(-800., 800.) / h_Cross_X2->GetBinWidth(15);
    c2->Print(Form("Figs/Cross_X2_Fit_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_X2_Fit_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_X2_Fit_%d_t%1.1f_m%d.root", run, threshold, MinClSize));

    c1->cd();
    lat1->DrawLatex(0.15, 0.7, Form("Has Cross BgrSubtr = %d #rightarrow %1.2f %% ", int(N_Cross_BgrSubtr), 100. * double(N_Cross_BgrSubtr) / counts_integral));
    c1->Print(Form("Figs/Number_OF_V_vs_U_clusters_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c1->Print(Form("Figs/Number_OF_V_vs_U_clusters_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c1->Print(Form("Figs/Number_OF_V_vs_U_clusters_%d_t%1.1f_m%d.root", run, threshold, MinClSize));

    c2->cd();
    TH2D *h_Cross_YXc1 = (TH2D*) file_in->Get("h_Cross_YXc1");
    h_Cross_YXc1->SetStats(0);
    h_Cross_YXc1->SetTitle("; Cross X coordinate [mm]; Cross Y coordinate [mm]");
    h_Cross_YXc1->SetTitleSize(0.05, "Y");
    h_Cross_YXc1->SetLabelSize(0.05, "Y");
    h_Cross_YXc1->SetTitleOffset(0.9, "Y");
    h_Cross_YXc1->SetTitleSize(0.05, "X");
    h_Cross_YXc1->SetLabelSize(0.05, "X");
    h_Cross_YXc1->SetMaximum(h_Cross_YXc1->GetMaximum() / 5.);
    h_Cross_YXc1->SetMinimum(1.5);
    h_Cross_YXc1->Draw("col");
    DrawActiveArea();
    c2->Print(Form("Figs/Cross_YXc1_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc1_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc1_%d_t%1.1f_m%d.root", run, threshold, MinClSize));

    TH2D *h_Cross_YXc3 = (TH2D*) file_in->Get("h_Cross_YXc3");
    h_Cross_YXc3->SetStats(0);
    h_Cross_YXc3->SetTitle("; Cross X coordinate [mm]; Cross Y coordinate [mm]");
    h_Cross_YXc3->SetTitleSize(0.05, "Y");
    h_Cross_YXc3->SetLabelSize(0.05, "Y");
    h_Cross_YXc3->SetTitleOffset(0.9, "Y");
    h_Cross_YXc3->SetTitleSize(0.05, "X");
    h_Cross_YXc3->SetLabelSize(0.05, "X");
    h_Cross_YXc3->Draw("col");
    h_Cross_YXc3->SetMaximum(h_Cross_YXc3->GetMaximum() / 5.);
    uRwellTools::DrawGroupStripBiundaries();
    c2->Print(Form("Figs/Cross_YXc3_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc3_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc3_%d_t%1.1f_m%d.root", run, threshold, MinClSize));

    int Y_bin2 = h_Cross_YXc3->GetYaxis()->FindBin(245);
    int Y_bin1 = h_Cross_YXc3->GetYaxis()->FindBin(-245);
    TH1D *h_Cross_X3 = (TH1D*) h_Cross_YXc3->ProjectionX("h_Cross_X3", Y_bin1, Y_bin2);
    h_Cross_X3->SetLineColor(95);
    h_Cross_X3->SetFillColor(95);
    h_Cross_X3->Draw();

    c2->Print(Form("Figs/Cross_X3_%d_%1.1f_%d.pdf", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_X3_%d_%1.1f_%d.png", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_X3_%d_%1.1f_%d.root", run, threshold, MinClSize));

    h_Cross_X3->SetAxisRange(-90., 90.);
    h_Cross_X3->SetMinimum(0.);
    h_Cross_X3->SetStats(0);
    h_Cross_X3->SetTitleSize(0.05, "X");
    h_Cross_X3->SetLabelSize(0.05, "X");
    h_Cross_X3->SetLabelSize(0.05, "Y");
    c2->SetGridy();
    c2->SetGridx();
    c2->Modified();
    c2->Update();
    c2->Print(Form("Figs/Cross_X3_%d_%1.1f_%d_ZoomedOnX.pdf", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_X3_%d_%1.1f_%d_ZoomedOnX.png", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_X3_%d_%1.1f_%d_ZoomedOnX.root", run, threshold, MinClSize));


    TH2D *h_Cross_YXc_Weighted3 = (TH2D*) file_in->Get("h_Cross_YXc_Weighted3");
    h_Cross_YXc_Weighted3->SetStats(0);
    h_Cross_YXc_Weighted3->SetTitle("; Cross X coordinate [mm]; Cross Y coordinate [mm]");
    h_Cross_YXc_Weighted3->SetTitleSize(0.05, "Y");
    h_Cross_YXc_Weighted3->SetLabelSize(0.05, "Y");
    h_Cross_YXc_Weighted3->SetTitleOffset(0.9, "Y");
    h_Cross_YXc_Weighted3->SetTitleSize(0.05, "X");
    h_Cross_YXc_Weighted3->SetLabelSize(0.05, "X");
    h_Cross_YXc_Weighted3->Draw("col");
    h_Cross_YXc_Weighted3->SetMaximum(h_Cross_YXc_Weighted3->GetMaximum() / 5.);
    uRwellTools::DrawGroupStripBiundaries();
    c2->Print(Form("Figs/Cross_YXc3_Weighted_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc3_Weighted_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc3_Weighted_%d_t%1.1f_m%d.root", run, threshold, MinClSize));

    TH2D *h_AvgADC_Cross_YXc3 = (TH2D*) h_Cross_YXc_Weighted3->Clone("h_AvgADC_Cross_YXc3");
    h_AvgADC_Cross_YXc3->Divide(h_Cross_YXc3);
    h_AvgADC_Cross_YXc3->Draw("colz");
    h_AvgADC_Cross_YXc3->SetMaximum(h_AvgADC_Cross_YXc3->GetMaximum() / 2.);
    c2->Print(Form("Figs/AvgADC_Cross_YXC3_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c2->Print(Form("Figs/AvgADC_Cross_YXC3_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c2->Print(Form("Figs/AvgADC_Cross_YXC3_%d_t%1.1f_m%d.root", run, threshold, MinClSize));

    Y_bin2 = h_Cross_YXc_Weighted3->GetYaxis()->FindBin(245);
    Y_bin1 = h_Cross_YXc_Weighted3->GetYaxis()->FindBin(-245);


    TH1D *h_Cross_X_Weighted3 = (TH1D*) h_Cross_YXc_Weighted3->ProjectionX("h_Cross_X_Weighted3", Y_bin1, Y_bin2);
    h_Cross_X_Weighted3->SetLineColor(4);
    h_Cross_X_Weighted3->SetFillColor(4);
    int bin2Nomalize = h_Cross_X_Weighted3->FindBin(10.);
    h_Cross_X3->Scale(h_Cross_X_Weighted3->GetBinContent(bin2Nomalize) / h_Cross_X3->GetBinContent(bin2Nomalize));
    h_Cross_X3->GetXaxis()->UnZoom();

    TLegend *leg2 = new TLegend(0.15, 0.7, 0.3, 0.97);
    leg2->SetBorderSize(0);
    leg2->AddEntry(h_Cross_X_Weighted3, "Weighted");
    leg2->AddEntry(h_Cross_X3, "Not Weighted");
    h_Cross_X_Weighted3->SetStats(0);
    h_Cross_X_Weighted3->Draw("hist");
    h_Cross_X3->Draw("hist same");
    leg2->Draw();
    c2->Print(Form("Figs/Cross_X3_Weighted_Unweighted_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_X3_Weighted_Unweighted_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_X3_Weighted_Unweighted_%d_t%1.1f_m%d.root", run, threshold, MinClSize));

    
    TH2D *h_Cross_YXc_Max1 = (TH2D*) file_in->Get("h_Cross_YXc_Max1");
    h_Cross_YXc_Max1->SetStats(0);
    h_Cross_YXc_Max1->SetTitle("; Cross X coordinate [mm]; Cross Y coordinate [mm]");
    h_Cross_YXc_Max1->SetTitleSize(0.05, "Y");
    h_Cross_YXc_Max1->SetLabelSize(0.05, "Y");
    h_Cross_YXc_Max1->SetTitleOffset(0.9, "Y");
    h_Cross_YXc_Max1->SetTitleSize(0.05, "X");
    h_Cross_YXc_Max1->SetLabelSize(0.05, "X");
    //h_Cross_YXc_Max1->SetMaximum( 0.15 * h_Cross_YXc1->GetMaximum() );
    h_Cross_YXc_Max1->SetMaximum( 0.5*h_Cross_YXc1->GetEntries()/( h_Cross_YXc1->GetNbinsX()*h_Cross_YXc1->GetNbinsY() ) );
    h_Cross_YXc_Max1->SetMinimum(3.5);
    h_Cross_YXc_Max1->Draw("col");
    DrawActiveArea();
    uRwellTools::DrawGroupStripBiundaries();
    c2->Print(Form("Figs/Cross_YXc_Max1_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc_Max1_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc_Max1_%d_t%1.1f_m%d.root", run, threshold, MinClSize));

    
    TH2D *h_Cross_YXc_Max_InsideDet1 = (TH2D*) file_in->Get("h_Cross_YXc_Max_InsideDet1");
    h_Cross_YXc_Max_InsideDet1->SetStats(0);
    h_Cross_YXc_Max_InsideDet1->SetTitle("; Cross X coordinate [mm]; Cross Y coordinate [mm]");
    h_Cross_YXc_Max_InsideDet1->SetTitleSize(0.07, "Y");
    h_Cross_YXc_Max_InsideDet1->SetLabelSize(0.07, "Y");
    h_Cross_YXc_Max_InsideDet1->SetTitleOffset(0.9, "Y");
    h_Cross_YXc_Max_InsideDet1->SetTitleSize(0.07, "X");
    h_Cross_YXc_Max_InsideDet1->SetLabelSize(0.07, "X");
    h_Cross_YXc_Max_InsideDet1->SetTitleOffset(0.9, "X");
    //h_Cross_YXc_Max1->SetMaximum( 0.15 * h_Cross_YXc1->GetMaximum() );
    h_Cross_YXc_Max_InsideDet1->SetMaximum( 0.5*h_Cross_YXc1->GetEntries()/( h_Cross_YXc1->GetNbinsX()*h_Cross_YXc1->GetNbinsY() ) );
    h_Cross_YXc_Max_InsideDet1->SetMinimum(3.5);
    h_Cross_YXc_Max_InsideDet1->Draw("col");
    c2->Print(Form("Figs/Cross_YXc_Max_InsideDet1_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc_Max_InsideDet1_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc_Max_InsideDet1_%d_t%1.1f_m%d.root", run, threshold, MinClSize));

    
    
    Y_bin2 = h_Cross_YXc3->GetYaxis()->FindBin(245);
    Y_bin1 = h_Cross_YXc3->GetYaxis()->FindBin(-245);
    TH1D *h_Cross_X_Max1 = (TH1D*) h_Cross_YXc_Max1->ProjectionX("h_Cross_X_Max1", Y_bin1, Y_bin2);
    h_Cross_X_Max1->SetLineColor(95);
    h_Cross_X_Max1->SetFillColor(95);
    h_Cross_X_Max1->SetTitleSize(0.07, "Y");
    h_Cross_X_Max1->SetLabelSize(0.07, "Y");
    h_Cross_X_Max1->SetTitleOffset(0.9, "Y");
    h_Cross_X_Max1->SetTitleSize(0.07, "X");
    h_Cross_X_Max1->SetLabelSize(0.07, "X");
    h_Cross_X_Max1->SetTitleOffset(0.9, "X");
    h_Cross_X_Max1->SetAxisRange(-85, 85, "X");
    h_Cross_X_Max1->Draw();

    c2->Print(Form("Figs/Cross_X_Max1_%d_%1.1f_%d.pdf", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_X_Max1_%d_%1.1f_%d.png", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_X_Max1_%d_%1.1f_%d.root", run, threshold, MinClSize));
    
    
    
    TH2D *h_Cross_YXc_Max2 = (TH2D*) file_in->Get("h_Cross_YXc_Max2");
    h_Cross_YXc_Max2->SetStats(0);
    h_Cross_YXc_Max2->SetTitle("; Cross X coordinate [mm]; Cross Y coordinate [mm]");
    h_Cross_YXc_Max2->SetTitleSize(0.05, "Y");
    h_Cross_YXc_Max2->SetLabelSize(0.05, "Y");
    h_Cross_YXc_Max2->SetTitleOffset(0.9, "Y");
    h_Cross_YXc_Max2->SetTitleSize(0.05, "X");
    h_Cross_YXc_Max2->SetLabelSize(0.05, "X");
    h_Cross_YXc_Max2->Draw("col");
    DrawActiveArea();
    c2->Print(Form("Figs/Cross_YXc_Max2_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc_Max2_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc_Max2_%d_t%1.1f_m%d.root", run, threshold, MinClSize));
    
    
    TH2D *h_Cross_YXc_Max_Weighted1 = (TH2D*)file_in->Get("h_Cross_YXc_Max_Weighted1");
    h_Cross_YXc_Max_Weighted1->Divide(h_Cross_YXc_Max1);
    h_Cross_YXc_Max_Weighted1->SetTitle("; Cross X coordinate [mm]; Cross Y coordinate [mm]");
    h_Cross_YXc_Max_Weighted1->SetTitleSize(0.05, "Y");
    h_Cross_YXc_Max_Weighted1->SetLabelSize(0.05, "Y");
    h_Cross_YXc_Max_Weighted1->SetTitleOffset(0.9, "Y");
    h_Cross_YXc_Max_Weighted1->SetTitleSize(0.05, "X");
    h_Cross_YXc_Max_Weighted1->SetLabelSize(0.05, "X");
    //h_Cross_YXc_Max_Weighted1->SetMinimum(1.5);
    //h_Cross_YXc_Max_Weighted1->SetMaximum(0.15*h_Cross_YXc_Max_Weighted1->GetMaximum() );
    h_Cross_YXc_Max_Weighted1->Draw("colz");
    c2->Print(Form("Figs/Cross_YXc_Max1_Weighted_TotADC_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc_Max1_Weighted_TotADC_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc_Max1_Weighted_TotADC_%d_t%1.1f_m%d.root", run, threshold, MinClSize));
    
    TH2D *h_Cross_YXc_Max_Weighted_ADC_U1 = (TH2D*)file_in->Get("h_Cross_YXc_Max_Weighted_ADC_U1");
    h_Cross_YXc_Max_Weighted_ADC_U1->Divide(h_Cross_YXc_Max1);
    h_Cross_YXc_Max_Weighted_ADC_U1->SetTitle("; Cross X coordinate [mm]; Cross Y coordinate [mm]");
    h_Cross_YXc_Max_Weighted_ADC_U1->SetTitleSize(0.05, "Y");
    h_Cross_YXc_Max_Weighted_ADC_U1->SetLabelSize(0.05, "Y");
    h_Cross_YXc_Max_Weighted_ADC_U1->SetTitleOffset(0.9, "Y");
    h_Cross_YXc_Max_Weighted_ADC_U1->SetTitleSize(0.05, "X");
    h_Cross_YXc_Max_Weighted_ADC_U1->SetLabelSize(0.05, "X");
//    h_Cross_YXc_Max_Weighted_ADC_U1->SetMinimum(1.5);
//    h_Cross_YXc_Max_Weighted_ADC_U1->SetMaximum(0.15*h_Cross_YXc_Max_Weighted_ADC_U1->GetMaximum() );
    h_Cross_YXc_Max_Weighted_ADC_U1->Draw("colz");
    c2->Print(Form("Figs/Cross_YXc_Max1_Weighted_ADC_U_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc_Max1_Weighted_ADC_U_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc_Max1_Weighted_ADC_U_%d_t%1.1f_m%d.root", run, threshold, MinClSize));

    TH2D *h_Cross_YXc_Max_Weighted_ADC_V1 = (TH2D*)file_in->Get("h_Cross_YXc_Max_Weighted_ADC_V1");
    h_Cross_YXc_Max_Weighted_ADC_V1->Divide(h_Cross_YXc_Max1);
    h_Cross_YXc_Max_Weighted_ADC_V1->SetTitle("; Cross X coordinate [mm]; Cross Y coordinate [mm]");
    h_Cross_YXc_Max_Weighted_ADC_V1->SetTitleSize(0.05, "Y");
    h_Cross_YXc_Max_Weighted_ADC_V1->SetLabelSize(0.05, "Y");
    h_Cross_YXc_Max_Weighted_ADC_V1->SetTitleOffset(0.9, "Y");
    h_Cross_YXc_Max_Weighted_ADC_V1->SetTitleSize(0.05, "X");
    h_Cross_YXc_Max_Weighted_ADC_V1->SetLabelSize(0.05, "X");
//    h_Cross_YXc_Max_Weighted_ADC_V1->SetMinimum(1.5);
//    h_Cross_YXc_Max_Weighted_ADC_V1->SetMaximum(0.15*h_Cross_YXc_Max_Weighted_ADC_V1->GetMaximum() );
    h_Cross_YXc_Max_Weighted_ADC_V1->Draw("colz");
    c2->Print(Form("Figs/Cross_YXc_Max1_Weighted_ADC_V_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc_Max1_Weighted_ADC_V_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc_Max1_Weighted_ADC_V_%d_t%1.1f_m%d.root", run, threshold, MinClSize));

    TH2D *h_Cross_YXc_Max_Weighted_tU1 = (TH2D*)file_in->Get("h_Cross_YXc_Max_Weighted_tU1");
    h_Cross_YXc_Max_Weighted_tU1->Divide(h_Cross_YXc_Max1);
    h_Cross_YXc_Max_Weighted_tU1->SetTitle("; Cross X coordinate [mm]; Cross Y coordinate [mm]");
    h_Cross_YXc_Max_Weighted_tU1->SetTitleSize(0.05, "Y");
    h_Cross_YXc_Max_Weighted_tU1->SetLabelSize(0.05, "Y");
    h_Cross_YXc_Max_Weighted_tU1->SetTitleOffset(0.9, "Y");
    h_Cross_YXc_Max_Weighted_tU1->SetTitleSize(0.05, "X");
    h_Cross_YXc_Max_Weighted_tU1->SetLabelSize(0.05, "X");
//    h_Cross_YXc_Max_Weighted_tU1->SetMinimum(1.5);
//    h_Cross_YXc_Max_Weighted_tU1->SetMaximum(0.05*h_Cross_YXc_Max_Weighted_tU1->GetMaximum() );
    h_Cross_YXc_Max_Weighted_tU1->Draw("colz");
    c2->Print(Form("Figs/Cross_YXc_Max1_Weighted_tU_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc_Max1_Weighted_tU_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc_Max1_Weighted_tU_%d_t%1.1f_m%d.root", run, threshold, MinClSize));
    
    TH2D *h_Cross_YXc_Max_Weighted_tV1 = (TH2D*)file_in->Get("h_Cross_YXc_Max_Weighted_tV1");
    h_Cross_YXc_Max_Weighted_tV1->Divide(h_Cross_YXc_Max1);
    h_Cross_YXc_Max_Weighted_tV1->SetTitle("; Cross X coordinate [mm]; Cross Y coordinate [mm]");
    h_Cross_YXc_Max_Weighted_tV1->SetTitleSize(0.05, "Y");
    h_Cross_YXc_Max_Weighted_tV1->SetLabelSize(0.05, "Y");
    h_Cross_YXc_Max_Weighted_tV1->SetTitleOffset(0.9, "Y");
    h_Cross_YXc_Max_Weighted_tV1->SetTitleSize(0.05, "X");
    h_Cross_YXc_Max_Weighted_tV1->SetLabelSize(0.05, "X");
//    h_Cross_YXc_Max_Weighted_tV1->SetMinimum(1.5);
//    h_Cross_YXc_Max_Weighted_tV1->SetMaximum(0.05*h_Cross_YXc_Max_Weighted_tV1->GetMaximum() );
    h_Cross_YXc_Max_Weighted_tV1->Draw("colz");
    c2->Print(Form("Figs/Cross_YXc_Max1_Weighted_tV_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc_Max1_Weighted_tV_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc_Max1_Weighted_tV_%d_t%1.1f_m%d.root", run, threshold, MinClSize));
    
    TH2D *h_Cross_YXc_Max_Weighted_clUSize1 = (TH2D*)file_in->Get("h_Cross_YXc_Max_Weighted_clUSize1");
    h_Cross_YXc_Max_Weighted_clUSize1->Divide(h_Cross_YXc_Max1);
    h_Cross_YXc_Max_Weighted_clUSize1->SetTitle("; Cross X coordinate [mm]; Cross Y coordinate [mm]");
    h_Cross_YXc_Max_Weighted_clUSize1->SetTitleSize(0.05, "Y");
    h_Cross_YXc_Max_Weighted_clUSize1->SetLabelSize(0.05, "Y");
    h_Cross_YXc_Max_Weighted_clUSize1->SetTitleOffset(0.9, "Y");
    h_Cross_YXc_Max_Weighted_clUSize1->SetTitleSize(0.05, "X");
    h_Cross_YXc_Max_Weighted_clUSize1->SetLabelSize(0.05, "X");
//    h_Cross_YXc_Max_Weighted_clUSize1->SetMinimum(1.5);
//    h_Cross_YXc_Max_Weighted_clUSize1->SetMaximum(0.1*h_Cross_YXc_Max_Weighted_clUSize1->GetMaximum() );
    h_Cross_YXc_Max_Weighted_clUSize1->SetMaximum( 12 );
    h_Cross_YXc_Max_Weighted_clUSize1->Draw("colz");
    c2->Print(Form("Figs/Cross_YXc_Max1_Weighted_clU_Size_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc_Max1_Weighted_clU_Size_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc_Max1_Weighted_clU_Size_%d_t%1.1f_m%d.root", run, threshold, MinClSize));
    
    TH2D *h_Cross_YXc_Max_Weighted_clVSize1 = (TH2D*)file_in->Get("h_Cross_YXc_Max_Weighted_clVSize1");
    h_Cross_YXc_Max_Weighted_clVSize1->Divide(h_Cross_YXc_Max1);
    h_Cross_YXc_Max_Weighted_clVSize1->SetTitle("; Cross X coordinate [mm]; Cross Y coordinate [mm]");
    h_Cross_YXc_Max_Weighted_clVSize1->SetTitleSize(0.05, "Y");
    h_Cross_YXc_Max_Weighted_clVSize1->SetLabelSize(0.05, "Y");
    h_Cross_YXc_Max_Weighted_clVSize1->SetTitleOffset(0.9, "Y");
    h_Cross_YXc_Max_Weighted_clVSize1->SetTitleSize(0.05, "X");
    h_Cross_YXc_Max_Weighted_clVSize1->SetLabelSize(0.05, "X");
//    h_Cross_YXc_Max_Weighted_clVSize1->SetMinimum(1.5);
//    h_Cross_YXc_Max_Weighted_clVSize1->SetMaximum(0.1*h_Cross_YXc_Max_Weighted_clVSize1->GetMaximum() );
    h_Cross_YXc_Max_Weighted_clVSize1->SetMaximum(12 );
    h_Cross_YXc_Max_Weighted_clVSize1->Draw("colz");
    c2->Print(Form("Figs/Cross_YXc_Max1_Weighted_clV_Size_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc_Max1_Weighted_clV_Size_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc_Max1_Weighted_clV_Size_%d_t%1.1f_m%d.root", run, threshold, MinClSize));

    TH2D *h_Cross_YXc_Max_Weighted_TotEnergy1 = (TH2D*)file_in->Get("h_Cross_YXc_Max_Weighted_TotEnergy1");
    double weighted_Integral = h_Cross_YXc_Max_Weighted_TotEnergy1->Integral();
    h_Cross_YXc_Max_Weighted_TotEnergy1->Divide(h_Cross_YXc_Max1);
    h_Cross_YXc_Max_Weighted_TotEnergy1->SetTitle("; Cross X coordinate [mm]; Cross Y coordinate [mm]");
    h_Cross_YXc_Max_Weighted_TotEnergy1->SetTitleSize(0.05, "Y");
    h_Cross_YXc_Max_Weighted_TotEnergy1->SetLabelSize(0.05, "Y");
    h_Cross_YXc_Max_Weighted_TotEnergy1->SetTitleOffset(0.9, "Y");
    h_Cross_YXc_Max_Weighted_TotEnergy1->SetTitleSize(0.05, "X");
    h_Cross_YXc_Max_Weighted_TotEnergy1->SetLabelSize(0.05, "X");
//    h_Cross_YXc_Max_Weighted_TotEnergy1->SetMinimum(1.5);
//    h_Cross_YXc_Max_Weighted_TotEnergy1->SetMaximum(0.3*h_Cross_YXc_Max_Weighted_TotEnergy1->GetMaximum() );
    h_Cross_YXc_Max_Weighted_TotEnergy1->SetMaximum(6*weighted_Integral/h_Cross_YXc_Max1->Integral() );
    h_Cross_YXc_Max_Weighted_TotEnergy1->Draw("colz");
    c2->Print(Form("Figs/Cross_YXc_Max1_Weighted_TotEnergy_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc_Max1_Weighted_TotEnergy_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc_Max1_Weighted_TotEnergy_%d_t%1.1f_m%d.root", run, threshold, MinClSize));

    TH2D *h_Cross_YXc_Max_Weighted_Energy_U1 = (TH2D*)file_in->Get("h_Cross_YXc_Max_Weighted_Energy_U1");
    weighted_Integral = h_Cross_YXc_Max_Weighted_Energy_U1->Integral();
    h_Cross_YXc_Max_Weighted_Energy_U1->Divide(h_Cross_YXc_Max1);
    h_Cross_YXc_Max_Weighted_Energy_U1->SetTitle("; Cross X coordinate [mm]; Cross Y coordinate [mm]");
    h_Cross_YXc_Max_Weighted_Energy_U1->SetTitleSize(0.05, "Y");
    h_Cross_YXc_Max_Weighted_Energy_U1->SetLabelSize(0.05, "Y");
    h_Cross_YXc_Max_Weighted_Energy_U1->SetTitleOffset(0.9, "Y");
    h_Cross_YXc_Max_Weighted_Energy_U1->SetTitleSize(0.05, "X");
    h_Cross_YXc_Max_Weighted_Energy_U1->SetLabelSize(0.05, "X");
//    h_Cross_YXc_Max_Weighted_Energy_U1->SetMinimum(1.5);
//    h_Cross_YXc_Max_Weighted_Energy_U1->SetMaximum(0.3*h_Cross_YXc_Max_Weighted_Energy_U1->GetMaximum() );
    h_Cross_YXc_Max_Weighted_Energy_U1->SetMaximum(6.*weighted_Integral/h_Cross_YXc_Max1->Integral() );
    h_Cross_YXc_Max_Weighted_Energy_U1->Draw("colz");
    c2->Print(Form("Figs/Cross_YXc_Max1_Weighted_Energy_U_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc_Max1_Weighted_Energy_U_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc_Max1_Weighted_Energy_U_%d_t%1.1f_m%d.root", run, threshold, MinClSize));

    TH2D *h_Cross_YXc_Max_Weighted_Energy_V1 = (TH2D*)file_in->Get("h_Cross_YXc_Max_Weighted_Energy_V1");
    weighted_Integral = h_Cross_YXc_Max_Weighted_Energy_V1->Integral();
    h_Cross_YXc_Max_Weighted_Energy_V1->Divide(h_Cross_YXc_Max1);
    h_Cross_YXc_Max_Weighted_Energy_V1->SetTitle("; Cross X coordinate [mm]; Cross Y coordinate [mm]");
    h_Cross_YXc_Max_Weighted_Energy_V1->SetTitleSize(0.05, "Y");
    h_Cross_YXc_Max_Weighted_Energy_V1->SetLabelSize(0.05, "Y");
    h_Cross_YXc_Max_Weighted_Energy_V1->SetTitleOffset(0.9, "Y");
    h_Cross_YXc_Max_Weighted_Energy_V1->SetTitleSize(0.05, "X");
    h_Cross_YXc_Max_Weighted_Energy_V1->SetLabelSize(0.05, "X");
    //h_Cross_YXc_Max_Weighted_Energy_V1->SetMinimum(1.5);
    //h_Cross_YXc_Max_Weighted_Energy_V1->SetMaximum(0.3*h_Cross_YXc_Max_Weighted_Energy_V1->GetMaximum() );
    h_Cross_YXc_Max_Weighted_Energy_V1->SetMaximum(6*weighted_Integral/h_Cross_YXc_Max1->Integral() );
    h_Cross_YXc_Max_Weighted_Energy_V1->Draw("colz");
    c2->Print(Form("Figs/Cross_YXc_Max1_Weighted_Energy_V_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc_Max1_Weighted_Energy_V_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c2->Print(Form("Figs/Cross_YXc_Max1_Weighted_Energy_V_%d_t%1.1f_m%d.root", run, threshold, MinClSize));
    
    c1->cd()->SetLogz(0);
    c1->SetRightMargin(0.15);
    c1->SetLeftMargin(0.12);
    
    TH2D *h_t_V_vs_U_Max1 = (TH2D*)file_in->Get("h_t_V_vs_U_Max1");
    h_t_V_vs_U_Max1->SetTitle("; peak time sample U; peak time sample V");
    h_t_V_vs_U_Max1->SetTitleSize(0.05, "Y");
    h_t_V_vs_U_Max1->SetLabelSize(0.05, "Y");
    h_t_V_vs_U_Max1->SetTitleOffset(0.9, "Y");
    h_t_V_vs_U_Max1->SetTitleSize(0.05, "X");
    h_t_V_vs_U_Max1->SetLabelSize(0.05, "X");
    h_t_V_vs_U_Max1->Draw("colz");
    c1->Print(Form("Figs/Time_V_vs_Time_U_Max_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c1->Print(Form("Figs/Time_V_vs_Time_U_Max_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c1->Print(Form("Figs/Time_V_vs_Time_U_Max_%d_t%1.1f_m%d.root", run, threshold, MinClSize));
    
    TH1D *h_t_U_Max1 = (TH1D*)h_t_V_vs_U_Max1->ProjectionX("h_t_U_Max1", 1, h_t_V_vs_U_Max1->GetNbinsY() );
    h_t_U_Max1->Draw();
    c1->Print(Form("Figs/Time_U_Max_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c1->Print(Form("Figs/Time_U_Max_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c1->Print(Form("Figs/Time_U_Max_%d_t%1.1f_m%d.root", run, threshold, MinClSize));

    TH1D *h_t_V_Max1 = (TH1D*)h_t_V_vs_U_Max1->ProjectionY("h_t_V_Max1", 1, h_t_V_vs_U_Max1->GetNbinsX() );
    h_t_V_Max1->Draw();
    c1->Print(Form("Figs/Time_V_Max_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c1->Print(Form("Figs/Time_V_Max_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c1->Print(Form("Figs/Time_V_Max_%d_t%1.1f_m%d.root", run, threshold, MinClSize));
    
    c1->SetLogz();
    TH2D *h_clV_clU_size1 = (TH2D*)file_in->Get("h_clV_clU_size1");
    h_clV_clU_size1->SetTitle("; U cluster size; V cluster size");
    h_clV_clU_size1->Draw("colz");
    c1->Print(Form("Figs/cl_V_vs_U_Size_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c1->Print(Form("Figs/cl_V_vs_U_Size_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c1->Print(Form("Figs/cl_V_vs_U_Size_%d_t%1.1f_m%d.root", run, threshold, MinClSize));
    
    TH1D *h_Ucl_size1 = (TH1D*)h_clV_clU_size1->ProjectionX("h_Ucl_size1", 1, h_clV_clU_size1->GetNbinsY() );
    h_Ucl_size1->Draw();
    c1->Print(Form("Figs/Ucl_size1_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c1->Print(Form("Figs/Ucl_size1_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c1->Print(Form("Figs/Ucl_size1_%d_t%1.1f_m%d.root", run, threshold, MinClSize));

    TH1D *h_Vcl_size1 = (TH1D*)h_clV_clU_size1->ProjectionY("h_Vcl_size1", 1, h_clV_clU_size1->GetNbinsX() );
    h_Vcl_size1->Draw();
    c1->Print(Form("Figs/Vcl_size1_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c1->Print(Form("Figs/Vcl_size1_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c1->Print(Form("Figs/Vcl_size1_%d_t%1.1f_m%d.root", run, threshold, MinClSize));
    
    
    TH2D *h_GEM_cl_YXC1_MAX1 = (TH2D*)file_in->Get("h_GEM_cl_YXC1_MAX1");
    h_GEM_cl_YXC1_MAX1->SetTitle("; GEM X coordinate [cm]; GEM Y coordinate [cm]");
    
    c1->SetLogz(0);
    TH2D *h_GEM_cl_YXC_MAX_Weighted_ADC_Tot1 = (TH2D*)file_in->Get("h_GEM_cl_YXC_MAX_Weighted_ADC_Tot1");
    h_GEM_cl_YXC_MAX_Weighted_ADC_Tot1->Divide(h_GEM_cl_YXC1_MAX1);
    h_GEM_cl_YXC_MAX_Weighted_ADC_Tot1->Draw("colz");
    c1->Print(Form("Figs/GEM_YXC_Avg_ADC_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c1->Print(Form("Figs/GEM_YXC_Avg_ADC_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c1->Print(Form("Figs/GEM_YXC_Avg_ADC_%d_t%1.1f_m%d.root", run, threshold, MinClSize));
    
    TH2D *h_GEM_ADCY_vs_ADCX_Max1 = (TH2D*)file_in->Get("h_GEM_ADCY_vs_ADCX_Max1");
    h_GEM_ADCY_vs_ADCX_Max1->SetTitle("; ADC X; ADC Y ");
    h_GEM_ADCY_vs_ADCX_Max1->Draw();
    c1->Print(Form("Figs/GEM_ADC_YXC_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c1->Print(Form("Figs/GEM_ADC_YXC_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c1->Print(Form("Figs/GEM_ADC_YXC_%d_t%1.1f_m%d.root", run, threshold, MinClSize));
    
    TH1D *h_GEM_ADC_X1 = (TH1D*)h_GEM_ADCY_vs_ADCX_Max1->ProjectionX("h_GEM_ADC_X1", 1, h_GEM_ADCY_vs_ADCX_Max1->GetNbinsY() );
    h_GEM_ADC_X1->Draw();
    h_GEM_ADC_X1->Fit(f_Landau, "MeV", "", 0., 800);
    double peak_X = f_Landau->GetParameter(1);
    lat1->DrawLatex(0.65, 0.8, Form("MPV = %1.2f", peak_X));    
    c1->Print(Form("Figs/GEM_ADC_X_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c1->Print(Form("Figs/GEM_ADC_X_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c1->Print(Form("Figs/GEM_ADC_X_%d_t%1.1f_m%d.root", run, threshold, MinClSize));
    
    TH1D *h_GEM_ADC_Y1 = (TH1D*)h_GEM_ADCY_vs_ADCX_Max1->ProjectionY("h_GEM_ADC_Y1", 1, h_GEM_ADCY_vs_ADCX_Max1->GetNbinsX() );
    h_GEM_ADC_Y1->Draw();
    h_GEM_ADC_Y1->Fit(f_Landau, "MeV", "", 0., 800);
    peak_X = f_Landau->GetParameter(1);
    lat1->DrawLatex(0.65, 0.8, Form("MPV = %1.2f", peak_X));    
    c1->Print(Form("Figs/GEM_ADC_Y_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c1->Print(Form("Figs/GEM_ADC_Y_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c1->Print(Form("Figs/GEM_ADC_Y_%d_t%1.1f_m%d.root", run, threshold, MinClSize));
    
    TH2D *h_GEM_tY_vs_tX_Max1 = (TH2D*)file_in->Get("h_GEM_tY_vs_tX_Max1");
    h_GEM_tY_vs_tX_Max1->SetTitle("; GEM peak time X; GEM peak time Y");
    h_GEM_tY_vs_tX_Max1->Draw("colz");
    c1->Print(Form("Figs/GEM_Peak_time_Y_vs_X_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c1->Print(Form("Figs/GEM_Peak_time_Y_vs_X_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c1->Print(Form("Figs/GEM_Peak_time_Y_vs_X_%d_t%1.1f_m%d.root", run, threshold, MinClSize));
    
    TH1D *hGEM_peakTime_X1 = (TH1D*)h_GEM_tY_vs_tX_Max1->ProjectionX("hGEM_peakTime_X1", 1, h_GEM_tY_vs_tX_Max1->GetNbinsY() );
    hGEM_peakTime_X1->Draw();
    c1->Print(Form("Figs/GEM_Peak_time_X_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c1->Print(Form("Figs/GEM_Peak_time_X_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c1->Print(Form("Figs/GEM_Peak_time_X_%d_t%1.1f_m%d.root", run, threshold, MinClSize));
    
    TH1D *hGEM_peakTime_Y1 = (TH1D*)h_GEM_tY_vs_tX_Max1->ProjectionY("hGEM_peakTime_Y1", 1, h_GEM_tY_vs_tX_Max1->GetNbinsX() );
    hGEM_peakTime_Y1->Draw();
    c1->Print(Form("Figs/GEM_Peak_time_Y_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c1->Print(Form("Figs/GEM_Peak_time_Y_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c1->Print(Form("Figs/GEM_Peak_time_Y_%d_t%1.1f_m%d.root", run, threshold, MinClSize));
    
    
    TH2D *h_GEM_clSize_Y_vs_X_Max1 = (TH2D*)file_in->Get("h_GEM_clSize_Y_vs_X_Max1");
    h_GEM_clSize_Y_vs_X_Max1->SetTitle("; GEM X cluster size; GEM Y cluster size");
    h_GEM_clSize_Y_vs_X_Max1->Draw("colz");
    c1->Print(Form("Figs/GEM_cl_Size_Y_vs_X_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c1->Print(Form("Figs/GEM_cl_Size_Y_vs_X_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c1->Print(Form("Figs/GEM_cl_Size_Y_vs_X_%d_t%1.1f_m%d.root", run, threshold, MinClSize));
    
    TH1D *h_GEM_cl_Size_X1 = (TH1D*)h_GEM_clSize_Y_vs_X_Max1->ProjectionX("h_GEM_cl_Size_X1", 1, h_GEM_clSize_Y_vs_X_Max1->GetNbinsY() );
    h_GEM_cl_Size_X1->Draw();
    c1->Print(Form("Figs/GEM_cl_Size_X_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c1->Print(Form("Figs/GEM_cl_Size_X_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c1->Print(Form("Figs/GEM_cl_Size_X_%d_t%1.1f_m%d.root", run, threshold, MinClSize));
    
    TH1D *h_GEM_cl_Size_Y1 = (TH1D*)h_GEM_clSize_Y_vs_X_Max1->ProjectionY("h_GEM_cl_Size_Y1", 1, h_GEM_clSize_Y_vs_X_Max1->GetNbinsX() );
    h_GEM_cl_Size_Y1->Draw();
    c1->Print(Form("Figs/GEM_cl_Size_Y_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c1->Print(Form("Figs/GEM_cl_Size_Y_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c1->Print(Form("Figs/GEM_cl_Size_Y_%d_t%1.1f_m%d.root", run, threshold, MinClSize));
    
    
    TCanvas *c3 = new TCanvas("c3", "", 1600, 1200);
    c3->Divide(4, 3, 0., 0.);
    TCanvas *c4 = new TCanvas("c4", "", 1600, 1200);
    c4->Divide(4, 3, 0., 0.);
    TCanvas *c5 = new TCanvas("c5", "", 1600, 1200);
    c5->Divide(4, 3, 0., 0.);

    
    
    // We will fit distributions with a Landau Function
    f_Landau->SetNpx(4500);

    lat1->SetTextSize(0.08);
    for (int i = 0; i < uRwellTools::nGroups; i++) {

        TH2D *h_curADC_VU = (TH2D*) file_in->Get(Form("h_ADC_V_U_Max1_%d", i));
        h_curADC_VU->SetTitle("; ADC_U; ADC_V");
        c3->cd(i + 1);
        h_curADC_VU->Draw("colz");

        lat1->DrawLatex(0.15, 0.8, Form("Group %d", i));

        c4->cd(i + 1);

        TH1D *h_curADC_U = (TH1D*) h_curADC_VU->ProjectionX(Form("ADC_U_group_%d", i), 1, h_curADC_VU->GetNbinsY());
        double peak_X = h_curADC_U->GetBinCenter(h_curADC_U->GetMaximumBin());
        double init_sigm = peak_X / 5.; // trying to put an empiric relation
        double p0 = h_curADC_U->GetMaximum() * peak_X / init_sigm;
        f_Landau->SetParameters(p0, peak_X, init_sigm);
        h_curADC_U->Fit(f_Landau, "MeV", "", 0., 600);
        lat1->DrawLatex(0.25, 0.8, Form("Group %d", i));
        lat1->DrawLatex(0.25, 0.7, Form("MPV = %1.1f", f_Landau->GetParameter(1)));
        lat1->DrawLatex(0.25, 0.6, Form("Width U = %1.0f #mum", uRwellTools::m_UStripWidth.at(i)));
        lat1->DrawLatex(0.25, 0.5, Form("Width V = %1.0f #mum", uRwellTools::m_VStripWidth.at(i)));

        c5->cd(i + 1);

        TH1D *h_curADC_V = (TH1D*) h_curADC_VU->ProjectionY(Form("ADC_V_group_%d", i), 1, h_curADC_VU->GetNbinsX());
        peak_X = h_curADC_V->GetBinCenter(h_curADC_V->GetMaximumBin());
        init_sigm = peak_X / 5.; // trying to put an empiric relation
        p0 = h_curADC_V->GetMaximum() * peak_X / init_sigm;
        f_Landau->SetParameters(p0, peak_X, init_sigm);
        h_curADC_V->Fit(f_Landau, "MeV", "", 0., 600);
        lat1->DrawLatex(0.3, 0.8, Form("Group %d", i));
        lat1->DrawLatex(0.25, 0.7, Form("MPV = %1.1f", f_Landau->GetParameter(1)));
        lat1->DrawLatex(0.25, 0.6, Form("Width U = %1.0f #mum", uRwellTools::m_UStripWidth.at(i)));
        lat1->DrawLatex(0.25, 0.5, Form("Width V = %1.0f #mum", uRwellTools::m_VStripWidth.at(i)));
    }
    c3->Print(Form("Figs/ADC_VU_Max_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c3->Print(Form("Figs/ADC_VU_Max_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c3->Print(Form("Figs/ADC_VU_Max_%d_t%1.1f_m%d.root", run, threshold, MinClSize));

    c4->Print(Form("Figs/ADC_U_Max_Fit_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c4->Print(Form("Figs/ADC_U_Max_Fit_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c4->Print(Form("Figs/ADC_U_Max_Fit_%d_t%1.1f_m%d.root", run, threshold, MinClSize));

    c5->Print(Form("Figs/ADC_V_Max_Fit_%d_t%1.1f_m%d.pdf", run, threshold, MinClSize));
    c5->Print(Form("Figs/ADC_V_Max_Fit_%d_t%1.1f_m%d.png", run, threshold, MinClSize));
    c5->Print(Form("Figs/ADC_V_Max_Fit_%d_t%1.1f_m%d.root", run, threshold, MinClSize));

    return 0;
}

void DrawActiveArea() {

    TLine *line1 = new TLine();
    line1->SetLineColor(2);
    line1->SetLineWidth(3);
    line1->DrawLine(-723, 250., 723., 250.);
    line1->DrawLine(-506.14, -250., 506.14, -250.);
    line1->DrawLine(-506.14, -250., -723, 250.);
    line1->DrawLine(506.14, -250., 723, 250.);
}
