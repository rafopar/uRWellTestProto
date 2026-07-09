//
// EVEvioReader.h
//
// Reads a raw EVIO file (urwell_maroc_00<run>.evio.<idx>) and decodes it on the
// fly with the Decoder library so the viewer shows the same quantities a decoded
// HIPO file would contain (uRwell U/V pulses + hodoscope TDC hits), PLUS the raw
// non-common-mode-subtracted SRS words used by the "Raw data" tab.
//
// On Open() the CCDB translation tables and the pedestals are loaded and every
// event's raw words are cached, giving random access. ReadEvent() re-parses the
// cached buffer for the requested event and fills the EVEvent.
//

#ifndef EVEVIOREADER_H
#define EVEVIOREADER_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <bank.h>
#include <dictionary.h>

#include "EVEvent.h"
#include "EVReaderBase.h"

#include "AbstractRawDecoder.h"
#include "EventParser.h"
#include "MarocHodoDecoder.h"
#include "PulseBuilder.h"
#include "TranslationTable.h"
#include "uRwellSRSDecoder.h"

// Persistent header-bank decoder (run/event/unixtime), mirroring the one in
// Decoder/uRwellDecoder.cc: the values persist across events and are only
// updated when the header bank is present.
class EVHeaderBankDecoder : public AbstractRawDecoder {
public:
    int run = 0, event = 0, unixtime = 0;
    void Decode(const uint32_t *p, uint32_t len, std::vector<int> &) override {
        if (len >= 2) run = static_cast<int>(p[1]);
        if (len >= 3) event = static_cast<int>(p[2]);
        if (len >= 4 && p[3] != 0) unixtime = static_cast<int>(p[3]);
    }
    void Clear() override {} // persistent: do not reset between events
};

class EVEvioReader : public EVReaderBase {
public:
    bool Open(const char *filename) override;
    int GetEntries() const override { return fEntries; }
    const std::string &GetFileName() const override { return fFileName; }
    bool ReadEvent(int index, EVEvent &ev) override;
    bool IsEvio() const override { return true; }

private:
    void ensureRegistered();

    std::string fFileName;
    int fRun = -1;
    int fEntries = 0;

    // Decode configuration (loaded once in Open()).
    TranslationTable fUrwTT;
    TranslationTable fHodoTT;
    PulseBuilder fBuilder;

    // One raw-word copy per event, for random access.
    std::vector<std::vector<uint32_t>> fEventBuffers;

    // Reusable EVIO parsing objects (decoders registered once).
    EventParser fParser;
    EVHeaderBankDecoder fHeader;
    uRwellSRSDecoder fSrs;
    MarocHodoDecoder fHodo;
    bool fRegistered = false;

    // In-memory XYHODO::tdc bank consumed by the hodoscope tabs; recreated each
    // ReadEvent and kept alive until the next call (EVEvent::hodoBank points here).
    hipo::schema fHodoSchema;
    std::unique_ptr<hipo::bank> fHodoBank;
};

#endif /* EVEVIOREADER_H */
