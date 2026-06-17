//
// EVHodoHitsTab.h
//
// Shows the hits of the two XY hodoscopes. Each hodoscope is drawn as a
// 33 x 12 rectangle (33 short bars along X, 12 long bars along Y):
// hodoscope 0 on the top, hodoscope 1 on the bottom. A short-bar hit is a
// vertical line, a long-bar hit is a horizontal line at the position of the
// scintillator; PMT1 hits are blue, PMT2 hits are red, slightly displaced
// from each other. On the right side the bank content of every drawn hit
// is listed (detector, bar type, bar ID, rise/fall TDC).
//
// Cuts:
//  - time-over-threshold (XYHodoAnalyzer::SetTOverThreshold)
//  - rising  edge TDC window (applied in this tab)
//  - falling edge TDC window (applied in this tab)
//

#ifndef EVHODOHITSTAB_H
#define EVHODOHITSTAB_H

#include <memory>
#include <utility>
#include <vector>

#include <TObject.h>
#include <TString.h>

#include "EVTab.h"

class TGCheckButton;
class TGNumberEntry;
class TRootEmbeddedCanvas;

class EVHodoHitsTab : public EVTab {
public:
    EVHodoHitsTab(const TGWindow *p);

    void Redraw() override;

    Bool_t ProcessMessage(Longptr_t msg, Longptr_t parm1, Longptr_t parm2) override;

private:
    enum EWidgetIDs {
        kChkTOT = 301,
        kNumTOT = 302,
        kChkRise = 303,
        kNumRiseMin = 304,
        kNumRiseMax = 305,
        kChkFall = 306,
        kNumFallMin = 307,
        kNumFallMax = 308,
        kBtnRedraw = 309
    };

    // Draws the hodoscope "det" into the current pad and appends a
    // description of every drawn hit (with its PMT color) to infoLines.
    void DrawHodoscope(int det, std::vector<std::pair<TString, Color_t>> &infoLines);

    bool PassEdgeCuts(int riseTDC, int fallTDC) const;

    TGCheckButton *fChkTOT = nullptr;
    TGCheckButton *fChkRise = nullptr;
    TGCheckButton *fChkFall = nullptr;
    TGNumberEntry *fNumTOT = nullptr;
    TGNumberEntry *fNumRiseMin = nullptr;
    TGNumberEntry *fNumRiseMax = nullptr;
    TGNumberEntry *fNumFallMin = nullptr;
    TGNumberEntry *fNumFallMax = nullptr;

    TRootEmbeddedCanvas *fCanvas = nullptr;
    std::vector<std::unique_ptr<TObject>> fDrawObjects; // objects owned by the current drawing
};

#endif /* EVHODOHITSTAB_H */
