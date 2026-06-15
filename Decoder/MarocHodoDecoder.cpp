#include "MarocHodoDecoder.h"
#include <cstring>
#include <cstdint>

void MarocHodoDecoder::Decode(const uint32_t *pBuf, uint32_t fBufLen,
                              std::vector<int> &vTagTrack)
{
    const int len = static_cast<int>(fBufLen);
    if (len < 4) return;

    // crate = tag immediately above this leaf bank (chain = [event, crate, 311]).
    int crate = (vTagTrack.size() >= 2)
                  ? vTagTrack[vTagTrack.size() - 2]
                  : (vTagTrack.empty() ? -1 : vTagTrack.back());

    // Composite layout:
    //   p[0]            : format tagsegment header, low 16 bits = #format words
    //   p[1 .. fmtLen]  : the format string characters ("c,i,l,N(c,s)")
    //   p[dataBank]     : data-bank length word (words that follow it)
    //   p[dataBank+1]   : data-bank header word 2 (pad = bits 14-15)
    //   p[dataBank+2..] : the packed data
    uint32_t fmtLen   = pBuf[0] & 0xffff;
    int      dataBank = 1 + static_cast<int>(fmtLen);
    if (dataBank + 1 >= len) return;

    uint32_t dlen = pBuf[dataBank];
    uint32_t hdr2 = pBuf[dataBank + 1];
    int      pad  = (hdr2 >> 14) & 0x3;

    const uint8_t *d = reinterpret_cast<const uint8_t *>(&pBuf[dataBank + 2]);
    long nbytes = static_cast<long>(dlen - 1) * 4 - pad;   // data bytes (sans pad)
    if (nbytes <= 0) return;

    long off = 0;
    // header = slot(1) + trig(4) + time(8) + N(4) = 17 bytes
    while (off + 17 <= nbytes) {
        int8_t slotByte = static_cast<int8_t>(d[off]);      off += 1;
        off += 4;                                           // trigger number (unused)
        off += 8;                                           // time stamp     (unused)
        int32_t N;  std::memcpy(&N, d + off, 4);            off += 4;

        int slot = static_cast<int>(slotByte) + kHodoSlotOffset;

        for (int k = 0; k < N && off + 3 <= nbytes; ++k) {
            int channel = d[off] & 0xFF;                    off += 1;
            uint16_t rawtdc; std::memcpy(&rawtdc, d + off, 2); off += 2;

            int edge = (rawtdc >> 15) & 0x1;
            int tdc  = rawtdc & 0x7FFF;
            int maroc_id   = channel / 64;
            int TT_Channel = 2 * (channel - maroc_id * 64) + edge;

            HodoRawHit h;
            h.crate   = crate;
            h.slot    = slot;
            h.channel = TT_Channel;
            h.tdc     = tdc;
            fHits.push_back(h);
        }
    }
}
