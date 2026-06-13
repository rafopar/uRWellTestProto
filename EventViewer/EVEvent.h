//
// EVEvent.h
//
// A plain data container holding everything the event viewer needs to know
// about a single event. It is filled by EVReader and consumed by the tabs.
//

#ifndef EVEVENT_H
#define EVEVENT_H

#include <vector>

#include <uRwellTools.h>

struct EVEvent {
    int eventIndex = -1; // 0-based index of the event in the hipo file
    long trueEventNumber = -1; // The event number from the RUN::config bank (if present)
    int runNumber = -1; // The run number from the RUN::config bank (if present)

    std::vector<uRwellTools::APV25Pulse> v_U_Pulses; // uRwell U-layer pulses
    std::vector<uRwellTools::APV25Pulse> v_V_Pulses; // uRwell V-layer pulses

    /*
     * Raw rows of the XYHODO::tdc bank. The "Hodoscope" and "Combined" tabs are
     * placeholders for now, but the data is already read out so those tabs can
     * be implemented later without touching the I/O code.
     */
    struct HodoTDCHit {
        int sector;
        int layer;
        int component;
        int order;
        int tdc;
    };
    std::vector<HodoTDCHit> v_HodoHits;

    int nPulses() const { return int(v_U_Pulses.size() + v_V_Pulses.size()); }
};

#endif /* EVEVENT_H */
