//
// Created by rafopar on 1/5/26.
//

#include <fstream>
#include <iostream>
#include <ostream>
#include <TCanvas.h>

#include <TF1.h>
#include <TH2D.h>
#include <TH1D.h>
#include <TMath.h>
#include <TFile.h>

using namespace std;

TH1D SliceFit(TH2D*);

int main( int argc, char* argv[] ) {

    if ( argc != 2 ) {
        cerr << "Usage: UpdateStripSigmas.exe <RUN> " << endl;
        exit( EXIT_FAILURE );
    }

    int run = atoi(argv[1]);

    auto c1 = new TCanvas("c1", "", 1200, 900);

    auto file_in = TFile(Form("AnaPulseFits_%d.root", run));

    auto h_Cross_UCluster_Sgima_vs_Strip1 = dynamic_cast<TH2D*>(file_in.Get("h_Cross_UCluster_Sgima_vs_Strip1"));
    h_Cross_UCluster_Sgima_vs_Strip1->SetTitle("; U Strip; Pulse #sigma [25 ns]");
    auto h_Cross_VCluster_Sgima_vs_Strip1 = dynamic_cast<TH2D*>(file_in.Get("h_Cross_VCluster_Sgima_vs_Strip1"));
    h_Cross_VCluster_Sgima_vs_Strip1->SetTitle("; V Strip; Pulse #sigma [25 ns]");

    ofstream out_dat(Form("Pars/Pulse_Sigmas_%d.dat", run));


    auto h_Sliced_U_Sigmas = SliceFit(h_Cross_UCluster_Sgima_vs_Strip1);
    h_Sliced_U_Sigmas.SetName("h_Sliced_U_Sigmas");
    h_Sliced_U_Sigmas.SetMarkerColor(2);
    h_Cross_UCluster_Sgima_vs_Strip1->Draw();
    h_Sliced_U_Sigmas.Draw("Same p");
    c1->Print(Form("Figs/Sliced_U_Sigmas_%d.pdf", run));
    c1->Print(Form("Figs/Sliced_U_Sigmas_%d.png", run));
    c1->Print(Form("Figs/Sliced_U_Sigmas_%d.root", run));


    for ( int ib = 1; ib <= h_Sliced_U_Sigmas.GetNbinsX(); ib++ ) {
        out_dat<<ib<<"\t"<<h_Sliced_U_Sigmas.GetBinContent(ib)<<endl;
    }


    auto h_Sliced_V_Sigmas = SliceFit(h_Cross_VCluster_Sgima_vs_Strip1);
    h_Sliced_V_Sigmas.SetName("h_Sliced_V_Sigmas");
    h_Sliced_V_Sigmas.SetMarkerColor(2);
    h_Cross_VCluster_Sgima_vs_Strip1->Draw();
    h_Sliced_V_Sigmas.Draw("Same p");
    c1->Print(Form("Figs/Sliced_V_Sigmas_%d.pdf", run));
    c1->Print(Form("Figs/Sliced_V_Sigmas_%d.png", run));
    c1->Print(Form("Figs/Sliced_V_Sigmas_%d.root", run));
    for ( int ib = 1; ib <= h_Sliced_V_Sigmas.GetNbinsX(); ib++ ) {
        out_dat<<1000+ib<<"\t"<<h_Sliced_V_Sigmas.GetBinContent(ib)<<endl;
    }


    return 0;
}

TH1D SliceFit(TH2D* h) {
    double xMin = h->GetXaxis()->GetXmin();
    double xMax = h->GetXaxis()->GetXmax();
    int nBins = h->GetNbinsX();

    const double yMin = 1.2;
    const double yMax = 3.5;
    int yMinBin = h->GetYaxis()->FindBin(yMin);
    int yMaxBin = h->GetYaxis()->FindBin(yMax);

    auto f_Gaus = new TF1("f_Gays", "[0]*TMath::Gaus(x, [1], [2])", 0., 9);
    f_Gaus->SetNpx(4500);
    f_Gaus->SetParLimits(1, yMin, yMax);

    TH1D h_Sliced_Sigma("h_Sliced_Sigma", "", nBins, xMin, xMax);

    for (int iBin = 1; iBin <= nBins; iBin++) {
        auto h_tmp = h->ProjectionY(Form("h_Proj_%d", iBin), iBin, iBin);

        h_tmp->SetAxisRange(yMin, yMax);
        double mean = h_tmp->GetBinCenter( h_tmp->GetMaximumBin() );
        double sigma = h_tmp->GetRMS();
        double max = h_tmp->GetMaximum();
        f_Gaus->SetParameters(max, mean, sigma);
        h_tmp->Fit(f_Gaus, "MeV", "", yMin, yMax);
        mean = f_Gaus->GetParameter(1);
        h_Sliced_Sigma.SetBinContent(iBin, mean);
    }

    return h_Sliced_Sigma;
}