//
// EVTab.h
//
// The base class of all event viewer tabs. To add a new tab:
//  1. Derive from EVTab, build the tab widgets in the constructor and
//     implement Redraw() (fEvent points to the current event, it can be
//     nullptr before the first event is loaded).
//  2. Create the tab in EVMainFrame::BuildTabs() and register it with
//     AddViewerTab() so it receives the events.
//

#ifndef EVTAB_H
#define EVTAB_H

#include <TGFrame.h>

#include "EVEvent.h"

class EVTab : public TGCompositeFrame {
public:
    EVTab(const TGWindow *p) : TGCompositeFrame(p, 10, 10, kVerticalFrame) {
    }

    ~EVTab() override = default;

    // Called by the main frame whenever a new event is loaded.
    virtual void SetEvent(const EVEvent *ev) {
        fEvent = ev;
        Redraw();
    }

    virtual void Redraw() = 0;

protected:
    const EVEvent *fEvent = nullptr;
};

#endif /* EVTAB_H */
