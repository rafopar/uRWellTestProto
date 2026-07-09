//
// EVMainFrame.h
//
// The main window of the event viewer. It owns the file reader, the cut
// engine, the event navigation/cut controls and the hierarchy of tabs:
//   uRwell     -> Pulses | Hits | Clusters | Raw data (EVIO only)
//   Hodoscope  -> Hits | Crosses
//   Combined   -> (placeholder)
//
// The reader is chosen by file type: a HIPO file is read by EVReader, a raw
// EVIO file (urwell_maroc_00<run>.evio.<idx>) by EVEvioReader (decoded on the
// fly). Both are used through the EVReaderBase interface.
//

#ifndef EVMAINFRAME_H
#define EVMAINFRAME_H

#include <memory>
#include <vector>

#include <TGFrame.h>

#include "EVCutEngine.h"
#include "EVEvent.h"
#include "EVReaderBase.h"

class EVTab;
class TGLabel;
class TGNumberEntry;
class TGTextEntry;

class EVMainFrame : public TGMainFrame {
public:
    // Throws std::runtime_error if the file can not be opened.
    EVMainFrame(const TGWindow *p, const char *filename);

    Bool_t ProcessMessage(Longptr_t msg, Longptr_t parm1, Longptr_t parm2) override;

    void CloseWindow() override;

private:
    enum EWidgetIDs {
        kBtnPrevEvent = 1,
        kBtnNextEvent = 2,
        kNumEventEntry = 3,
        kBtnGoToEvent = 4,
        kEntryPulseCut = 5,
        kEntryEventCut = 6,
        kBtnApplyCuts = 7,
        kBtnOpenFile = 8,
        kBtnExit = 9
    };

    // Builds the reader that matches the file type and opens the file.
    // Returns nullptr if the file can not be opened.
    static std::unique_ptr<EVReaderBase> MakeReader(const char *filename);

    void BuildControls();
    void BuildTabs();

    void AddViewerTab(EVTab *tab) { fTabs.push_back(tab); }

    // Loads the event with the given index (no event-cut search)
    void LoadEvent(int index);

    // Steps to the next (dir = +1) or previous (dir = -1) event that
    // passes the event cut.
    void StepEvent(int dir);

    // Shows a file dialog and switches the viewer to the chosen file.
    void OpenNewFile();

    void ApplyCuts();
    void UpdateTabs();
    void SetStatus(const char *text);

    std::unique_ptr<EVReaderBase> fReader;
    EVCutEngine fCuts;

    EVEvent fRawEvent; // the current event as read from the file
    EVEvent fFilteredEvent; // the current event after the pulse cut
    int fCurrentIndex = 0;

    std::vector<EVTab *> fTabs;

    TGLabel *fEventLabel = nullptr;
    TGLabel *fStatusLabel = nullptr;
    TGNumberEntry *fEventNumEntry = nullptr;
    TGTextEntry *fPulseCutEntry = nullptr;
    TGTextEntry *fEventCutEntry = nullptr;
};

#endif /* EVMAINFRAME_H */
