#include "uRwellSRSDecoder.h"

#include <map>
#include <vector>
#include <algorithm>
#include <cstdint>

// Byte-swap a 16-bit value (Java Short.reverseBytes), result kept signed.
static inline int16_t revShort(int16_t v) {
    uint16_t u = static_cast<uint16_t>(v);
    return static_cast<int16_t>((u << 8) | (u >> 8));
}

void uRwellSRSDecoder::Decode(const uint32_t *pBuf, uint32_t fBufLen,
                              std::vector<int> &vTagTrack)
{
    // Constants from Java getDataEntries_57631.
    const int16_t HEADER    = 1500;
    const int     n_APV_CH  = 128;

    // crate = tag immediately above this leaf bank (chain = [event, crate, 287]).
    int crate = (vTagTrack.size() >= 2)
                  ? vTagTrack[vTagTrack.size() - 2]
                  : (vTagTrack.empty() ? -1 : vTagTrack.back());

    const int len = static_cast<int>(fBufLen);

    // ---- Pass 1: split the buffer into per-hybrid APV sample vectors ----------
    // Java: int[] intBuff = node.getIntData(); then Integer.reverseBytes each word.
    // Our pBuf words equal Java's getIntData() *before* reverseBytes, so we bswap.
    std::map<int, std::vector<int16_t>> m_APV;   // hybrid id -> APV samples
    int curHybrid = -1;

    auto bswap32 = [](uint32_t w) -> uint32_t { return __builtin_bswap32(w); };
    auto addWord = [&](uint32_t rawWord) {
        uint32_t w = bswap32(rawWord);
        int16_t w1 = revShort(static_cast<int16_t>((w >> 16) & 0xffff));
        int16_t w2 = revShort(static_cast<int16_t>(w & 0xffff));
        std::vector<int16_t> &lst = m_APV[curHybrid];
        lst.push_back(w1);
        lst.push_back(w2);
    };

    for (int idata = 0; idata < len; idata++) {

        bool haveNext = (idata + 1 < len);
        uint32_t next = haveNext ? bswap32(pBuf[idata + 1]) : 0;

        if (haveNext && ((next >> 8) & 0xffffff) == 0x414443) {
            // "ADC" marker: least sig byte of next word is the Hybrid id.
            curHybrid = static_cast<int16_t>(next & 0xff);
            m_APV[curHybrid];           // create empty entry (Java: put new list)
            idata += 2;                 // skip marker + FEC-id word
        } else if (haveNext && next == 0xfafafafau) {
            // 0xfafafafa marks the end of SRS data for this hybrid.
            addWord(pBuf[idata]);
            idata += 1;
        } else {
            addWord(pBuf[idata]);
        }
    }

    // ---- Pass 2: per hybrid, find APV frames, subtract common mode ------------
    for (auto &entry : m_APV) {
        const int slot = entry.first;            // hybrid id == slot
        const std::vector<int16_t> &cur = entry.second;
        const int sz = static_cast<int>(cur.size());

        // Optionally keep the raw word sequence of this hybrid for the viewer.
        uRwellSRSDecoder::RawHybrid *rawH = nullptr;
        if (fRecordRaw) {
            fRawHybrids.push_back({slot, cur, {}});
            rawH = &fRawHybrids.back();
        }

        int ts = 0;
        for (int i_apv = 0; i_apv < sz - 3; i_apv++) {

            // APV data begins where three consecutive samples are below HEADER.
            if (cur[i_apv] < HEADER && cur[i_apv + 1] < HEADER &&
                cur[i_apv + 2] < HEADER && (i_apv + 138 < sz + 1)) {

                const int frameStart = i_apv;    // first word (metadata) of this frame

                i_apv += 12;             // 3 header + 8 address + 1 error word

                const bool specialSlot = (slot == 0 || slot == 6);

                // Common mode: skip first/last channels per special-slot masks,
                // sort, drop the lowest 5, average the rest.
                int i_apvTmp = i_apv;
                std::vector<int> tmpList;
                tmpList.reserve(n_APV_CH);
                for (int ich = 0; ich < n_APV_CH; ich++) {
                    if (specialSlot) {
                        // Edge APVs (slots 0 and 6) are half-populated: only 64 of the
                        // 128 channels are uRwell (sector 6) strips, the other 64 map
                        // to sector 7. The mask selects the sector-6 half (mask >= 64)
                        // so the common mode is built only from real uRwell strips.
                        // Verified against the CCDB translation table: for BOTH slot 0
                        // and slot 6, sector 6 <-> mask >= 64.
                        //
                        // NOTE: this deviates on purpose from the Java getDataEntries_57631,
                        // which used mask < 64 for slot 6 and therefore built its common
                        // mode from the sector-7 channels (biased high).
                        int mask = 32 * (ich % 4) + 8 * (ich / 4) - 31 * (ich / 16);
                        if (mask >= 64) tmpList.push_back(static_cast<int>(cur[i_apvTmp]));
                    } else {
                        tmpList.push_back(static_cast<int>(cur[i_apvTmp]));
                    }
                    i_apvTmp++;
                }

                std::sort(tmpList.begin(), tmpList.end());
                double cmnMode = 0;
                for (int ich = 5; ich < static_cast<int>(tmpList.size()); ich++)
                    cmnMode += tmpList[ich];
                cmnMode = cmnMode / static_cast<double>(tmpList.size() - 5);

                // Record this time-sample frame (metadata + 128 samples) for the
                // viewer: word range [frameStart, frameStart+139], clamped.
                if (rawH != nullptr) {
                    int wordHi = frameStart + 12 + n_APV_CH - 1;
                    if (wordHi > sz - 1) wordHi = sz - 1;
                    rawH->frames.push_back({ts, cmnMode, frameStart, wordHi});
                }

                for (int ich = 0; ich < n_APV_CH; ich++) {
                    URwellRawHit h;
                    h.crate   = crate;
                    h.slot    = slot;
                    h.channel = ich;
                    h.ts      = ts;
                    h.adc     = static_cast<int>(cur[i_apv] - cmnMode);
                    fHits.push_back(h);
                    i_apv++;
                }
                i_apv--;
                ts++;
            }
        }
    }
}
