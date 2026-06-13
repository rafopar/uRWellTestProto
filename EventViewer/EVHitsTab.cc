//
// EVHitsTab.cc
//

#include "EVHitsTab.h"

#include <TCanvas.h>
#include <TH2D.h>
#include <TLatex.h>
#include <TLine.h>
#include <TRootEmbeddedCanvas.h>
#include <TString.h>

#include <uRwellTools.h>

#include "EVGeometry.h"

EVHitsTab::EVHitsTab(const TGWindow *p) : EVTab(p) {
    fCanvas = new TRootEmbeddedCanvas("HitsCanvas", this, 800, 600);
    AddFrame(fCanvas, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2));
}

void EVHitsTab::Redraw() {

    TCanvas *c = fCanvas->GetCanvas();
    c->Clear();
    c->cd();
    fDrawObjects.clear();

    auto hFrame = std::make_unique<TH2D>("h_frame_Hits", "; X [mm]; Y [mm]",
                                         10, -800., 800., 10, -320., 320.);
    hFrame->SetStats(0);
    hFrame->Draw();
    fDrawObjects.push_back(std::move(hFrame));

    uRwellTools::DrawActiveArea();

    int nU = 0;
    int nV = 0;
    if (fEvent != nullptr) {
        for (const auto &p : fEvent->v_U_Pulses) {
            double a, b;
            EVGeo::GetUStripLine(p.hit.strip, a, b);
            double x1, y1, x2, y2;
            if (!EVGeo::ClipLineToActiveArea(a, b, x1, y1, x2, y2)) {
                continue;
            }
            auto line = std::make_unique<TLine>(x1, y1, x2, y2);
            line->SetLineColor(kRed);
            line->SetLineWidth(2);
            line->Draw();
            fDrawObjects.push_back(std::move(line));
            nU++;
        }
        for (const auto &p : fEvent->v_V_Pulses) {
            double a, b;
            EVGeo::GetVStripLine(p.hit.strip, a, b);
            double x1, y1, x2, y2;
            if (!EVGeo::ClipLineToActiveArea(a, b, x1, y1, x2, y2)) {
                continue;
            }
            auto line = std::make_unique<TLine>(x1, y1, x2, y2);
            line->SetLineColor(kBlue);
            line->SetLineWidth(2);
            line->Draw();
            fDrawObjects.push_back(std::move(line));
            nV++;
        }
    }

    auto lat = std::make_unique<TLatex>();
    lat->SetNDC();
    lat->SetTextFont(42);
    lat->SetTextSize(0.04);
    lat->DrawLatex(0.12, 0.92, Form("#color[2]{U hits: %d}   #color[4]{V hits: %d}", nU, nV));
    fDrawObjects.push_back(std::move(lat));

    c->Update();
}
