#ifndef MAROC_HODO_DECODER_H
#define MAROC_HODO_DECODER_H

#include "AbstractRawDecoder.h"
#include "DecoderTypes.h"
#include <vector>

// 1:1 C++ port of CodaEventDecoder.getDataEntries_57655 (MAROC XY hodoscope).
// Registered with EventParser for the 12-bit-masked tag 311 (= 57655 & 0x0fff).
//
// The bank payload is EVIO composite data, format "c,i,l,N(c,s)":
//   per slot block: slot(int8) trig(int32) time(int64) N(int32)
//                   then N * ( channel(int8), rawtdc(int16) )
// stored TIGHT-PACKED little-endian (no inter-field alignment), with the inner
// data-bank carrying a trailing pad (pad = (hdr2>>14)&3).
class MarocHodoDecoder : public AbstractRawDecoder
{
public:
    void Decode(const uint32_t *pBuf, uint32_t fBufLen,
                std::vector<int> &vTagTrack) override;
    void Clear() override { fHits.clear(); }

    const std::vector<HodoRawHit> &hits() const { return fHits; }

private:
    static const int kHodoSlotOffset = 20;   // CodaEventDecoder.HodoSlotOffset
    std::vector<HodoRawHit> fHits;
};

#endif
