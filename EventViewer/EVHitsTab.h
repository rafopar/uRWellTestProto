//
// EVHitsTab.h
//
// Draws the uRwell active area, with the U (red) and V (blue) strips of the
// event drawn at their real detector coordinates.
//

#ifndef EVHITSTAB_H
#define EVHITSTAB_H

#include <memory>
#include <vector>

#include <TObject.h>

#include "EVTab.h"

class TRootEmbeddedCanvas;

class EVHitsTab : public EVTab {
public:
    EVHitsTab(const TGWindow *p);

    void Redraw() override;

private:
    TRootEmbeddedCanvas *fCanvas = nullptr;
    std::vector<std::unique_ptr<TObject>> fDrawObjects;
};

#endif /* EVHITSTAB_H */
