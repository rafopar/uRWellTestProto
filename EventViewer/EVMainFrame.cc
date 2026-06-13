//
// EVMainFrame.cc
//

#include "EVMainFrame.h"

#include <stdexcept>
#include <string>

#include <TApplication.h>
#include <TGButton.h>
#include <TGClient.h>
#include <TGFileDialog.h>
#include <TGLabel.h>
#include <TGNumberEntry.h>
#include <TGTab.h>
#include <TGTextEntry.h>
#include <TString.h>
#include <WidgetMessageTypes.h>

#include "EVClustersTab.h"
#include "EVHitsTab.h"
#include "EVPulsesTab.h"
#include "EVTab.h"

EVMainFrame::EVMainFrame(const TGWindow *p, const char *filename) : TGMainFrame(p, 1500, 950) {

    if (!fReader.Open(filename)) {
        throw std::runtime_error(std::string("Can not open the input file ") + filename);
    }

    BuildControls();
    BuildTabs();

    SetWindowName(Form("uRwell Event Viewer  -  %s", filename));
    MapSubwindows();
    Resize(1500, 950);
    MapWindow();

    LoadEvent(0);
}

void EVMainFrame::BuildControls() {

    // ============== Event navigation =================
    auto *navFrame = new TGHorizontalFrame(this);

    auto *btnOpen = new TGTextButton(navFrame, "Open file...", kBtnOpenFile);
    btnOpen->Associate(this);
    navFrame->AddFrame(btnOpen, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 15, 2, 2));

    auto *btnPrev = new TGTextButton(navFrame, "<< Prev", kBtnPrevEvent);
    btnPrev->Associate(this);
    navFrame->AddFrame(btnPrev, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 2, 2));

    auto *btnNext = new TGTextButton(navFrame, "Next >>", kBtnNextEvent);
    btnNext->Associate(this);
    navFrame->AddFrame(btnNext, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 2, 2));

    fEventNumEntry = new TGNumberEntry(navFrame, 0, 9, kNumEventEntry,
                                       TGNumberFormat::kNESInteger,
                                       TGNumberFormat::kNEANonNegative,
                                       TGNumberFormat::kNELLimitMinMax,
                                       0, fReader.GetEntries() - 1);
    navFrame->AddFrame(fEventNumEntry, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 15, 5, 2, 2));

    auto *btnGo = new TGTextButton(navFrame, "Go", kBtnGoToEvent);
    btnGo->Associate(this);
    navFrame->AddFrame(btnGo, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 2, 2));

    fEventLabel = new TGLabel(navFrame, Form("Event 0 / %d", fReader.GetEntries() - 1));
    navFrame->AddFrame(fEventLabel, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 20, 5, 2, 2));

    auto *btnExit = new TGTextButton(navFrame, "Exit", kBtnExit);
    btnExit->Associate(this);
    navFrame->AddFrame(btnExit, new TGLayoutHints(kLHintsRight | kLHintsCenterY, 5, 5, 2, 2));

    AddFrame(navFrame, new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));

    // ============== Cuts =================
    auto *cutFrame = new TGHorizontalFrame(this);

    cutFrame->AddFrame(new TGLabel(cutFrame, "Pulse cut:"),
                       new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 2, 2, 2));
    fPulseCutEntry = new TGTextEntry(cutFrame, "", kEntryPulseCut);
    fPulseCutEntry->Associate(this);
    fPulseCutEntry->SetToolTipText("e.g.  pulse_Sigma > 3.5 && pulse_Chi2/pulse_NDF < 40");
    fPulseCutEntry->Resize(350, fPulseCutEntry->GetDefaultHeight());
    cutFrame->AddFrame(fPulseCutEntry, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 10, 2, 2));

    cutFrame->AddFrame(new TGLabel(cutFrame, "Event cut:"),
                       new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 2, 2, 2));
    fEventCutEntry = new TGTextEntry(cutFrame, "", kEntryEventCut);
    fEventCutEntry->Associate(this);
    fEventCutEntry->SetToolTipText("e.g.  n_U_pulses + n_V_pulses > 4   (used by Next/Prev)");
    fEventCutEntry->Resize(350, fEventCutEntry->GetDefaultHeight());
    cutFrame->AddFrame(fEventCutEntry, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 10, 2, 2));

    auto *btnApply = new TGTextButton(cutFrame, "Apply cuts", kBtnApplyCuts);
    btnApply->Associate(this);
    cutFrame->AddFrame(btnApply, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 2, 2));

    AddFrame(cutFrame, new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));

    // ============== Status line =================
    fStatusLabel = new TGLabel(this, "");
    fStatusLabel->SetTextJustify(kTextLeft);
    AddFrame(fStatusLabel, new TGLayoutHints(kLHintsExpandX, 7, 2, 2, 2));
}

void EVMainFrame::BuildTabs() {

    auto *topTab = new TGTab(this);
    AddFrame(topTab, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2));

    // ============== uRwell tab with daughter tabs =================
    TGCompositeFrame *uRwellContainer = topTab->AddTab("uRwell");
    auto *uRwellTab = new TGTab(uRwellContainer);
    uRwellContainer->AddFrame(uRwellTab, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2));

    TGCompositeFrame *pulsesContainer = uRwellTab->AddTab("Pulses");
    auto *pulsesTab = new EVPulsesTab(pulsesContainer);
    pulsesContainer->AddFrame(pulsesTab, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
    AddViewerTab(pulsesTab);

    TGCompositeFrame *hitsContainer = uRwellTab->AddTab("Hits");
    auto *hitsTab = new EVHitsTab(hitsContainer);
    hitsContainer->AddFrame(hitsTab, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
    AddViewerTab(hitsTab);

    TGCompositeFrame *clustersContainer = uRwellTab->AddTab("Clusters");
    auto *clustersTab = new EVClustersTab(clustersContainer);
    clustersContainer->AddFrame(clustersTab, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
    AddViewerTab(clustersTab);

    // ============== Placeholder tabs =================
    TGCompositeFrame *hodoContainer = topTab->AddTab("Hodoscope");
    hodoContainer->AddFrame(new TGLabel(hodoContainer, "Hodoscope plots will be added later."),
                            new TGLayoutHints(kLHintsCenterX | kLHintsCenterY));

    TGCompositeFrame *combinedContainer = topTab->AddTab("Combined");
    combinedContainer->AddFrame(new TGLabel(combinedContainer, "Combined uRwell + Hodoscope plots will be added later."),
                                new TGLayoutHints(kLHintsCenterX | kLHintsCenterY));
}

Bool_t EVMainFrame::ProcessMessage(Longptr_t msg, Longptr_t parm1, Longptr_t) {

    if (GET_MSG(msg) == kC_COMMAND && GET_SUBMSG(msg) == kCM_BUTTON) {
        switch (parm1) {
            case kBtnPrevEvent:
                StepEvent(-1);
                return kTRUE;
            case kBtnNextEvent:
                StepEvent(+1);
                return kTRUE;
            case kBtnGoToEvent:
                LoadEvent(int(fEventNumEntry->GetNumber()));
                return kTRUE;
            case kBtnApplyCuts:
                ApplyCuts();
                return kTRUE;
            case kBtnOpenFile:
                OpenNewFile();
                return kTRUE;
            case kBtnExit:
                CloseWindow();
                return kTRUE;
            default:
                break;
        }
    }

    // Return pressed inside one of the text entries
    if (GET_MSG(msg) == kC_TEXTENTRY && GET_SUBMSG(msg) == kTE_ENTER) {
        if (parm1 == kEntryPulseCut || parm1 == kEntryEventCut) {
            ApplyCuts();
            return kTRUE;
        }
    }

    return TGMainFrame::ProcessMessage(msg, parm1, 0);
}

void EVMainFrame::LoadEvent(int index) {

    index = std::max(0, std::min(index, fReader.GetEntries() - 1));

    if (!fReader.ReadEvent(index, fRawEvent)) {
        SetStatus(Form("Could not read event %d", index));
        return;
    }

    fCurrentIndex = index;
    fFilteredEvent = fCuts.FilterEvent(fRawEvent);

    fEventLabel->SetText(Form("Event %d / %d   (run %d, event number %ld)",
                              fCurrentIndex, fReader.GetEntries() - 1,
                              fRawEvent.runNumber, fRawEvent.trueEventNumber));
    fEventNumEntry->SetNumber(fCurrentIndex);
    Layout();

    SetStatus(Form("%zu U and %zu V pulses (%d before cuts)",
                   fFilteredEvent.v_U_Pulses.size(), fFilteredEvent.v_V_Pulses.size(),
                   fRawEvent.nPulses()));

    UpdateTabs();
}

void EVMainFrame::StepEvent(int dir) {

    int index = fCurrentIndex + dir;

    while (index >= 0 && index < fReader.GetEntries()) {
        if (!fCuts.HasEventCut()) {
            LoadEvent(index);
            return;
        }

        EVEvent raw;
        if (!fReader.ReadEvent(index, raw)) {
            break;
        }
        EVEvent filtered = fCuts.FilterEvent(raw);
        if (fCuts.PassEvent(filtered)) {
            LoadEvent(index);
            return;
        }
        index += dir;
    }

    SetStatus(dir > 0 ? "No following event passes the event cut"
                      : "No preceding event passes the event cut");
}

void EVMainFrame::OpenNewFile() {

    static const char *filetypes[] = {"HIPO files", "*.hipo",
                                      "All files", "*",
                                      nullptr, nullptr};

    TGFileInfo fileInfo;
    fileInfo.fFileTypes = filetypes;
    new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &fileInfo); // deletes itself when closed

    if (fileInfo.fFilename == nullptr) {
        return; // the dialog was cancelled
    }

    // Keep the current file name to be able to fall back to it
    const std::string oldFile = fReader.GetFileName();

    if (!fReader.Open(fileInfo.fFilename)) {
        SetStatus(Form("Can not open %s (no uRwell::Pulse bank?). Staying with the current file.",
                       fileInfo.fFilename));
        fReader.Open(oldFile.c_str());
        return;
    }

    SetWindowName(Form("uRwell Event Viewer  -  %s", fileInfo.fFilename));
    fEventNumEntry->SetLimitValues(0, fReader.GetEntries() - 1);
    LoadEvent(0);
}

void EVMainFrame::ApplyCuts() {

    std::string error;
    if (!fCuts.SetPulseCut(fPulseCutEntry->GetText(), error)) {
        SetStatus(Form("Pulse cut error: %s", error.c_str()));
        return;
    }
    if (!fCuts.SetEventCut(fEventCutEntry->GetText(), error)) {
        SetStatus(Form("Event cut error: %s", error.c_str()));
        return;
    }

    // Re-apply to the current event
    fFilteredEvent = fCuts.FilterEvent(fRawEvent);
    SetStatus(Form("Cuts applied. %zu U and %zu V pulses (%d before cuts)",
                   fFilteredEvent.v_U_Pulses.size(), fFilteredEvent.v_V_Pulses.size(),
                   fRawEvent.nPulses()));
    UpdateTabs();
}

void EVMainFrame::UpdateTabs() {
    for (auto *tab : fTabs) {
        tab->SetEvent(&fFilteredEvent);
    }
}

void EVMainFrame::SetStatus(const char *text) {
    fStatusLabel->SetText(text);
    Layout();
}

void EVMainFrame::CloseWindow() {
    gApplication->Terminate(0);
}
