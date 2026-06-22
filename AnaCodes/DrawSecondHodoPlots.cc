//
// DrawSecondHodoPlots.cc
//
// Reads the merged analysis ROOT file produced for the second prototype
// double-hodoscope analysis (AnaSecondHodoDoubleHodo_<RUN>.root) and computes
// the uRWell efficiency for each hodoscope pixel (i, j).
//
// For a given pixel (i, j) the efficiency is defined as the ratio of:
//   * the integral of h_uRwell_Cross_YXc1_<i>_<j> over an (i, j)-dependent
//     spatial window, and
//   * the number of vertical-track tags in that pixel, given by
//     h_Det0_Occupancy_vertTrk1->GetBinContent(i + 1, j + 1).
//
// The window for pixel (i, j) is:
//   x_min / x_max = (-850 + i*45) / (-650 + i*45)
//   y_min / y_max = (-325 + j*45) / (-150 + j*45)
//
// The result is stored in a 2D efficiency histogram that spans the same range
// as the h_uRwell_Cross_YXc1_<i>_<j> histograms (X: -900..900, Y: -500..500)
// but with 99 (X) and 36 (Y) bins.
//
// This program is meant to be run from the directory that contains the input
// ROOT file, e.g. /u/home/rafopar/work/builds/TestProto/bin
//

#include <TF1.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TH2D.h>
#include <TLatex.h>
#include <TStyle.h>

#include <XYHodoTools.h>

#include <iostream>
using namespace std;

int main(int argc, const char *argv[]) {

    if (argc < 2) {
        cerr << "Usage: ./DrawSecondHodoPlots.exe  <RUN>" << endl;
        return 1;
    }

    const int run = atoi(argv[1]);

    static constexpr double ts2ns = 25.;
    auto c1 = new TCanvas("c1", "", 1800., 1000.);

    TLatex lat1;
    lat1.SetNDC();
    lat1.SetTextFont(42);

    TF1 *f_Gaus = new TF1("f_Gaus", "[0]*TMath::Gaus(x, [1], [2])", -25., 25);
    f_Gaus->SetNpx(4500);
    f_Gaus->SetParLimits(2, 0., 10.);

    TFile file_in(Form("AnaSecondHodoDoubleHodo_%d.root", run));
    if (file_in.IsZombie()) {
        cerr << "Could not open input file AnaSecondHodoDoubleHodo_" << run << ".root" << endl;
        return 1;
    }

    auto h_Det0_Occupancy_vertTrk1 = dynamic_cast<TH2D*>(file_in.Get("h_Det0_Occupancy_vertTrk1"));
    if (h_Det0_Occupancy_vertTrk1 == nullptr) {
        cerr << "Could not find the histogram h_Det0_Occupancy_vertTrk1 in the input file" << endl;
        return 1;
    }

    // Efficiency histogram: same range as the h_uRwell_Cross_YXc1_<i>_<j>
    // histograms (X: -900..900, Y: -500..500) but coarser binning (99 x 36).
    TH2D h_SecondProt_2DEff("h_SecondProt_2DEff", "; Cross X [mm]; Cross Y [mm]",
                            200, -900., 900., 72, -500., 500.);
    h_SecondProt_2DEff.SetStats(0);

    TH2D h_pixel_deltaT_mean("h_pixel_deltaT_mean", "", 150, -900., 900, 50, -500., 500.);
    h_pixel_deltaT_mean.SetTitle("; uRwell X [mm]; uRwell Y [mm] ");
    TH2D h_pixel_deltaT_sigm("h_pixel_deltaT_sigm", "", 150, -900., 900, 50, -500., 500.);
    h_pixel_deltaT_sigm.SetTitle("; uRwell X [mm]; uRwell Y [mm] ");

    auto *c_2d_dtFit = new TCanvas("c_2d_dtFit", "", 1800, 1000);
    c_2d_dtFit->Print(Form("Figs/2d_dtMean_Run_%d.pdf[", run));

    for (int i = 0; i < XYHodoTools::nShortBars; i++) {
        for (int j = 0; j < XYHodoTools::nLongBars; j++) {

            auto h_uRwell_Cross_YXc1 = dynamic_cast<TH2D*>(file_in.Get(Form("h_uRwell_Cross_YXc1_%d_%d", i, j)));
            if (h_uRwell_Cross_YXc1 == nullptr) {
                continue;
            }

            // (i, j)-dependent integration window.
            const double x_min = -850. + i * 45.;
            const double x_max = -650. + i * 45.;
            const double y_min = -325. + j * 45.;
            const double y_max = -150. + j * 45.;

            const int binx_min = h_uRwell_Cross_YXc1->GetXaxis()->FindBin(x_min);
            const int binx_max = h_uRwell_Cross_YXc1->GetXaxis()->FindBin(x_max);
            const int biny_min = h_uRwell_Cross_YXc1->GetYaxis()->FindBin(y_min);
            const int biny_max = h_uRwell_Cross_YXc1->GetYaxis()->FindBin(y_max);

            const double n_uRwell = h_uRwell_Cross_YXc1->Integral(binx_min, binx_max, biny_min, biny_max);
            const double n_tags = h_Det0_Occupancy_vertTrk1->GetBinContent(i + 1, j + 1);

            if (n_tags <= 0) {
                continue;
            }

            const double eff = n_uRwell / n_tags;

            // Place the efficiency at the centre of the (i, j) window.
            const double xc = 0.5 * (x_min + x_max);
            const double yc = 0.5 * (y_min + y_max);
            h_SecondProt_2DEff.Fill(xc, yc, eff);

            auto h_DeltaStartTime_UV1 = dynamic_cast<TH1D*>(file_in.Get(Form("h_DeltaStartTime_UV1_%d_%d", i, j)));

            if ( h_DeltaStartTime_UV1->GetEntries() > 80 ) {
                double mean = h_DeltaStartTime_UV1->GetMean();
                double rms = h_DeltaStartTime_UV1->GetRMS();
                f_Gaus->SetParameters( h_DeltaStartTime_UV1->GetMaximum(), mean, rms );
                h_DeltaStartTime_UV1->Fit(f_Gaus, "Me", "", mean - 2.5*rms, mean + 2.5*rms);
                c_2d_dtFit->Print(Form("Figs/2d_dtMean_Run_%d.pdf", run));

                mean = f_Gaus->GetParameter(1)*ts2ns;
                double sigm = f_Gaus->GetParameter(2)*ts2ns;

                h_pixel_deltaT_mean.Fill( xc, yc, mean);
                h_pixel_deltaT_sigm.Fill( xc, yc, sigm);
            }

        }
    }

    c_2d_dtFit->Print(Form("Figs/2d_dtMean_Run_%d.pdf]", run));

    gStyle->SetPalette(kBird);
    gStyle->SetNumberContours(99);
    gStyle->SetOptStat(0);

    c1->Clear();
    c1->cd();
    h_SecondProt_2DEff.SetMaximum(1.);
    h_SecondProt_2DEff.Draw("colz");
    lat1.DrawLatex(0.12, 0.91, Form("Run %d", run));

    c1->Print(Form("Figs/SecondProt_2DEff_%d.pdf", run));
    c1->Print(Form("Figs/SecondProt_2DEff_%d.png", run));
    c1->Print(Form("Figs/SecondProt_2DEff_%d.root", run));

    h_pixel_deltaT_mean.SetMaximum(25.);
    h_pixel_deltaT_mean.SetMinimum(-25.);
    h_pixel_deltaT_mean.Draw("colz");
    lat1.DrawLatex(0.12, 0.91, Form("Run %d #Delta t mean", run));
    c1->Print(Form("Figs/SecondProt_2D_dEltaT_mean_%d.pdf", run));
    c1->Print(Form("Figs/SecondProt_2D_dEltaT_mean_%d.png", run));
    c1->Print(Form("Figs/SecondProt_2D_dEltaT_mean_%d.root", run));

    h_pixel_deltaT_sigm.SetMaximum(45);
    h_pixel_deltaT_sigm.Draw("colz");
    lat1.DrawLatex(0.12, 0.91, Form("Run %d #Delta t #sigma", run));
    c1->Print(Form("Figs/SecondProt_2D_dEltaT_sigm_%d.pdf", run));
    c1->Print(Form("Figs/SecondProt_2D_dEltaT_sigm_%d.png", run));
    c1->Print(Form("Figs/SecondProt_2D_dEltaT_sigm_%d.root", run));

    file_in.Close();
    return 0;
}
