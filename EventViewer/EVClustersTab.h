//
// EVClustersTab.h
//
// Similar to the "Hits" tab, but the lines drawn are U and V cluster
// centroids (clusters are built with uRwellTools::getPulseClusters).
// UxV intersections of the displayed clusters are marked.
// When the "Only Max ADC clusters" checkbox is selected, only the cluster
// with the highest total pulse integral is shown in each layer, i.e. there
// is at most one UxV cross.
//

#ifndef EVCLUSTERSTAB_H
#define EVCLUSTERSTAB_H

#include <memory>
#include <vector>

#include <TObject.h>

#include "EVTab.h"

class TGCheckButton;
class TRootEmbeddedCanvas;

class EVClustersTab : public EVTab {
public:
    EVClustersTab(const TGWindow *p);

    void Redraw() override;

    Bool_t ProcessMessage(Longptr_t msg, Longptr_t parm1, Longptr_t parm2) override;

private:
    enum EWidgetIDs {
        kChkOnlyMaxADC = 201
    };

    TRootEmbeddedCanvas *fCanvas = nullptr;
    TGCheckButton *fOnlyMaxADCCheck = nullptr;
    std::vector<std::unique_ptr<TObject>> fDrawObjects;
};

#endif /* EVCLUSTERSTAB_H */
