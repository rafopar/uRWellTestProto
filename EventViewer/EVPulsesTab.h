//
// EVPulsesTab.h
//
// Shows "ADC vs time sample" of every uRwell pulse of the event, overlaid
// with the Landau fit function p0 + A0*Landau(t, MPV, Sigma) and a dashed
// line at the strip noise (ped_rms). U pulses are drawn in red, V pulses
// in blue. At most 10 pulses are shown per page; the page can be changed
// with the "Prev/Next page" buttons.
//

#ifndef EVPULSESTAB_H
#define EVPULSESTAB_H

#include <memory>
#include <vector>

#include <TObject.h>

#include "EVTab.h"

class TGLabel;
class TRootEmbeddedCanvas;

class EVPulsesTab : public EVTab {
public:
    EVPulsesTab(const TGWindow *p);

    void SetEvent(const EVEvent *ev) override;
    void Redraw() override;

    Bool_t ProcessMessage(Longptr_t msg, Longptr_t parm1, Longptr_t parm2) override;

private:
    enum EWidgetIDs {
        kBtnPrevPage = 101,
        kBtnNextPage = 102
    };

    static constexpr int kPulsesPerPage = 10;

    TRootEmbeddedCanvas *fCanvas = nullptr;
    TGLabel *fPageLabel = nullptr;

    int fCurrentPage = 0;
    std::vector<std::unique_ptr<TObject>> fDrawObjects; // objects owned by the current drawing
};

#endif /* EVPULSESTAB_H */
