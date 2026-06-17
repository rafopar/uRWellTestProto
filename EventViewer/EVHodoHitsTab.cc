//
// EVHodoHitsTab.cc
//

#include "EVHodoHitsTab.h"

#include <TCanvas.h>
#include <TGButton.h>
#include <TGLabel.h>
#include <TGNumberEntry.h>
#include <TH2D.h>
#include <TLatex.h>
#include <TLine.h>
#include <TPad.h>
#include <TRootEmbeddedCanvas.h>
#include <TString.h>
#include <WidgetMessageTypes.h>

#include <XYHodoAnalyzer.h>

EVHodoHitsTab::EVHodoHitsTab(const TGWindow *p) : EVTab(p) {

    auto *controlFrame = new TGHorizontalFrame(this);

    fChkTOT = new TGCheckButton(controlFrame, "T over thr. >", kChkTOT);
    fChkTOT->Associate(this);
    controlFrame->AddFrame(fChkTOT, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 2, 2, 2));
    fNumTOT = new TGNumberEntry(controlFrame, 100, 6, kNumTOT, TGNumberFormat::kNESInteger,
                                TGNumberFormat::kNEAAnyNumber, TGNumberFormat::kNELNoLimits);
    fNumTOT->Associate(this);
    controlFrame->AddFrame(fNumTOT, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 20, 2, 2));

    fChkRise = new TGCheckButton(controlFrame, "Rise TDC in", kChkRise);
    fChkRise->Associate(this);
    controlFrame->AddFrame(fChkRise, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 2, 2, 2));
    fNumRiseMin = new TGNumberEntry(controlFrame, 0, 7, kNumRiseMin, TGNumberFormat::kNESInteger,
                                    TGNumberFormat::kNEAAnyNumber, TGNumberFormat::kNELNoLimits);
    fNumRiseMin->Associate(this);
    controlFrame->AddFrame(fNumRiseMin, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 2, 2, 2));
    fNumRiseMax = new TGNumberEntry(controlFrame, 50000, 7, kNumRiseMax, TGNumberFormat::kNESInteger,
                                    TGNumberFormat::kNEAAnyNumber, TGNumberFormat::kNELNoLimits);
    fNumRiseMax->Associate(this);
    controlFrame->AddFrame(fNumRiseMax, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 20, 2, 2));

    fChkFall = new TGCheckButton(controlFrame, "Fall TDC in", kChkFall);
    fChkFall->Associate(this);
    controlFrame->AddFrame(fChkFall, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 2, 2, 2));
    fNumFallMin = new TGNumberEntry(controlFrame, 0, 7, kNumFallMin, TGNumberFormat::kNESInteger,
                                    TGNumberFormat::kNEAAnyNumber, TGNumberFormat::kNELNoLimits);
    fNumFallMin->Associate(this);
    controlFrame->AddFrame(fNumFallMin, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 2, 2, 2));
    fNumFallMax = new TGNumberEntry(controlFrame, 50000, 7, kNumFallMax, TGNumberFormat::kNESInteger,
                                    TGNumberFormat::kNEAAnyNumber, TGNumberFormat::kNELNoLimits);
    fNumFallMax->Associate(this);
    controlFrame->AddFrame(fNumFallMax, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 20, 2, 2));

    auto *btnRedraw = new TGTextButton(controlFrame, "Redraw", kBtnRedraw);
    btnRedraw->Associate(this);
    controlFrame->AddFrame(btnRedraw, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 2, 2));

    AddFrame(controlFrame, new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));

    fCanvas = new TRootEmbeddedCanvas("HodoHitsCanvas", this, 800, 600);
    AddFrame(fCanvas, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2));
}

Bool_t EVHodoHitsTab::ProcessMessage(Longptr_t msg, Longptr_t parm1, Longptr_t) {
    if (GET_MSG(msg) == kC_COMMAND && GET_SUBMSG(msg) == kCM_BUTTON && parm1 == kBtnRedraw) {
        Redraw();
        return kTRUE;
    }
    if (GET_MSG(msg) == kC_COMMAND && GET_SUBMSG(msg) == kCM_CHECKBUTTON) {
        Redraw();
        return kTRUE;
    }
    if (GET_MSG(msg) == kC_TEXTENTRY && GET_SUBMSG(msg) == kTE_ENTER) {
        Redraw();
        return kTRUE;
    }
    return TGCompositeFrame::ProcessMessage(msg, parm1, 0);
}

bool EVHodoHitsTab::PassEdgeCuts(int riseTDC, int fallTDC) const {
    if (fChkRise->IsOn() &&
        (riseTDC < fNumRiseMin->GetNumber() || riseTDC > fNumRiseMax->GetNumber())) {
        return false;
    }
    if (fChkFall->IsOn() &&
        (fallTDC < fNumFallMin->GetNumber() || fallTDC > fNumFallMax->GetNumber())) {
        return false;
    }
    return true;
}

void EVHodoHitsTab::Redraw() {

    TCanvas *c = fCanvas->GetCanvas();
    c->Clear();
    c->cd();
    fDrawObjects.clear();

    /*
     * The pads are owned by the canvas: TPad sets the kCanDelete bit, so
     * the next c->Clear() deletes them. They must NOT go to fDrawObjects,
     * otherwise they get deleted twice.
     */
    TPad *padH0 = new TPad("pad_hodoHits_0", "", 0.00, 0.50, 0.65, 1.00);
    TPad *padH1 = new TPad("pad_hodoHits_1", "", 0.00, 0.00, 0.65, 0.50);
    TPad *padInfo = new TPad("pad_hodoHits_info", "", 0.65, 0.00, 1.00, 1.00);
    padH0->Draw();
    padH1->Draw();
    padInfo->Draw();

    std::vector<std::pair<TString, Color_t>> infoLines;

    padH0->cd();
    DrawHodoscope(0, infoLines);
    padH1->cd();
    DrawHodoscope(1, infoLines);

    // ============== Hit list on the right side =================
    padInfo->cd();
    auto lat = std::make_unique<TLatex>();
    lat->SetNDC();
    lat->SetTextFont(82); // monospace, to align the columns
    lat->SetTextSize(0.045);

    lat->SetTextColor(kBlack);
    lat->DrawLatex(0.03, 0.955, "det bar    rise   fall");

    const int maxLines = 18;
    const double y0 = 0.905;
    const double dy = 0.048;
    int nShown = 0;
    for (const auto &line : infoLines) {
        if (nShown >= maxLines) {
            lat->SetTextColor(kBlack);
            lat->DrawLatex(0.03, y0 - nShown * dy, Form("... %zu more hits", infoLines.size() - nShown));
            break;
        }
        lat->SetTextColor(line.second);
        lat->DrawLatex(0.03, y0 - nShown * dy, line.first.Data());
        nShown++;
    }
    fDrawObjects.push_back(std::move(lat));

    c->cd();
    c->Update();
}

void EVHodoHitsTab::DrawHodoscope(int det, std::vector<std::pair<TString, Color_t>> &infoLines) {

    gPad->SetLeftMargin(0.07);
    gPad->SetRightMargin(0.02);
    gPad->SetGridx();
    gPad->SetGridy();

    auto hFrame = std::make_unique<TH2D>(Form("h_frame_hodoHits_%d", det),
                                         Form("Hodoscope %d; Short bar ID; Long bar ID", det),
                                         XYHodoTools::nShortBars, 0., double(XYHodoTools::nShortBars),
                                         XYHodoTools::nLongBars, 0., double(XYHodoTools::nLongBars));
    hFrame->SetStats(0);
    hFrame->GetXaxis()->SetNdivisions(-XYHodoTools::nShortBars);
    hFrame->GetYaxis()->SetNdivisions(-XYHodoTools::nLongBars);
    hFrame->Draw();
    fDrawObjects.push_back(std::move(hFrame));

    if (fEvent == nullptr || fEvent->hodoBank == nullptr) {
        return;
    }

    XYHodoTools::XYHodoAnalyzer ana(det);
    ana.SetHitBank(*fEvent->hodoBank);
    if (fChkTOT->IsOn()) {
        ana.SetTOverThreshold(fNumTOT->GetNumber());
    }
    ana.AnalyzeEvent();

    const std::vector<std::shared_ptr<XYHodoTools::XYHodoHit>> *pmtHits[2] = {ana.PMT1_Hits(), ana.PMT2_Hits()};
    const Color_t pmtColors[2] = {kBlue, kRed}; // PMT1 blue, PMT2 red
    const double pmtShift[2] = {-0.12, +0.12}; // so the PMT1 and PMT2 lines do not overlap

    int nDrawn[2] = {0, 0};
    for (int ipmt = 0; ipmt < 2; ipmt++) {
        for (const auto &hit : *pmtHits[ipmt]) {
            if (!PassEdgeCuts(hit->RiseTime(), hit->FallTime())) {
                continue;
            }

            XYHodoTools::DetElement el = hit->DetectorElement();
            std::unique_ptr<TLine> line;
            if (el.layer == 0) { // short bar -> vertical line
                double x = el.barID + 0.5 + pmtShift[ipmt];
                line = std::make_unique<TLine>(x, 0., x, double(XYHodoTools::nLongBars));
            } else { // long bar -> horizontal line
                double y = el.barID + 0.5 + pmtShift[ipmt];
                line = std::make_unique<TLine>(0., y, double(XYHodoTools::nShortBars), y);
            }
            line->SetLineColor(pmtColors[ipmt]);
            line->SetLineWidth(2);
            line->Draw();
            fDrawObjects.push_back(std::move(line));
            nDrawn[ipmt]++;

            infoLines.emplace_back(Form("H%d %s%-2d %6d %6d", det,
                                        el.layer == 0 ? "S" : "L", el.barID,
                                        hit->RiseTime(), hit->FallTime()),
                                   pmtColors[ipmt]);
        }
    }

    auto lat = std::make_unique<TLatex>();
    lat->SetNDC();
    lat->SetTextFont(42);
    lat->SetTextSize(0.05);
    lat->DrawLatex(0.09, 0.92, Form("#color[%d]{PMT1 hits: %d}   #color[%d]{PMT2 hits: %d}",
                                    kBlue, nDrawn[0], kRed, nDrawn[1]));
    fDrawObjects.push_back(std::move(lat));
}
