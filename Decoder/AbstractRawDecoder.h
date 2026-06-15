#ifndef ABSTRACT_RAW_DECODER_H
#define ABSTRACT_RAW_DECODER_H

#include <cstdint>
#include <vector>

////////////////////////////////////////////////////////////////
// Minimal interface for a per-bank raw decoder, registered with
// EventParser by (12-bit) bank tag.
//
// This is a slimmed copy of the interface from Xinzhan's
// mpd_gem_view_ssp decoder, dropped of the MPDDataStruct.h
// dependency which we don't need here.

class AbstractRawDecoder
{
public:
    AbstractRawDecoder() = default;
    AbstractRawDecoder(const AbstractRawDecoder &) = delete;
    AbstractRawDecoder & operator = (const AbstractRawDecoder &) = delete;
    AbstractRawDecoder(AbstractRawDecoder &&) = delete;
    AbstractRawDecoder & operator = (AbstractRawDecoder &&) = delete;

    virtual ~AbstractRawDecoder() = default;

    // vTagTrack holds the chain of bank tags from the top-level
    // event bank down to (and including) this leaf bank's tag.
    virtual void Decode(const uint32_t *pBuf, uint32_t fBufLen,
            std::vector<int> &vTagTrack) = 0;

    virtual void Clear() = 0;
};

#endif
