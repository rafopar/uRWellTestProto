#ifndef URWELL_SRS_DECODER_H
#define URWELL_SRS_DECODER_H

#include "AbstractRawDecoder.h"
#include "DecoderTypes.h"
#include <cstdint>
#include <vector>

// 1:1 C++ port of CodaEventDecoder.getDataEntries_57631 (uRwell SRS-APV).
// Registered with EventParser for the 12-bit-masked tag 287 (= 57631 & 0x0fff).
//
// Decode() fills fHits with one URwellRawHit per (slot=hybrid, channel, ts).
// The crate is taken from vTagTrack (the tag immediately above this leaf bank).
//
// Optionally (setRecordRaw(true)) it also records, per hybrid, the raw
// endianness-corrected 16-bit word sequence of the bank plus the common mode of
// each time-sample frame. This is used by the event viewer's "Raw data" tab to
// display the uncorrected ADC values that are not present in the decoded output.
// Recording is OFF by default so the standard decode is unaffected.
class uRwellSRSDecoder : public AbstractRawDecoder
{
public:
    // One time-sample frame inside a hybrid's word sequence.
    struct RawFrame {
        int ts;          // time-sample index
        double cmnMode;  // common mode of this frame
        int wordLo;      // first word index of the frame (start of its metadata)
        int wordHi;      // last word index of the frame (last of its 128 samples)
    };
    // The raw word sequence of one hybrid and the frames found inside it.
    struct RawHybrid {
        int slot;                       // hybrid id
        std::vector<int16_t> words;     // endianness-corrected sequential words
        std::vector<RawFrame> frames;   // one entry per time sample
    };

    void Decode(const uint32_t *pBuf, uint32_t fBufLen,
                std::vector<int> &vTagTrack) override;
    void Clear() override { fHits.clear(); fRawHybrids.clear(); }

    const std::vector<URwellRawHit> &hits() const { return fHits; }

    // Enable/disable raw-word recording (default OFF).
    void setRecordRaw(bool v) { fRecordRaw = v; }
    const std::vector<RawHybrid> &rawHybrids() const { return fRawHybrids; }

private:
    std::vector<URwellRawHit> fHits;

    bool fRecordRaw = false;
    std::vector<RawHybrid> fRawHybrids;
};

#endif
