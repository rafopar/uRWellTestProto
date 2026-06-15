#ifndef URWELL_SRS_DECODER_H
#define URWELL_SRS_DECODER_H

#include "AbstractRawDecoder.h"
#include "DecoderTypes.h"
#include <vector>

// 1:1 C++ port of CodaEventDecoder.getDataEntries_57631 (uRwell SRS-APV).
// Registered with EventParser for the 12-bit-masked tag 287 (= 57631 & 0x0fff).
//
// Decode() fills fHits with one URwellRawHit per (slot=hybrid, channel, ts).
// The crate is taken from vTagTrack (the tag immediately above this leaf bank).
class uRwellSRSDecoder : public AbstractRawDecoder
{
public:
    void Decode(const uint32_t *pBuf, uint32_t fBufLen,
                std::vector<int> &vTagTrack) override;
    void Clear() override { fHits.clear(); }

    const std::vector<URwellRawHit> &hits() const { return fHits; }

private:
    std::vector<URwellRawHit> fHits;
};

#endif
