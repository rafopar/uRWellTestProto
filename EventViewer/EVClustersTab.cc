//
// EVClustersTab.cc
//

#include "EVClustersTab.h"

#include <TCanvas.h>
#include <TGButton.h>
#include <TH2D.h>
#include <TLatex.h>
#include <TLine.h>
#include <TMarker.h>
#include <TRootEmbeddedCanvas.h>
#include <TString.h>
#include <WidgetMessageTypes.h>

#include <uRwellTools.h>

#include "EVGeometry.h"

EVClustersTab::EVClustersTab(const TGWindow *p) : EVTab(p) {

    auto *controlFrame = new TGHorizontalFrame(this);
    fOnlyMaxADCCheck = new TGCheckButton(controlFrame, "Only Max ADC clusters", kChkOnlyMaxADC);
    fOnlyMaxADCCheck->Associate(this);
    controlFrame->AddFrame(fOnlyMaxADCCheck, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 2, 2));
    AddFrame(controlFrame, new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));

    fCanvas = new TRootEmbeddedCanvas("ClustersCanvas", this, 800, 600);
    AddFrame(fCanvas, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2));
}

Bool_t EVClustersTab::ProcessMessage(Longptr_t msg, Longptr_t parm1, Longptr_t) {
    if (GET_MSG(msg) == kC_COMMAND && GET_SUBMSG(msg) == kCM_CHECKBUTTON && parm1 == kChkOnlyMaxADC) {
        Redraw();
        return kTRUE;
    }
    return TGCompositeFrame::ProcessMessage(msg, parm1, 0);
}

void EVClustersTab::Redraw() {

    TCanvas *c = fCanvas->GetCanvas();
    c->Clear();
    c->cd();
    fDrawObjects.clear();

    auto hFrame = std::make_unique<TH2D>("h_frame_Clusters", "; X [mm]; Y [mm]",
                                         10, -800., 800., 10, -320., 320.);
    hFrame->SetStats(0);
    hFrame->Draw();
    fDrawObjects.push_back(std::move(hFrame));

    uRwellTools::DrawActiveArea();

    std::vector<uRwellTools::PulseCluster> v_U_Clusters;
    std::vector<uRwellTools::PulseCluster> v_V_Clusters;
    if (fEvent != nullptr) {
        if (!fEvent->v_U_Pulses.empty()) {
            v_U_Clusters = uRwellTools::getPulseClusters(fEvent->v_U_Pulses);
        }
        if (!fEvent->v_V_Pulses.empty()) {
            v_V_Clusters = uRwellTools::getPulseClusters(fEvent->v_V_Pulses);
        }
    }

    // When the filter is on, keep in each layer only the cluster with the
    // highest total pulse integral.
    if (fOnlyMaxADCCheck->IsOn()) {
        auto keepMaxIntegral = [](std::vector<uRwellTools::PulseCluster> &clusters) {
            if (clusters.size() < 2) {
                return;
            }
            size_t indMax = 0;
            for (size_t i = 1; i < clusters.size(); i++) {
                if (clusters[i].getClusterPulseIntegral() > clusters[indMax].getClusterPulseIntegral()) {
                    indMax = i;
                }
            }
            clusters = {clusters[indMax]};
        };
        keepMaxIntegral(v_U_Clusters);
        keepMaxIntegral(v_V_Clusters);
    }

    auto drawClusterLine = [this](double strip, bool isU) {
        double a, b;
        if (isU) {
            EVGeo::GetUStripLine(strip, a, b);
        } else {
            EVGeo::GetVStripLine(strip, a, b);
        }
        double x1, y1, x2, y2;
        if (!EVGeo::ClipLineToActiveArea(a, b, x1, y1, x2, y2)) {
            return;
        }
        auto line = std::make_unique<TLine>(x1, y1, x2, y2);
        line->SetLineColor(isU ? kRed : kBlue);
        line->SetLineWidth(2);
        line->Draw();
        fDrawObjects.push_back(std::move(line));
    };

    for (auto &cl : v_U_Clusters) {
        drawClusterLine(cl.getClusterCenter(), true);
    }
    for (auto &cl : v_V_Clusters) {
        drawClusterLine(cl.getClusterCenter(), false);
    }

    // Mark the UxV intersections of the displayed clusters
    for (auto &clU : v_U_Clusters) {
        for (auto &clV : v_V_Clusters) {
            double x = uRwellTools::getCrossX(clU.getClusterCenter(), clV.getClusterCenter());
            double y = uRwellTools::getCrossY(clU.getClusterCenter(), clV.getClusterCenter());
            auto marker = std::make_unique<TMarker>(x, y, 29);
            marker->SetMarkerSize(2.);
            marker->SetMarkerColor(kBlack);
            marker->Draw();
            fDrawObjects.push_back(std::move(marker));
        }
    }

    auto lat = std::make_unique<TLatex>();
    lat->SetNDC();
    lat->SetTextFont(42);
    lat->SetTextSize(0.04);
    lat->DrawLatex(0.12, 0.92, Form("#color[2]{U clusters: %d}   #color[4]{V clusters: %d}",
                                    int(v_U_Clusters.size()), int(v_V_Clusters.size())));
    fDrawObjects.push_back(std::move(lat));

    c->Update();
}
