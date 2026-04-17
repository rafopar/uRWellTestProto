//
// Created by rafopar on 2/14/26.
//

#include <TCanvas.h>
#include <TFile.h>
#include <TF2.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TLatex.h>
#include <TGraph2D.h>
#include <XYHodoTools.h>
#include <TStyle.h>

#include  <uRwellTools.h>

#include <iostream>
using namespace std;

int main(int argc, const char *argv[]) {

    if (argc < 2) {
        cerr << "Usage: ./DrawEffWithHodo.exe  <RUN>" << endl;
    }

    auto c1 = new TCanvas("c1", "", 1800., 1000.);

    int run = atoi(argv[1]);

    TLatex lat1;
    lat1.SetNDC();
    lat1.SetTextFont(42);

    constexpr double tsWidth = 25; // Width of the APV25 time sample

    auto f_Gaus = new TF1("f_Gaus", "[0]*TMath::Gaus(x, [1], [2])", -5., 5.);
    f_Gaus->SetParLimits(1, -2., 2.);
    f_Gaus->SetParLimits(2, 0., 2.);
    f_Gaus->SetNpx(4500);

    TFile file_in(Form("AnaPulseFits_%d.root", run));

    auto h_Hodo_XY_BarID_Tag1 = dynamic_cast<TH2D*>(file_in.Get("h_Hodo_XY_BarID_Tag1"));

    h_Hodo_XY_BarID_Tag1->SetTitle("; Short Bar ID; Long Bar ID");

    h_Hodo_XY_BarID_Tag1->Draw();
    c1->Print(Form("Figs/Hodo_XY_BarID_Tag1_%d.pdf", run));
    c1->Print(Form("Figs/Hodo_XY_BarID_Tag1_%d.png", run));
    c1->Print(Form("Figs/Hodo_XY_BarID_Tag1_%d.root", run));

    TGraph2D gr_Hodo_Eff;
    TH2D h_uRwell_YXc1 ("h_uRwell_YXc1", "", 80, -902.5, 897.5, 44, -492.5, 497.5);
    h_uRwell_YXc1.SetTitle("; X [mm]; Y [mm]");
    h_uRwell_YXc1.SetStats(0);
    TH2D h_uRwell_YXc_Eff1 = *(dynamic_cast<TH2D*>(h_uRwell_YXc1.Clone("h_uRwell_YXc_Eff1")));

    TH2D h_Cross_YXc_GoodPulseSigma_CoarseBin1 = *(dynamic_cast<TH2D*>(file_in.Get("h_Cross_YXc_GoodPulseSigma_CoarseBin1")));
    h_Cross_YXc_GoodPulseSigma_CoarseBin1.SetTitle("; X [mm]; Y [mm]");
    TH2D h_uRwell_YXC1_Sgima_deltaT_UClPulseIntegral_Avg = *(dynamic_cast<TH2D*>(h_Cross_YXc_GoodPulseSigma_CoarseBin1.Clone("h_uRwell_YXC1_Sgima_deltaT_UClPulseIntegral_Avg")));
    TH2D h_uRwell_YXC1_Sgima_deltaT_UClPulseIntegral_High = *(dynamic_cast<TH2D*>(h_Cross_YXc_GoodPulseSigma_CoarseBin1.Clone("h_uRwell_YXC1_Sgima_deltaT_UClPulseIntegral_High")));
    TH2D h_uRwell_YXC1_Sgima_deltaT_UClPulseIntegral_Low = *(dynamic_cast<TH2D*>(h_Cross_YXc_GoodPulseSigma_CoarseBin1.Clone("h_uRwell_YXC1_Sgima_deltaT_UClPulseIntegral_Low")));
    TH2D h_uRwell_YXC1_Mean_deltaT_UClPulseIntegral_Avg = *(dynamic_cast<TH2D*>(h_Cross_YXc_GoodPulseSigma_CoarseBin1.Clone("h_uRwell_YXC1_Mean_deltaT_UClPulseIntegral_Avg")));
    TH2D h_uRwell_YXC1_Mean_deltaT_UClPulseIntegral_High = *(dynamic_cast<TH2D*>(h_Cross_YXc_GoodPulseSigma_CoarseBin1.Clone("h_uRwell_YXC1_Mean_deltaT_UClPulseIntegral_High")));
    TH2D h_uRwell_YXC1_Mean_deltaT_UClPulseIntegral_Low = *(dynamic_cast<TH2D*>(h_Cross_YXc_GoodPulseSigma_CoarseBin1.Clone("h_uRwell_YXC1_Mean_deltaT_UClPulseIntegral_Low")));

    const int n_uRWell_CoarsBinsX = 20;
    const int n_uRWell_CoarsBinsY = 10;
    c1->Print(Form("Figs/HighUPulse_DeltaTFits_%d.pdf[", run));
    c1->Print(Form("Figs/LowUPulse_DeltaTFits_%d.pdf[", run));
    c1->Print(Form("Figs/AvgUPulse_DeltaTFits_%d.pdf[", run));
    TH2D *h_Cross_Cl_St_Time_Diff_vs_U_cl_PulseInt_GoodWidth_ActiveArea1_[n_uRWell_CoarsBinsX][n_uRWell_CoarsBinsY];
    for ( int ixBin = 0; ixBin < n_uRWell_CoarsBinsX; ixBin++ ) {
        for (int iyBin = 0; iyBin < n_uRWell_CoarsBinsY; iyBin++ ) {
            h_Cross_Cl_St_Time_Diff_vs_U_cl_PulseInt_GoodWidth_ActiveArea1_[ixBin][iyBin] = dynamic_cast<TH2D*>(file_in.Get(Form("h_Cross_Cl_St_Time_Diff_vs_U_cl_PulseInt_GoodWidth_ActiveArea1_%d_%d", ixBin, iyBin)));
            h_Cross_Cl_St_Time_Diff_vs_U_cl_PulseInt_GoodWidth_ActiveArea1_[ixBin][iyBin]->SetTitle("; U Cluster Pulse Integral; #Delta t Start Time [25 ns]");

            int nbinsX = h_Cross_Cl_St_Time_Diff_vs_U_cl_PulseInt_GoodWidth_ActiveArea1_[ixBin][iyBin]->GetNbinsX();

            // double RMS_deltaT_UClPulseIntegral = tsWidth*h_dT_StartTimeAvg_UPulseIntegral->GetRMS();
            double RMS_deltaT_UClPulseIntegral = tsWidth*h_Cross_Cl_St_Time_Diff_vs_U_cl_PulseInt_GoodWidth_ActiveArea1_[ixBin][iyBin]->GetRMS(2);

            auto h_dT_StartTime_AvgUPusle = h_Cross_Cl_St_Time_Diff_vs_U_cl_PulseInt_GoodWidth_ActiveArea1_[ixBin][iyBin]->ProjectionY( Form("h_dT_StartTime_AvgUPusle_%d_%d", ixBin, iyBin), 1, nbinsX);

            h_uRwell_YXC1_Sgima_deltaT_UClPulseIntegral_Avg.SetBinContent(ixBin, iyBin, 0);
            h_uRwell_YXC1_Mean_deltaT_UClPulseIntegral_Avg.SetBinContent(ixBin, iyBin, 0);
            if ( h_dT_StartTime_AvgUPusle->Integral() > 150 ) {
                double RMS = h_dT_StartTime_AvgUPusle->GetRMS();
                double mean = h_dT_StartTime_AvgUPusle->GetBinCenter( h_dT_StartTime_AvgUPusle->GetMaximumBin() );
                double max = h_dT_StartTime_AvgUPusle->GetMaximum();

                f_Gaus->SetParameters(max, mean, 0.7*RMS);
                h_dT_StartTime_AvgUPusle->Fit(f_Gaus, "MeV", "", mean - 0.7*RMS, mean + 0.7*RMS);
                double sigma = tsWidth*f_Gaus->GetParameter(2);
                mean = tsWidth*f_Gaus->GetParameter(1);
                h_uRwell_YXC1_Sgima_deltaT_UClPulseIntegral_Avg.SetBinContent(ixBin, iyBin, sigma);
                h_uRwell_YXC1_Mean_deltaT_UClPulseIntegral_Avg.SetBinContent(ixBin, iyBin, mean);

                double x = h_uRwell_YXC1_Sgima_deltaT_UClPulseIntegral_Avg.GetXaxis()->GetBinCenter(ixBin);
                double y = h_uRwell_YXC1_Sgima_deltaT_UClPulseIntegral_Avg.GetYaxis()->GetBinCenter(iyBin);
                lat1.DrawLatex(0.12, 0.91, Form("X = %1.2f mm; Y = %1.2f mm", x, y));
                lat1.DrawLatex(0.65, 0.8, Form("#sigma = %1.2f ns", sigma));
                lat1.DrawLatex(0.65, 0.75, Form("#mu = %1.2f ns", mean));


                c1->Print(Form("Figs/AvgUPulse_DeltaTFits_%d.pdf", run));
            }

            auto h_dT_StartTime_HighUPusle = h_Cross_Cl_St_Time_Diff_vs_U_cl_PulseInt_GoodWidth_ActiveArea1_[ixBin][iyBin]->ProjectionY( Form("h_dT_StartTime_HighUPusle_%d_%d", ixBin, iyBin), nbinsX - 50, nbinsX);
            h_uRwell_YXC1_Sgima_deltaT_UClPulseIntegral_High.SetBinContent(ixBin, iyBin, 0);
            h_uRwell_YXC1_Mean_deltaT_UClPulseIntegral_High.SetBinContent(ixBin, iyBin, 0);
            if ( h_dT_StartTime_HighUPusle->Integral() > 150 ) {
                double RMS = h_dT_StartTime_HighUPusle->GetRMS();
                double mean = h_dT_StartTime_HighUPusle->GetBinCenter( h_dT_StartTime_HighUPusle->GetMaximumBin() );
                double max = h_dT_StartTime_HighUPusle->GetMaximum();

                f_Gaus->SetParameters(max, mean, 0.7*RMS);
                h_dT_StartTime_HighUPusle->Fit(f_Gaus, "MeV", "", mean - 0.7*RMS, mean + 0.7*RMS);
                double sigma = tsWidth*f_Gaus->GetParameter(2);
                mean = tsWidth*f_Gaus->GetParameter(1);
                h_uRwell_YXC1_Sgima_deltaT_UClPulseIntegral_High.SetBinContent(ixBin, iyBin, sigma);
                h_uRwell_YXC1_Mean_deltaT_UClPulseIntegral_High.SetBinContent(ixBin, iyBin, mean);

                double x = h_uRwell_YXC1_Sgima_deltaT_UClPulseIntegral_High.GetXaxis()->GetBinCenter(ixBin);
                double y = h_uRwell_YXC1_Sgima_deltaT_UClPulseIntegral_High.GetYaxis()->GetBinCenter(iyBin);
                lat1.DrawLatex(0.12, 0.91, Form("X = %1.2f mm; Y = %1.2f mm", x, y));
                lat1.DrawLatex(0.65, 0.8, Form("#sigma = %1.2f ns", sigma));
                lat1.DrawLatex(0.65, 0.75, Form("#mu = %1.2f ns", mean));

                c1->Print(Form("Figs/HighUPulse_DeltaTFits_%d.pdf", run));
            }

            auto h_dT_StartTime_LowUPusle = h_Cross_Cl_St_Time_Diff_vs_U_cl_PulseInt_GoodWidth_ActiveArea1_[ixBin][iyBin]->ProjectionY( Form("h_dT_StartTime_LowUPusle_%d_%d", ixBin, iyBin), 1, 50);
            h_uRwell_YXC1_Sgima_deltaT_UClPulseIntegral_Low.SetBinContent(ixBin, iyBin, 0);
            h_uRwell_YXC1_Mean_deltaT_UClPulseIntegral_Low.SetBinContent(ixBin, iyBin, 0);
            if ( h_dT_StartTime_LowUPusle->Integral() > 150 ) {
                double RMS = h_dT_StartTime_LowUPusle->GetRMS();
                double mean = h_dT_StartTime_LowUPusle->GetBinCenter( h_dT_StartTime_LowUPusle->GetMaximumBin() );
                double max = h_dT_StartTime_LowUPusle->GetMaximum();

                f_Gaus->SetParameters(max, mean, 0.7*RMS);
                h_dT_StartTime_LowUPusle->Fit(f_Gaus, "MeV", "", mean - 0.7*RMS, mean + 0.7*RMS);
                double sigma = tsWidth*f_Gaus->GetParameter(2);
                mean = tsWidth*f_Gaus->GetParameter(1);
                h_uRwell_YXC1_Sgima_deltaT_UClPulseIntegral_Low.SetBinContent(ixBin, iyBin, sigma);

                h_uRwell_YXC1_Mean_deltaT_UClPulseIntegral_Low.SetBinContent(ixBin, iyBin, mean);

                double x = h_uRwell_YXC1_Sgima_deltaT_UClPulseIntegral_Low.GetXaxis()->GetBinCenter(ixBin);
                double y = h_uRwell_YXC1_Sgima_deltaT_UClPulseIntegral_Low.GetYaxis()->GetBinCenter(iyBin);
                lat1.DrawLatex(0.12, 0.91, Form("X = %1.2f mm; Y = %1.2f mm", x, y));
                lat1.DrawLatex(0.65, 0.8, Form("#sigma = %1.2f ns", sigma));
                lat1.DrawLatex(0.65, 0.75, Form("#mu = %1.2f ns", mean));
                c1->Print(Form("Figs/LowUPulse_DeltaTFits_%d.pdf", run));
            }


        }
    }
    c1->Print(Form("Figs/HighUPulse_DeltaTFits_%d.pdf]", run));
    c1->Print(Form("Figs/AvgUPulse_DeltaTFits_%d.pdf]", run));
    c1->Print(Form("Figs/LowUPulse_DeltaTFits_%d.pdf]", run));

    TH2D *h_Cross_YXC_Max1_[XYHodoTools::nShortBars][XYHodoTools::nLongBars];

    c1->Print(Form("Figs/uRwell_Cross_WithHodoTags_%d.pdf[", run));

    for ( auto ishortBar = 0; ishortBar < XYHodoTools::nShortBars; ishortBar++ ) {
        for (auto ilongBar = 0; ilongBar < XYHodoTools::nLongBars; ilongBar++ ) {
            h_Cross_YXC_Max1_[ishortBar][ilongBar] = dynamic_cast<TH2D*>(file_in.Get(Form("h_Cross_YXC_Max1_%d_%d", ishortBar, ilongBar)));
            h_Cross_YXC_Max1_[ishortBar][ilongBar]->SetTitle("; Cross X [mm]; Cross Y [mm]");
            h_Cross_YXC_Max1_[ishortBar][ilongBar]->Draw();
            lat1.DrawLatex(0.12, 0.91, Form("Hodo pixel (%d, %d)", ishortBar, ilongBar));
            c1->Print(Form("Figs/uRwell_Cross_WithHodoTags_%d.pdf", run));

            double x_avg = h_Cross_YXC_Max1_[ishortBar][ilongBar]->GetMean(1);
            double y_avg = h_Cross_YXC_Max1_[ishortBar][ilongBar]->GetMean(2);

            int hodo_binY = h_Hodo_XY_BarID_Tag1->GetYaxis()->FindBin(ilongBar);
            int hodo_binX = h_Hodo_XY_BarID_Tag1->GetXaxis()->FindBin(ishortBar);

            double NHodoHits = h_Hodo_XY_BarID_Tag1->GetBinContent(hodo_binX, hodo_binY);
            double nuRwellHits = h_Cross_YXC_Max1_[ishortBar][ilongBar]->Integral();
            double eff = nuRwellHits/NHodoHits;



            if ( !XYHodoTools::IsPixelOnuRwell(ishortBar, ilongBar) ) {
                continue;
            }

            if (nuRwellHits > 100) {
                gr_Hodo_Eff.AddPoint(x_avg, y_avg, eff);
                h_uRwell_YXc1.Fill(x_avg, y_avg);
                h_uRwell_YXc_Eff1.Fill(x_avg, y_avg, eff);
            }
        }
    }
    c1->Print(Form("Figs/uRwell_Cross_WithHodoTags_%d.pdf]", run));

    c1->Clear();
    gr_Hodo_Eff.Draw("pcol");
    c1->Print(Form("Figs/Hodo_2DEff_%d.pdf", run));
    c1->Print(Form("Figs/Hodo_2DEff_%d.png", run));
    c1->Print(Form("Figs/Hodo_2DEff_%d.root", run));

    c1->Clear();
    h_uRwell_YXc_Eff1.Divide(&h_uRwell_YXc1);
    h_uRwell_YXc_Eff1.SetMaximum(1.);
    h_uRwell_YXc_Eff1.Draw("colz");
    uRwellTools::DrawActiveArea();
    c1->Print(Form("Figs/uRwell_2DEff_WithHodoTags_%d.pdf", run));
    c1->Print(Form("Figs/uRwell_2DEff_WithHodoTags_%d.png", run));
    c1->Print(Form("Figs/uRwell_2DEff_WithHodoTags_%d.root", run));

    h_uRwell_YXC1_Sgima_deltaT_UClPulseIntegral_Avg.SetStats(0);
    h_uRwell_YXC1_Sgima_deltaT_UClPulseIntegral_Avg.Draw("colz");
    uRwellTools::DrawActiveArea();
    c1->Print(Form("Figs/cl_StartTime_DeltaT_UClPulseIntegral_Avg_%d.pdf", run));
    c1->Print(Form("Figs/cl_StartTime_DeltaT_UClPulseIntegral_Avg_%d.png", run));
    c1->Print(Form("Figs/cl_StartTime_DeltaT_UClPulseIntegral_Avg_%d.root", run));

    h_uRwell_YXC1_Sgima_deltaT_UClPulseIntegral_High.SetStats(0);
    h_uRwell_YXC1_Sgima_deltaT_UClPulseIntegral_High.Draw("colz");
    uRwellTools::DrawActiveArea();
    c1->Print(Form("Figs/cl_StartTime_DeltaT_UClPulseIntegral_High_%d.pdf", run));
    c1->Print(Form("Figs/cl_StartTime_DeltaT_UClPulseIntegral_High_%d.png", run));
    c1->Print(Form("Figs/cl_StartTime_DeltaT_UClPulseIntegral_High_%d.root", run));

    h_uRwell_YXC1_Sgima_deltaT_UClPulseIntegral_Low.SetStats(0);
    h_uRwell_YXC1_Sgima_deltaT_UClPulseIntegral_Low.Draw("colz");
    uRwellTools::DrawActiveArea();
    c1->Print(Form("Figs/cl_StartTime_DeltaT_UClPulseIntegral_Low_%d.pdf", run));
    c1->Print(Form("Figs/cl_StartTime_DeltaT_UClPulseIntegral_Low_%d.png", run));
    c1->Print(Form("Figs/cl_StartTime_DeltaT_UClPulseIntegral_Low_%d.root", run));

    gStyle->SetPalette(kBird); // or kViridis, kRainBow, kSunset, etc.
    gStyle->SetNumberContours(99);

    h_uRwell_YXC1_Mean_deltaT_UClPulseIntegral_Avg.SetStats(0);
    h_uRwell_YXC1_Mean_deltaT_UClPulseIntegral_Avg.Draw("colz");
    uRwellTools::DrawActiveArea();
    c1->Print(Form("Figs/cl_StartTime_DeltaT_Mean_UClPulseIntegral_Avg_%d.pdf", run));
    c1->Print(Form("Figs/cl_StartTime_DeltaT_Mean_UClPulseIntegral_Avg_%d.png", run));
    c1->Print(Form("Figs/cl_StartTime_DeltaT_Mean_UClPulseIntegral_Avg_%d.root", run));

    h_uRwell_YXC1_Mean_deltaT_UClPulseIntegral_High.SetStats(0);
    h_uRwell_YXC1_Mean_deltaT_UClPulseIntegral_High.Draw("colz");
    uRwellTools::DrawActiveArea();
    c1->Print(Form("Figs/cl_StartTime_DeltaT_Mean_UClPulseIntegral_High_%d.pdf", run));
    c1->Print(Form("Figs/cl_StartTime_DeltaT_Mean_UClPulseIntegral_High_%d.png", run));
    c1->Print(Form("Figs/cl_StartTime_DeltaT_Mean_UClPulseIntegral_High_%d.root", run));

    h_uRwell_YXC1_Mean_deltaT_UClPulseIntegral_Low.SetStats(0);
    h_uRwell_YXC1_Mean_deltaT_UClPulseIntegral_Low.Draw("colz");
    uRwellTools::DrawActiveArea();
    c1->Print(Form("Figs/cl_StartTime_DeltaT_Mean_UClPulseIntegral_Low_%d.pdf", run));
    c1->Print(Form("Figs/cl_StartTime_DeltaT_Mean_UClPulseIntegral_Low_%d.png", run));
    c1->Print(Form("Figs/cl_StartTime_DeltaT_Mean_UClPulseIntegral_Low_%d.root", run));


    TF2 f_UVStrROLengthDiff("f_UVStrROLengthDiff", [](double *x, double *p){ return (1./300.)*(uRwellTools::getROLength_U(x[0], x[1]) - uRwellTools::getROLength_V(x[0], x[1])); }, -900, 900, -500, 500);
    f_UVStrROLengthDiff.SetTitle("; Hit X coordinate [mm]; Hit Y coordinate");
    f_UVStrROLengthDiff.SetNpy(800);
    f_UVStrROLengthDiff.SetNpx(800);
    c1->Clear();
    f_UVStrROLengthDiff.Draw("colz");
    uRwellTools::DrawActiveArea();
    c1->Print("Figs/Str_ROLength_Diff.pdf");
    c1->Print("Figs/Str_ROLength_Diff.png");
    c1->Print("Figs/Str_ROLength_Diff.root");

    file_in.Close();
    return 0;
}