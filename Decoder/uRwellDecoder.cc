// Standalone C++ uRwell/hodoscope EVIO -> HIPO decoder.
//
// Reproduces the bank contents of the Java coatjava decoder
// (CodaEventDecoder.getDataEntries_57631 / _57655 + DetectorEventDecoder.translate)
// for the uRwell test-prototype data (single crate, single FEC).
//
//   uRwellDecoder -i <in.evio> -o <out.hipo> [-r <run>] [-n <maxEvents>]
//                 [-v <variation>] [--ccdb <conn>]
//                 [--pulse [--peds-dir <dir>]]
//
// Default output is the full URWELL::adc bank (one row per strip x time sample).
// With --pulse the decoder instead reproduces what AnaCodes/Skim_PulseFit.cc
// does: it builds the strip waveforms, keeps only strips above the 3-sigma
// threshold, Landau-fits each, and writes the small uRwell::Pulse bank.  This
// fuses decode + Skim_PulseFit into one pass and avoids writing/reading the
// gigantic URWELL::adc bank.  --pulse needs the pedestal files
// <peds-dir>/Peds_<run> and <peds-dir>/GEM_Peds_<run> (default dir "PedFiles").
//
// The crate is always taken from the EVIO bank hierarchy. The -c flag (in the
// Java decoder this selects the HIPO compression type) is accepted for CLI
// compatibility but ignored here: the bundled C++ hipo4 writer uses its default.
//
// An existing output file is never overwritten: if -o already exists the program
// reports it and exits.

#include "EvioFileReader.h"
#include "EventParser.h"
#include "AbstractRawDecoder.h"
#include "uRwellSRSDecoder.h"
#include "MarocHodoDecoder.h"
#include "TranslationTable.h"
#include "HipoBankWriter.h"
#include "PulseFitWriter.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <sys/stat.h>

// Masked (12-bit) bank tags as seen by EventParser.
static const int TAG_HEADER = 271;   // 57615 & 0x0fff
static const int TAG_SRS    = 287;   // 57631 & 0x0fff
static const int TAG_HODO   = 311;   // 57655 & 0x0fff

static const char *URWELL_TT = "/daq/tt/clasdev/urwellWithLdrdGem";
static const char *HODO_TT   = "/daq/tt/clasdev/Hodo";
static const char *DEFAULT_CCDB =
    "sqlite:////group/clas12/users/rafopar/uRWellImportant/clas12SecondProto.sqlite";

static bool fileExists(const std::string &path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

// Header bank (57615 -> 271): run/event/unixtime persist across events, exactly
// as CodaEventDecoder keeps them as members (set only when the bank is present).
class HeaderBankDecoder : public AbstractRawDecoder {
public:
    int run = 0, event = 0, unixtime = 0;
    void Decode(const uint32_t *p, uint32_t len, std::vector<int> &) override {
        // payload p[k] == Java intData[k+2]; run=intData[3], event=[4], unix=[5].
        // Read directly (no byte swap) - matches Java toIntArray logical ints.
        if (len >= 2) run      = static_cast<int>(p[1]);
        if (len >= 3) event    = static_cast<int>(p[2]);
        if (len >= 4 && p[3] != 0) unixtime = static_cast<int>(p[3]);
    }
    void Clear() override {}   // persistent: do not reset between events
};

static int parseRunFromName(const std::string &path) {
    const std::string key = "urwell_maroc_";
    size_t p = path.rfind(key);
    if (p == std::string::npos) return -1;
    p += key.size();
    size_t q = p;
    while (q < path.size() && isdigit(path[q])) q++;
    if (q == p) return -1;
    return atoi(path.substr(p, q - p).c_str());
}

int main(int argc, char **argv) {
    std::string inFile, outFile, ccdb = DEFAULT_CCDB, variation = "default";
    std::string pedsDir = "PedFiles";
    int runArg = -1, maxEvents = -1;
    bool pulseMode = false;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        auto next = [&]() -> const char * { return (i + 1 < argc) ? argv[++i] : ""; };
        if      (a == "-i") inFile = next();
        else if (a == "-o") outFile = next();
        else if (a == "-r") runArg = atoi(next());
        else if (a == "-n") maxEvents = atoi(next());
        else if (a == "-v") variation = next();
        else if (a == "--ccdb") ccdb = next();
        else if (a == "--pulse") pulseMode = true;
        else if (a == "--peds-dir") pedsDir = next();
        else if (a == "-c") next();            // compression (Java compat), ignored
    }
    if (inFile.empty() || outFile.empty()) {
        fprintf(stderr, "usage: uRwellDecoder -i <in.evio> -o <out.hipo> "
                        "[-r run] [-n maxEvents] [-v variation] [--ccdb conn] "
                        "[--pulse [--peds-dir dir]]\n");
        return 1;
    }

    // Never overwrite an existing output file.
    if (fileExists(outFile)) {
        fprintf(stderr, "error: output file \"%s\" already exists; refusing to overwrite. "
                        "Remove it or choose a different -o.\n", outFile.c_str());
        return 5;
    }

    int run = (runArg > 0) ? runArg : parseRunFromName(inFile);
    if (run <= 0) {
        fprintf(stderr, "error: could not determine run number; pass -r <run>\n");
        return 1;
    }

    // Translation tables (run-dependent, resolved from CCDB sqlite).
    TranslationTable urwTT, hodoTT;
    if (!urwTT.load(ccdb, URWELL_TT, run, variation)) return 2;
    if (!hodoTT.load(ccdb, HODO_TT, run, variation)) return 2;
    printf("Loaded TT: urwell=%zu rows, hodo=%zu rows (run %d, variation %s)\n",
           urwTT.size(), hodoTT.size(), run, variation.c_str());

    EvioFileReader reader(inFile);
    if (!reader.OpenFile()) return 3;

    EventParser parser;
    HeaderBankDecoder header;
    uRwellSRSDecoder srs;
    MarocHodoDecoder hodo;
    parser.RegisterRawDecoder(TAG_HEADER, &header);
    parser.RegisterRawDecoder(TAG_SRS, &srs);
    parser.RegisterRawDecoder(TAG_HODO, &hodo);

    // Output writer: full URWELL::adc (default) or fused pulse-fit (--pulse).
    HipoBankWriter adcWriter;
    PulseFitWriter pulseWriter;
    if (pulseMode) {
        if (!pulseWriter.open(outFile, run, pedsDir)) return 4;
        printf("Pulse-fit mode: pedestals from %s/Peds_%d and %s/GEM_Peds_%d\n",
               pedsDir.c_str(), run, pedsDir.c_str(), run);
    } else {
        adcWriter.open(outFile);
    }

    const uint32_t *buf;
    uint32_t buflen;
    long nev = 0, nADCrows = 0, nTDCrows = 0;
    std::vector<HipoBankWriter::AdcRow> adc;
    std::vector<HipoBankWriter::TdcRow> tdc;

    while (reader.ReadNoCopy(&buf, &buflen) == S_SUCCESS) {
        if (maxEvents > 0 && nev >= maxEvents) break;
        parser.ParseEvent(buf, buflen);   // clears srs/hodo, keeps header

        adc.clear();
        tdc.clear();

        TranslationTable::Entry e;
        for (const URwellRawHit &h : srs.hits())
            if (urwTT.find(h.crate, h.slot, h.channel, e))
                adc.push_back({e.sector, e.layer, e.component, e.order, h.adc, h.ts});

        for (const HodoRawHit &h : hodo.hits())
            if (hodoTT.find(h.crate, h.slot, h.channel, e))
                tdc.push_back({e.sector, e.layer, e.component, e.order, h.tdc});

        if (pulseMode)
            pulseWriter.writeEvent(header.run, header.event, header.unixtime, adc, tdc);
        else
            adcWriter.writeEvent(header.run, header.event, header.unixtime, adc, tdc);

        nADCrows += adc.size();
        nTDCrows += tdc.size();
        nev++;
    }

    if (pulseMode) {
        pulseWriter.close();
        printf("Decoded %ld events -> %s  (pulse-fit: %ld events kept, %ld pulses, "
               "XYHODO rows=%ld)\n",
               nev, outFile.c_str(), pulseWriter.eventsWritten(),
               pulseWriter.pulsesWritten(), nTDCrows);
    } else {
        adcWriter.close();
        printf("Decoded %ld events -> %s  (URWELL rows=%ld, XYHODO rows=%ld)\n",
               nev, outFile.c_str(), nADCrows, nTDCrows);
    }
    return 0;
}
