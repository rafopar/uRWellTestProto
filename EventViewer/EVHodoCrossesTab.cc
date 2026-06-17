//
// EVHodoCrossesTab.cc
//

#include "EVHodoCrossesTab.h"

#include <TCanvas.h>
#include <TGButton.h>
#include <TGLabel.h>
#include <TGNumberEntry.h>
#include <TH2D.h>
#include <TLatex.h>
#include <TMarker.h>
#include <TPad.h>
#include <TRootEmbeddedCanvas.h>
#include <TString.h>
#include <WidgetMessageTypes.h>

#include <XYHodoAnalyzer.h>

EVHodoCrossesTab::EVHodoCrossesTab(const TGWindow *p) : EVTab(p) {

    auto *controlFrame = new TGHorizontalFrame(this);

    fChkTOT = new TGCheckButton(controlFrame, "T over thr. >", kChkTOT);
    fChkTOT->Associate(this);
    controlFrame->AddFrame(fChkTOT, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 2, 2, 2));
    fNumTOT = new TGNumberEntry(controlFrame, 100, 6, kNumTOT, TGNumberFormat::kNESInteger,
                                TGNumberFormat::kNEAAnyNumber, TGNumberFormat::kNELNoLimits);
    fNumTOT->Associate(this);
    controlFrame->AddFrame(fNumTOT, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 20, 2, 2));

    fChkCrossDT = new TGCheckButton(controlFrame, "Cross |#Delta t| <", kChkCrossDT);
    fChkCrossDT->Associate(this);
    controlFrame->AddFrame(fChkCrossDT, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 2, 2, 2));
    fNumCrossDT = new TGNumberEntry(controlFrame, 100, 6, kNumCrossDT, TGNumberFormat::kNESInteger,
                                    TGNumberFormat::kNEAAnyNumber, TGNumberFormat::kNELNoLimits);
    fNumCrossDT->Associate(this);
    controlFrame->AddFrame(fNumCrossDT, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 20, 2, 2));

    fChkPMTMatch = new TGCheckButton(controlFrame, "PMT match |#Delta t| <", kChkPMTMatch);
    fChkPMTMatch->Associate(this);
    controlFrame->AddFrame(fChkPMTMatch, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 2, 2, 2));
    fNumPMTMatch = new TGNumberEntry(controlFrame, 100, 6, kNumPMTMatch, TGNumberFormat::kNESInteger,
                                     TGNumberFormat::kNEAAnyNumber, TGNumberFormat::kNELNoLimits);
    fNumPMTMatch->Associate(this);
    controlFrame->AddFrame(fNumPMTMatch, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 20, 2, 2));

    auto *btnRedraw = new TGTextButton(controlFrame, "Redraw", kBtnRedraw);
    btnRedraw->Associate(this);
    controlFrame->AddFrame(btnRedraw, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 2, 2));

    AddFrame(controlFrame, new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));

    fCanvas = new TRootEmbeddedCanvas("HodoCrossesCanvas", this, 800, 600);
    AddFrame(fCanvas, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2));
}

Bool_t EVHodoCrossesTab::ProcessMessage(Longptr_t msg, Longptr_t parm1, Longptr_t) {
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

void EVHodoCrossesTab::Redraw() {

    TCanvas *c = fCanvas->GetCanvas();
    c->Clear();
    c->cd();
    fDrawObjects.clear();

    /*
     * The pads are owned by the canvas: TPad sets the kCanDelete bit, so
     * the next c->Clear() deletes them. They must NOT go to fDrawObjects,
     * otherwise they get deleted twice.
     */
    TPad *padH0 = new TPad("pad_hodoCrosses_0", "", 0.00, 0.50, 0.65, 1.00);
    TPad *padH1 = new TPad("pad_hodoCrosses_1", "", 0.00, 0.00, 0.65, 0.50);
    TPad *padInfo = new TPad("pad_hodoCrosses_info", "", 0.65, 0.00, 1.00, 1.00);
    padH0->Draw();
    padH1->Draw();
    padInfo->Draw();

    std::vector<std::pair<TString, Color_t>> infoLines;

    padH0->cd();
    DrawHodoscope(0, infoLines);
    padH1->cd();
    DrawHodoscope(1, infoLines);

    // ============== Cross list on the right side =================
    padInfo->cd();
    auto lat = std::make_unique<TLatex>();
    lat->SetNDC();
    lat->SetTextFont(82); // monospace, to align the columns
    lat->SetTextSize(0.045);

    lat->SetTextColor(kBlack);
    lat->DrawLatex(0.03, 0.955, "det      SxL");

    const int maxLines = 18;
    const double y0 = 0.905;
    const double dy = 0.048;
    int nShown = 0;
    for (const auto &line : infoLines) {
        if (nShown >= maxLines) {
            lat->SetTextColor(kBlack);
            lat->DrawLatex(0.03, y0 - nShown * dy, Form("... %zu more crosses", infoLines.size() - nShown));
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

void EVHodoCrossesTab::DrawHodoscope(int det, std::vector<std::pair<TString, Color_t>> &infoLines) {

    gPad->SetLeftMargin(0.07);
    gPad->SetRightMargin(0.02);
    gPad->SetGridx();
    gPad->SetGridy();

    auto hFrame = std::make_unique<TH2D>(Form("h_frame_hodoCrosses_%d", det),
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
    if (fChkCrossDT->IsOn()) {
        ana.SetCrossDeltaT(fNumCrossDT->GetNumber());
    }
    if (fChkPMTMatch->IsOn()) {
        ana.SetPMTMatchTimeCut(fNumPMTMatch->GetNumber());
    }
    ana.AnalyzeEvent();

    const std::vector<std::shared_ptr<XYHodoTools::XYHodoCross>> *pmtCrosses[2] = {ana.PMT1_Crosses(), ana.PMT2_Crosses()};
    const Color_t pmtColors[2] = {kBlue, kRed}; // PMT1 blue, PMT2 red
    const double pmtShift[2] = {-0.12, +0.12}; // so the PMT1 and PMT2 markers do not overlap

    for (int ipmt = 0; ipmt < 2; ipmt++) {
        for (const auto &crs : *pmtCrosses[ipmt]) {
            auto marker = std::make_unique<TMarker>(crs->ShortBarId() + 0.5 + pmtShift[ipmt],
                                                    crs->LongBarId() + 0.5 + pmtShift[ipmt], 20);
            marker->SetMarkerColor(pmtColors[ipmt]);
            marker->SetMarkerSize(1.2);
            marker->Draw();
            fDrawObjects.push_back(std::move(marker));

            infoLines.emplace_back(Form("H%d PMT%d %2dx%-2d", det, ipmt + 1,
                                        crs->ShortBarId(), crs->LongBarId()),
                                   pmtColors[ipmt]);
        }
    }

    // PMT1-PMT2 matched crosses are circled in black
    for (const auto &match : *ana.LR_MatchedCrosses()) {
        auto marker = std::make_unique<TMarker>(match.first->ShortBarId() + 0.5,
                                                match.first->LongBarId() + 0.5, 24);
        marker->SetMarkerColor(kBlack);
        marker->SetMarkerSize(2.4);
        marker->Draw();
        fDrawObjects.push_back(std::move(marker));

        infoLines.emplace_back(Form("H%d mtch %2dx%-2d", det,
                                    match.first->ShortBarId(), match.first->LongBarId()),
                               kBlack);
    }

    auto lat = std::make_unique<TLatex>();
    lat->SetNDC();
    lat->SetTextFont(42);
    lat->SetTextSize(0.05);
    lat->DrawLatex(0.09, 0.92, Form("#color[%d]{PMT1 crosses: %d}   #color[%d]{PMT2 crosses: %d}   matched: %d",
                                    kBlue, ana.GetNPMT1Crosses(), kRed, ana.GetNPMT2Crosses(),
                                    ana.GetNLRMatches()));
    fDrawObjects.push_back(std::move(lat));
}
