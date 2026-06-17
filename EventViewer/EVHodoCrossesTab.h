//
// EVHodoCrossesTab.h
//
// Shows the crosses (short bar x long bar coincidences) of the two XY
// hodoscopes on the same 33 x 12 rectangles as the "Hits" tab: hodoscope 0
// on the top, hodoscope 1 on the bottom. PMT1 crosses are blue markers,
// PMT2 crosses are red markers (slightly displaced from each other);
// PMT1-PMT2 matched crosses are circled in black. The right side lists
// the bar IDs of every drawn cross.
//
// Cuts (all implemented in XYHodoTools::XYHodoAnalyzer):
//  - time-over-threshold      (SetTOverThreshold)
//  - cross |Delta t|          (SetCrossDeltaT)
//  - PMT1-PMT2 match |Delta t| (SetPMTMatchTimeCut)
//

#ifndef EVHODOCROSSESTAB_H
#define EVHODOCROSSESTAB_H

#include <memory>
#include <utility>
#include <vector>

#include <TObject.h>
#include <TString.h>

#include "EVTab.h"

class TGCheckButton;
class TGNumberEntry;
class TRootEmbeddedCanvas;

class EVHodoCrossesTab : public EVTab {
public:
    EVHodoCrossesTab(const TGWindow *p);

    void Redraw() override;

    Bool_t ProcessMessage(Longptr_t msg, Longptr_t parm1, Longptr_t parm2) override;

private:
    enum EWidgetIDs {
        kChkTOT = 401,
        kNumTOT = 402,
        kChkCrossDT = 403,
        kNumCrossDT = 404,
        kChkPMTMatch = 405,
        kNumPMTMatch = 406,
        kBtnRedraw = 407
    };

    // Draws the hodoscope "det" into the current pad and appends a
    // description of every drawn cross (with its color) to infoLines.
    void DrawHodoscope(int det, std::vector<std::pair<TString, Color_t>> &infoLines);

    TGCheckButton *fChkTOT = nullptr;
    TGCheckButton *fChkCrossDT = nullptr;
    TGCheckButton *fChkPMTMatch = nullptr;
    TGNumberEntry *fNumTOT = nullptr;
    TGNumberEntry *fNumCrossDT = nullptr;
    TGNumberEntry *fNumPMTMatch = nullptr;

    TRootEmbeddedCanvas *fCanvas = nullptr;
    std::vector<std::unique_ptr<TObject>> fDrawObjects; // objects owned by the current drawing
};

#endif /* EVHODOCROSSESTAB_H */
