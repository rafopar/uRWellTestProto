//
// Created by rafopar on 11/12/25.
//

#include <cstdlib>

#include <TH2D.h>
#include <TH1D.h>
#include <TMath.h>
#include <TFile.h>
#include <TLatex.h>
#include <TCanvas.h>

#include <uRwellTools.h>
#include <cxxopts.hpp>

using namespace std;
using namespace uRwellTools;


int main( int argc, char *argv[] ) {

    cxxopts::Options options("AnaPulseFits", "Performs clustering and also does analysis on cosmic data");

    options.add_options()
            ("r,Run", "Run number", cxxopts::value<int>())
            ;

    auto parsed_options = options.parse(argc, argv);

    int run = 0;

    if (parsed_options.count("Run")) {
        run = parsed_options["Run"].as<int>();
    } else {
        cout << "The run number is not provided. Exiting..." << endl;
        exit(1);
    }

    const double ts2ns = 25; // Conversion factor from time sampe to ns

    TFile file_in(Form("AnaPulseFits_%d.root", run), "Read");

    auto c2 = new TCanvas("c2", "", 1800., 1000.);

    c2->SetTopMargin(0.03);
    c2->SetRightMargin(0.03);

    auto h_Cross_YXc_MaxIntegral1 = dynamic_cast<TH2D *>(file_in.Get("h_Cross_YXc_MaxIntegral1"));
    h_Cross_YXc_MaxIntegral1->Draw();
    h_Cross_YXc_MaxIntegral1->SetStats(0);
    h_Cross_YXc_MaxIntegral1->SetTitle("; Cross X coordinate [mm]; Cross Y coordinate [mm]");
    h_Cross_YXc_MaxIntegral1->SetTitleSize(0.05, "Y");
    h_Cross_YXc_MaxIntegral1->SetLabelSize(0.05, "Y");
    h_Cross_YXc_MaxIntegral1->SetTitleOffset(0.9, "Y");
    h_Cross_YXc_MaxIntegral1->SetTitleSize(0.05, "X");
    h_Cross_YXc_MaxIntegral1->SetLabelSize(0.05, "X");
    h_Cross_YXc_MaxIntegral1->SetMaximum(h_Cross_YXc_MaxIntegral1->GetMaximum() / 2.);
    c2->Print(Form("Figs/Cross_YXc_MaxIntegral1_%d.pdf", run));
    c2->Print(Form("Figs/Cross_YXc_MaxIntegral1_%d.png", run));
    c2->Print(Form("Figs/Cross_YXc_MaxIntegral1_%d.root", run));


    auto c1 = new TCanvas("c1", "", 900., 900.);
    c1->SetTopMargin(0.03);
    c1->SetRightMargin(0.03);
    c1->cd();

    auto h_Cross_V_vs_U_ClusterMPV1 = dynamic_cast<TH2D*>(file_in.Get("h_Cross_V_vs_U_ClusterMPV1"));
    h_Cross_V_vs_U_ClusterMPV1->SetStats(0);
    h_Cross_V_vs_U_ClusterMPV1->SetTitle("; U cluster MPV; V cluster MPV");
    h_Cross_V_vs_U_ClusterMPV1->SetTitleSize(0.05, "Y");
    h_Cross_V_vs_U_ClusterMPV1->SetLabelSize(0.05, "Y");
    h_Cross_V_vs_U_ClusterMPV1->SetTitleOffset(0.9, "Y");
    h_Cross_V_vs_U_ClusterMPV1->SetTitleSize(0.05, "X");
    h_Cross_V_vs_U_ClusterMPV1->SetLabelSize(0.05, "X");
    h_Cross_V_vs_U_ClusterMPV1->Draw();
    c1->Print(Form("Figs/Cross_V_vs_U_ClusterMPV1_%d.pdf", run));
    c1->Print(Form("Figs/Cross_V_vs_U_ClusterMPV1_%d.png", run));
    c1->Print(Form("Figs/Cross_V_vs_U_ClusterMPV1_%d.root", run));

    auto h_U_ClusterMPV1 = h_Cross_V_vs_U_ClusterMPV1->ProjectionX("h_U_ClusterMPV1", 1, h_Cross_V_vs_U_ClusterMPV1->GetNbinsX());
    h_U_ClusterMPV1->SetStats(0);
    h_U_ClusterMPV1->SetTitleSize(0.05, "Y");
    h_U_ClusterMPV1->SetLabelSize(0.05, "Y");
    h_U_ClusterMPV1->Draw();
    c1->Print(Form("Figs/U_ClusterMPV1_%d.pdf", run));
    c1->Print(Form("Figs/U_ClusterMPV1_%d.png", run));
    c1->Print(Form("Figs/U_ClusterMPV1_%d.root", run));

    auto h_Cross_V_vs_U_SeedMPV1 = dynamic_cast<TH2D *>(file_in.Get("h_Cross_V_vs_U_SeedMPV1"));
    h_Cross_V_vs_U_SeedMPV1->SetStats(0);
    h_Cross_V_vs_U_SeedMPV1->SetTitle(";U Seed MPV; V Seed MPV");
    h_Cross_V_vs_U_SeedMPV1->SetTitleSize(0.05, "Y");
    h_Cross_V_vs_U_SeedMPV1->SetLabelSize(0.05, "Y");
    h_Cross_V_vs_U_SeedMPV1->SetTitleOffset(0.9, "Y");
    h_Cross_V_vs_U_SeedMPV1->SetTitleSize(0.05, "X");
    h_Cross_V_vs_U_SeedMPV1->SetLabelSize(0.05, "X");
    h_Cross_V_vs_U_SeedMPV1->Draw();
    c1->Print(Form("Figs/Cross_V_vs_U_SeedMPV1_%d.pdf", run));
    c1->Print(Form("Figs/Cross_V_vs_U_SeedMPV1_%d.png", run));
    c1->Print(Form("Figs/Cross_V_vs_U_SeedMPV1_%d.root", run));

    auto h_U_SeedMPV1 = h_Cross_V_vs_U_SeedMPV1->ProjectionX("h_U_SeedMPV1", 1, h_Cross_V_vs_U_SeedMPV1->GetNbinsY() );
    h_U_SeedMPV1->SetStats(0);
    h_U_SeedMPV1->SetTitleSize(0.05, "Y");
    h_U_SeedMPV1->SetLabelSize(0.05, "Y");
    h_U_SeedMPV1->Draw();
    c1->Print(Form("Figs/U_SeedMPV1_%d.pdf", run));
    c1->Print(Form("Figs/U_SeedMPV1_%d.png", run));
    c1->Print(Form("Figs/U_SeedMPV1_%d.root", run));

    auto f_Gaus = new TF1("f_Gaus", "[0]*TMath::Gaus(x, [1], [2])", -15., 15.);
    f_Gaus->SetNpx(4500);
    f_Gaus->SetParLimits(2, 0., 50.);

    auto lat1 = new TLatex();
    lat1->SetNDC();

    auto h_Cross_ClusterTimeDiff1 = dynamic_cast<TH1D*>(file_in.Get("h_Cross_ClusterTimeDiff1"));
    h_Cross_ClusterTimeDiff1->SetStats(0);
    h_Cross_ClusterTimeDiff1->SetTitle(";U Cluster MPV - V cluster MPV [25 ns]");
    h_Cross_ClusterTimeDiff1->SetTitleSize(0.05, "Y");
    h_Cross_ClusterTimeDiff1->SetLabelSize(0.05, "Y");
    h_Cross_ClusterTimeDiff1->SetTitleSize(0.05, "X");
    h_Cross_ClusterTimeDiff1->SetLabelSize(0.05, "X");
    h_Cross_ClusterTimeDiff1->Draw();
    double mean = h_Cross_ClusterTimeDiff1->GetMean();
    double rms = h_Cross_ClusterTimeDiff1->GetRMS();
    f_Gaus->SetParameters(h_Cross_ClusterTimeDiff1->GetMaximum(), mean, 0.2*rms );
    h_Cross_ClusterTimeDiff1->Fit(f_Gaus, "MeV", "", mean - 0.5*rms, mean + 0.5*rms);
    mean = f_Gaus->GetParameter(1);
    double sigma = f_Gaus->GetParameter(2);
    lat1->DrawLatex(0.15, 0.9, Form("#mu = %1.2f ns", mean*ts2ns));
    lat1->DrawLatex(0.15, 0.8, Form("#sigma = %1.2f ns", sigma*ts2ns));
    c1->Print(Form("Figs/Cross_ClusterTimeDiff1_%d.pdf", run));
    c1->Print(Form("Figs/Cross_ClusterTimeDiff1_%d.png", run));
    c1->Print(Form("Figs/Cross_ClusterTimeDiff1_%d.root", run));


    auto h_Cross_SeedTimeDiff1 = dynamic_cast<TH1D*>(file_in.Get("h_Cross_SeedTimeDiff1"));
    h_Cross_SeedTimeDiff1->SetStats(0);
    h_Cross_SeedTimeDiff1->SetTitleSize(0.05, "Y");
    h_Cross_SeedTimeDiff1->SetLabelSize(0.05, "Y");
    h_Cross_SeedTimeDiff1->SetTitleSize(0.05, "X");
    h_Cross_SeedTimeDiff1->SetLabelSize(0.05, "X");
    h_Cross_SeedTimeDiff1->SetTitle(";U Seed MPV - V Seed MPV [25 ns]");
    h_Cross_SeedTimeDiff1->Draw();
    mean = h_Cross_ClusterTimeDiff1->GetMean();
    rms = h_Cross_ClusterTimeDiff1->GetRMS();
    f_Gaus->SetParameters(h_Cross_SeedTimeDiff1->GetMaximum(), mean, 0.2*rms );
    h_Cross_SeedTimeDiff1->Fit(f_Gaus, "MeV", "", mean - 0.5*rms, mean + 0.5*rms);
    mean = f_Gaus->GetParameter(1);
    sigma = f_Gaus->GetParameter(2);
    lat1->DrawLatex(0.15, 0.9, Form("#mu = %1.2f ns", mean*ts2ns));
    lat1->DrawLatex(0.15, 0.8, Form("#sigma = %1.2f ns", sigma*ts2ns));
    c1->Print(Form("Figs/Cross_SeedTimeDiff1_%d.pdf", run));
    c1->Print(Form("Figs/Cross_SeedTimeDiff1_%d.png", run));
    c1->Print(Form("Figs/Cross_SeedTimeDiff1_%d.root", run));

    auto h_Cross_ClusterStartTimeDiff1 = dynamic_cast<TH1D*>(file_in.Get("h_Cross_ClusterStartTimeDiff1"));
    h_Cross_ClusterStartTimeDiff1->SetStats(0);
    h_Cross_ClusterStartTimeDiff1->SetTitleSize(0.05, "Y");
    h_Cross_ClusterStartTimeDiff1->SetLabelSize(0.05, "Y");
    h_Cross_ClusterStartTimeDiff1->SetTitleSize(0.05, "X");
    h_Cross_ClusterStartTimeDiff1->SetLabelSize(0.05, "X");
    h_Cross_ClusterStartTimeDiff1->SetTitle("; U Cluster Start time - V Cluster Start time [25 ns]");
    h_Cross_ClusterStartTimeDiff1->Draw();
    mean = h_Cross_ClusterStartTimeDiff1->GetMean();
    rms = h_Cross_ClusterStartTimeDiff1->GetRMS();
    h_Cross_ClusterStartTimeDiff1->Fit(f_Gaus, "MeV", "", mean - 0.5*rms, mean + 0.5*rms);
    mean = f_Gaus->GetParameter(1);
    sigma = f_Gaus->GetParameter(2);
    lat1->DrawLatex(0.15, 0.9, Form("#mu = %1.2f ns", mean*ts2ns));
    lat1->DrawLatex(0.15, 0.8, Form("#sigma = %1.2f ns", sigma*ts2ns));
    c1->Print(Form("Figs/Cross_ClusterStartTimeDiff1_%d.pdf", run));
    c1->Print(Form("Figs/Cross_ClusterStartTimeDiff1_%d.png", run));
    c1->Print(Form("Figs/Cross_ClusterStartTimeDiff1_%d.root", run));

    auto h_Cross_SeedStartTimeDiff1 = dynamic_cast<TH1D*>(file_in.Get("h_Cross_SeedStartTimeDiff1"));
    h_Cross_SeedStartTimeDiff1->SetStats(0);
    h_Cross_SeedStartTimeDiff1->SetTitleSize(0.05, "Y");
    h_Cross_SeedStartTimeDiff1->SetLabelSize(0.05, "Y");
    h_Cross_SeedStartTimeDiff1->SetTitleSize(0.05, "X");
    h_Cross_SeedStartTimeDiff1->SetLabelSize(0.05, "X");
    h_Cross_SeedStartTimeDiff1->SetTitle("; U Seed Start time - V Seed Start time [25 ns]");
    rms = h_Cross_SeedStartTimeDiff1->GetRMS();
    mean = h_Cross_SeedStartTimeDiff1->GetMean();
    f_Gaus->SetParameters(h_Cross_SeedStartTimeDiff1->GetMaximum(), mean, 0.2*rms );
    h_Cross_SeedStartTimeDiff1->Fit(f_Gaus, "MeV", "", mean - 0.5*rms, mean + 0.5*rms);
    mean = f_Gaus->GetParameter(1);
    sigma = f_Gaus->GetParameter(2);
    lat1->DrawLatex(0.15, 0.9, Form("#mu = %1.2f ns", mean*ts2ns));
    lat1->DrawLatex(0.15, 0.8, Form("#sigma = %1.2f ns", sigma*ts2ns));
    c1->Print(Form("Figs/Cross_Seed_StartTimeDiff1_%d.pdf", run));
    c1->Print(Form("Figs/Cross_Seed_StartTimeDiff1_%d.png", run));
    c1->Print(Form("Figs/Cross_Seed_StartTimeDiff1_%d.root", run));

    auto h_Cross_ClusterTimeDiff_vs_U_clPulseIntegral1 = dynamic_cast<TH2D*>(file_in.Get("h_Cross_ClusterTimeDiff_vs_U_clPulseIntegral1"));
    h_Cross_ClusterTimeDiff_vs_U_clPulseIntegral1->SetStats(0);
    h_Cross_ClusterTimeDiff_vs_U_clPulseIntegral1->SetTitleSize(0.05, "Y");
    h_Cross_ClusterTimeDiff_vs_U_clPulseIntegral1->SetLabelSize(0.05, "Y");
    h_Cross_ClusterTimeDiff_vs_U_clPulseIntegral1->SetTitleOffset(0.9, "Y");
    h_Cross_ClusterTimeDiff_vs_U_clPulseIntegral1->SetTitleSize(0.05, "X");
    h_Cross_ClusterTimeDiff_vs_U_clPulseIntegral1->SetLabelSize(0.05, "X");
    h_Cross_ClusterTimeDiff_vs_U_clPulseIntegral1->GetXaxis()->SetNdivisions(404);
    h_Cross_ClusterTimeDiff_vs_U_clPulseIntegral1->SetTitle("; U Cluster Pulse Integral; Cluster time difference [25 ns]");
    h_Cross_ClusterTimeDiff_vs_U_clPulseIntegral1->Draw();
    c1->Print(Form("Figs/Cross_ClusterTimeDiff_vs_U_clPulseIntegral1_%d.pdf", run));
    c1->Print(Form("Figs/Cross_ClusterTimeDiff_vs_U_clPulseIntegral1_%d.png", run));
    c1->Print(Form("Figs/Cross_ClusterTimeDiff_vs_U_clPulseIntegral1_%d.root", run));

    int nbinsX = h_Cross_ClusterTimeDiff_vs_U_clPulseIntegral1->GetNbinsX();
    auto h_U_lowADC_ClustTimeDifference1 = h_Cross_ClusterTimeDiff_vs_U_clPulseIntegral1->ProjectionY("h_U_lowADC_ClustTimeDifference1", 1, 50);
    mean = h_U_lowADC_ClustTimeDifference1->GetMean();
    rms = h_U_lowADC_ClustTimeDifference1->GetRMS();
    f_Gaus->SetParameters(h_U_lowADC_ClustTimeDifference1->GetMaximum(), mean, 0.2*rms);
    h_U_lowADC_ClustTimeDifference1->Fit(f_Gaus, "MeV", "", mean - 0.55*rms, mean + 0.55*rms);
    mean = f_Gaus->GetParameter(1);
    sigma = f_Gaus->GetParameter(2);
    lat1->DrawLatex(0.15, 0.9, Form("#mu = %1.2f ns", mean*ts2ns));
    lat1->DrawLatex(0.15, 0.8, Form("#sigma = %1.2f ns", sigma*ts2ns));
    c1->Print(Form("Figs/U_lowADC_ClustTimeDifference1_%d.pdf", run));
    c1->Print(Form("Figs/U_lowADC_ClustTimeDifference1_%d.png", run));
    c1->Print(Form("Figs/U_lowADC_ClustTimeDifference1_%d.root", run));

    auto h_U_highADC_ClustTimeDifference1 = h_Cross_ClusterTimeDiff_vs_U_clPulseIntegral1->ProjectionY("h_U_highADC_ClustTimeDifference1", nbinsX - 50, nbinsX);
    mean = h_U_highADC_ClustTimeDifference1->GetMean();
    rms = h_U_highADC_ClustTimeDifference1->GetRMS();
    f_Gaus->SetParameters(h_U_highADC_ClustTimeDifference1->GetMaximum(), mean, 0.5*rms);
    h_U_highADC_ClustTimeDifference1->Fit(f_Gaus, "MeV", "", mean - 0.4*rms, mean + 0.4*rms);
    mean = f_Gaus->GetParameter(1);
    sigma = f_Gaus->GetParameter(2);
    lat1->DrawLatex(0.15, 0.9, Form("#mu = %1.2f ns", mean*ts2ns));
    lat1->DrawLatex(0.15, 0.8, Form("#sigma = %1.2f ns", sigma*ts2ns));
    c1->Print(Form("Figs/U_highADC_ClustTimeDifference1_%d.pdf", run));
    c1->Print(Form("Figs/U_highADC_ClustTimeDifference1_%d.png", run));
    c1->Print(Form("Figs/U_highADC_ClustTimeDifference1_%d.root", run));

    auto h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral1 = dynamic_cast<TH2D*>(file_in.Get("h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral1"));
    h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral1->SetStats(0);
    h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral1->SetTitleOffset(0.9, "Y");
    h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral1->SetTitleSize(0.05, "Y");
    h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral1->SetLabelSize(0.05, "Y");
    h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral1->SetTitleSize(0.05, "X");
    h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral1->SetLabelSize(0.05, "X");
    h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral1->SetTitle("; U Cluster Pulse Integral; Cluster Start Time difference [25 ns]");
    h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral1->GetXaxis()->SetNdivisions(404);
    h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral1->Draw();
    c1->Print(Form("Figs/Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral1_%d.pdf", run));
    c1->Print(Form("Figs/Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral1_%d.png", run));
    c1->Print(Form("Figs/Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral1_%d.root", run));

    auto h_U_lowADC_ClustStartTimeDifference1 = h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral1->ProjectionY("h_U_lowADC_ClustStartTimeDifference1", 1, 50);
    h_U_lowADC_ClustStartTimeDifference1->Draw();
    mean = h_U_lowADC_ClustStartTimeDifference1->GetMean();
    rms = h_U_lowADC_ClustStartTimeDifference1->GetRMS();
    f_Gaus->SetParameters(h_U_lowADC_ClustStartTimeDifference1->GetMaximum(), mean, 0.5*rms);
    h_U_lowADC_ClustStartTimeDifference1->Fit(f_Gaus, "MeV", "", mean - 0.55*rms, mean + 0.65*rms);
    mean = f_Gaus->GetParameter(1);
    sigma = f_Gaus->GetParameter(2);
    lat1->DrawLatex(0.15, 0.9, Form("#mu = %1.2f ns", mean*ts2ns));
    lat1->DrawLatex(0.15, 0.8, Form("#sigma = %1.2f ns", sigma*ts2ns));
    c1->Print(Form("Figs/U_lowADC_ClustStartTimeDifference1_%d.pdf", run));
    c1->Print(Form("Figs/U_lowADC_ClustStartTimeDifference1_%d.png", run));
    c1->Print(Form("Figs/U_lowADC_ClustStartTimeDifference1_%d.root", run));

    nbinsX = h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral1->GetNbinsX();
    auto h_U_highADC_ClustStartTimeDifference1 = h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral1->ProjectionY("h_U_highADC_ClustStartTimeDifference1", nbinsX - 50, nbinsX);
    mean = h_U_highADC_ClustStartTimeDifference1->GetMean();
    rms = h_U_highADC_ClustStartTimeDifference1->GetRMS();
    f_Gaus->SetParameters(h_U_highADC_ClustStartTimeDifference1->GetMaximum(), mean, 0.5*rms);
    h_U_highADC_ClustStartTimeDifference1->Fit(f_Gaus, "MeV", "", mean - 0.45*rms, mean + 0.2*rms);
    mean = f_Gaus->GetParameter(1);
    sigma = f_Gaus->GetParameter(2);
    lat1->DrawLatex(0.15, 0.9, Form("#mu = %1.2f ns", mean*ts2ns));
    lat1->DrawLatex(0.15, 0.8, Form("#sigma = %1.2f ns", sigma*ts2ns));
    c1->Print(Form("Figs/U_highADC_ClustStartTimeDifference1_%d.pdf", run));
    c1->Print(Form("Figs/U_highADC_ClustStartTimeDifference1_%d.png", run));
    c1->Print(Form("Figs/U_highADC_ClustStartTimeDifference1_%d.root", run));

    auto h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral_GoodWidth1 = dynamic_cast<TH2D*>(file_in.Get("h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral_GoodWidth1"));
    h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral_GoodWidth1->SetStats(0);
    h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral_GoodWidth1->SetTitleOffset(0.9, "Y");
    h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral_GoodWidth1->SetTitleSize(0.05, "Y");
    h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral_GoodWidth1->SetLabelSize(0.05, "Y");
    h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral_GoodWidth1->SetTitleSize(0.05, "X");
    h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral_GoodWidth1->SetLabelSize(0.05, "X");
    h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral_GoodWidth1->SetTitle("; U Cluster Pulse Integral; Cluster Start Time difference [25 ns]");
    h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral_GoodWidth1->GetXaxis()->SetNdivisions(404);
    h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral_GoodWidth1->Draw();
    c1->Print(Form("Figs/Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral_GoodWidth1_%d.pdf", run));
    c1->Print(Form("Figs/Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral_GoodWidth1_%d.png", run));
    c1->Print(Form("Figs/Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral_GoodWidth1_%d.root", run));

    auto h_U_lowADC_ClustStartTimeDifference_GoodWidth1 = h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral_GoodWidth1->ProjectionY("h_U_lowADC_ClustStartTimeDifference_GoodWidth1", 1, 50);
    h_U_lowADC_ClustStartTimeDifference_GoodWidth1->Draw();
    mean = h_U_lowADC_ClustStartTimeDifference_GoodWidth1->GetMean();
    rms = h_U_lowADC_ClustStartTimeDifference_GoodWidth1->GetRMS();
    f_Gaus->SetParameters(h_U_lowADC_ClustStartTimeDifference_GoodWidth1->GetMaximum(), mean, 0.5*rms);
    h_U_lowADC_ClustStartTimeDifference_GoodWidth1->Fit(f_Gaus, "MeV", "", mean - 0.95*rms, mean + 0.95*rms);
    mean = f_Gaus->GetParameter(1);
    sigma = f_Gaus->GetParameter(2);
    lat1->DrawLatex(0.15, 0.9, Form("#mu = %1.2f ns", mean*ts2ns));
    lat1->DrawLatex(0.15, 0.8, Form("#sigma = %1.2f ns", sigma*ts2ns));
    c1->Print(Form("Figs/U_lowADC_ClustStartTimeDifference_GoodWidth1_%d.pdf", run));
    c1->Print(Form("Figs/U_lowADC_ClustStartTimeDifference_GoodWidth1_%d.png", run));
    c1->Print(Form("Figs/U_lowADC_ClustStartTimeDifference_GoodWidth1_%d.root", run));

    nbinsX = h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral_GoodWidth1->GetNbinsX();
    auto h_U_highADC_ClustStartTimeDifference_GoodWidth1 = h_Cross_ClusterStartTimeDiff_vs_U_cl_PulseIntegral_GoodWidth1->ProjectionY("h_U_highADC_ClustStartTimeDifference_GoodWidth1",
        nbinsX-50, nbinsX);
    h_U_highADC_ClustStartTimeDifference_GoodWidth1->Draw();
    mean = h_U_highADC_ClustStartTimeDifference_GoodWidth1->GetMean();
    rms = h_U_highADC_ClustStartTimeDifference_GoodWidth1->GetRMS();
    f_Gaus->SetParameters(h_U_highADC_ClustStartTimeDifference_GoodWidth1->GetMaximum(), mean, 0.45*rms);
    h_U_highADC_ClustStartTimeDifference_GoodWidth1->Fit(f_Gaus, "MeV", "", mean - 0.55*rms, mean + 0.35*rms);
    mean = f_Gaus->GetParameter(1);
    sigma = f_Gaus->GetParameter(2);
    lat1->DrawLatex(0.15, 0.9, Form("#mu = %1.2f ns", mean*ts2ns));
    lat1->DrawLatex(0.15, 0.8, Form("#sigma = %1.2f ns", sigma*ts2ns));
    c1->Print(Form("Figs/U_highADC_ClustStartTimeDifference_GoodWidth1_%d.pdf", run));
    c1->Print(Form("Figs/U_highADC_ClustStartTimeDifference_GoodWidth1_%d.png", run));
    c1->Print(Form("Figs/U_highADC_ClustStartTimeDifference_GoodWidth1_%d.root", run));


}