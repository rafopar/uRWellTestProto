//
// Created by rafopar on 2/14/26.
//

#include <TCanvas.h>
#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TLatex.h>
#include <TGraph2D.h>
#include <XYHodoTools.h>

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
    h_uRwell_YXc_Eff1.Draw("colz");
    uRwellTools::DrawActiveArea();
    c1->Print(Form("Figs/uRwell_2DEff_WithHodoTags_%d.pdf", run));
    c1->Print(Form("Figs/uRwell_2DEff_WithHodoTags_%d.png", run));
    c1->Print(Form("Figs/uRwell_2DEff_WithHodoTags_%d.root", run));

    file_in.Close();
    return 0;
}