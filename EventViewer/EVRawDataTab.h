//
// EVRawDataTab.h
//
// EVIO-only tab under "uRwell". For every APV hybrid present in the event it
// draws one pad with a graph of the sequential word number (X) versus the
// endianness-corrected raw ADC / metadata value (Y) of that hybrid's bank data,
// i.e. the raw values BEFORE common-mode subtraction (which are not stored in
// the decoded HIPO file). A red horizontal line at the common mode of each time
// sample is drawn spanning that time sample's word range.
//
// For non-EVIO (HIPO) input the tab shows an explanatory placeholder.
//

#ifndef EVRAWDATATAB_H
#define EVRAWDATATAB_H

#include <memory>
#include <vector>

#include <TObject.h>

#include "EVTab.h"

class TRootEmbeddedCanvas;

class EVRawDataTab : public EVTab {
public:
    EVRawDataTab(const TGWindow *p);

    void Redraw() override;

private:
    TRootEmbeddedCanvas *fCanvas = nullptr;
    std::vector<std::unique_ptr<TObject>> fDrawObjects; // objects owned by the current drawing
};

#endif /* EVRAWDATATAB_H */
