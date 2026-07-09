//
// EVRawDataTab.cc
//

#include "EVRawDataTab.h"

#include <algorithm>
#include <cmath>

#include <TCanvas.h>
#include <TGraph.h>
#include <TLatex.h>
#include <TLine.h>
#include <TPad.h>
#include <TRootEmbeddedCanvas.h>
#include <TString.h>

EVRawDataTab::EVRawDataTab(const TGWindow *p) : EVTab(p) {
    fCanvas = new TRootEmbeddedCanvas("RawDataCanvas", this, 800, 600);
    AddFrame(fCanvas, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2));
}

void EVRawDataTab::Redraw() {

    TCanvas *c = fCanvas->GetCanvas();
    c->Clear();
    fDrawObjects.clear();

    auto message = [&](const char *txt) {
        c->cd();
        auto lat = std::make_unique<TLatex>(0.5, 0.5, txt);
        lat->SetNDC();
        lat->SetTextAlign(22);
        lat->Draw();
        fDrawObjects.push_back(std::move(lat));
        c->Update();
    };

    if (fEvent == nullptr || !fEvent->isEvio) {
        message("Raw ADC data is only available for EVIO input");
        return;
    }

    const std::vector<EVEvent::RawSRSHybrid> &hybrids = fEvent->rawSRS;
    if (hybrids.empty()) {
        message("No SRS raw data in this event");
        return;
    }

    // One pad per hybrid, in a roughly square grid.
    const int n = int(hybrids.size());
    const int nCols = std::max(1, int(std::ceil(std::sqrt(double(n)))));
    const int nRows = (n + nCols - 1) / nCols;
    c->Divide(nCols, nRows);

    for (int i = 0; i < n; i++) {
        c->cd(i + 1);
        gPad->SetLeftMargin(0.12);
        gPad->SetRightMargin(0.02);

        const EVEvent::RawSRSHybrid &H = hybrids[i];

        auto gr = std::make_unique<TGraph>();
        double yMin = 1e30;
        double yMax = -1e30;
        for (size_t w = 0; w < H.words.size(); w++) {
            const double y = H.words[w];
            gr->SetPoint(int(w), double(w), y);
            yMin = std::min(yMin, y);
            yMax = std::max(yMax, y);
        }
        if (H.words.empty()) {
            yMin = 0.;
            yMax = 1.;
        }

        double span = yMax - yMin;
        if (span <= 0.) {
            span = 1.;
        }
        gr->SetTitle(Form("Hybrid (slot) %d;word number in bank;ADC value", H.slot));
        gr->SetLineColor(kBlue + 1);
        gr->SetLineWidth(1);
        gr->SetMinimum(yMin - 0.05 * span);
        gr->SetMaximum(yMax + 0.10 * span);
        gr->Draw("AL");

        // Red common-mode line spanning each time sample's word range.
        for (const EVEvent::RawSRSFrame &fr : H.frames) {
            auto line = std::make_unique<TLine>(double(fr.wordLo), fr.commonMode,
                                                double(fr.wordHi), fr.commonMode);
            line->SetLineColor(kRed);
            line->SetLineWidth(2);
            line->Draw();
            fDrawObjects.push_back(std::move(line));
        }

        fDrawObjects.push_back(std::move(gr));
    }

    c->cd();
    c->Update();
}
