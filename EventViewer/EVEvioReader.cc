//
// EVEvioReader.cc
//

#include "EVEvioReader.h"

#include <cctype>
#include <cstdlib>
#include <iostream>

#include "DecoderTypes.h"
#include "EvioFileReader.h"

namespace {

// Masked (12-bit) bank tags, as in Decoder/uRwellDecoder.cc.
const int TAG_HEADER = 271;
const int TAG_SRS = 287;
const int TAG_HODO = 311;

const char *URWELL_TT = "/daq/tt/clasdev/urwellWithLdrdGem";
const char *HODO_TT = "/daq/tt/clasdev/Hodo";
const char *DEFAULT_CCDB =
    "sqlite:////group/clas12/users/rafopar/uRWellImportant/clas12SecondProto.sqlite";

// Extract <run> from a name like ".../urwell_maroc_003101.evio.00000".
int parseRunFromName(const std::string &path) {
    const std::string key = "urwell_maroc_";
    size_t p = path.rfind(key);
    if (p == std::string::npos) return -1;
    p += key.size();
    size_t q = p;
    while (q < path.size() && std::isdigit(static_cast<unsigned char>(path[q]))) q++;
    if (q == p) return -1;
    return std::atoi(path.substr(p, q - p).c_str());
}

} // namespace

void EVEvioReader::ensureRegistered() {
    if (fRegistered) return;
    fSrs.setRecordRaw(true); // the viewer needs the raw, non-CM-subtracted words
    fParser.RegisterRawDecoder(TAG_HEADER, &fHeader);
    fParser.RegisterRawDecoder(TAG_SRS, &fSrs);
    fParser.RegisterRawDecoder(TAG_HODO, &fHodo);
    fRegistered = true;
}

bool EVEvioReader::Open(const char *filename) {
    fFileName = filename;
    fEventBuffers.clear();
    fEntries = 0;

    fRun = parseRunFromName(fFileName);
    if (fRun <= 0) {
        std::cerr << "EVEvioReader::Open: cannot determine run number from \"" << fFileName
                  << "\" (expected urwell_maroc_00<run>.evio...)" << std::endl;
        return false;
    }

    const char *ccdbEnv = std::getenv("CCDB_CONNECTION");
    const std::string ccdb = ccdbEnv ? ccdbEnv : DEFAULT_CCDB;
    if (!fUrwTT.load(ccdb, URWELL_TT, fRun, "default")) {
        std::cerr << "EVEvioReader::Open: failed to load uRwell translation table from " << ccdb
                  << std::endl;
        return false;
    }
    if (!fHodoTT.load(ccdb, HODO_TT, fRun, "default")) {
        std::cerr << "EVEvioReader::Open: failed to load hodo translation table from " << ccdb
                  << std::endl;
        return false;
    }

    const char *pedEnv = std::getenv("EV_PEDS_DIR");
    const std::string pedDir = pedEnv ? pedEnv : "PedFiles";
    if (!fBuilder.loadPedestals(fRun, pedDir)) {
        std::cerr << "EVEvioReader::Open: failed to load pedestals from \"" << pedDir
                  << "\" for run " << fRun << " (need Peds_" << fRun << " and GEM_Peds_" << fRun
                  << ")" << std::endl;
        return false;
    }

    // Cache every event's raw words (copies) for random access.
    EvioFileReader reader(fFileName);
    if (!reader.OpenFile()) {
        std::cerr << "EVEvioReader::Open: cannot open EVIO file " << fFileName << std::endl;
        return false;
    }
    const uint32_t *buf = nullptr;
    uint32_t len = 0;
    while (reader.ReadNoCopy(&buf, &len) == S_SUCCESS) {
        fEventBuffers.emplace_back(buf, buf + len);
    }
    reader.CloseFile();
    fEntries = static_cast<int>(fEventBuffers.size());

    // XYHODO::tdc schema for the in-memory hodo bank the hodoscope tabs consume.
    fHodoSchema = hipo::schema("XYHODO::tdc", 21850, 13);
    fHodoSchema.parse("sector/B,layer/B,component/S,order/B,TDC/I");

    ensureRegistered();

    std::cout << "EVEvioReader: run " << fRun << ", " << fEntries << " events cached from "
              << fFileName << std::endl;
    return fEntries > 0;
}

bool EVEvioReader::ReadEvent(int index, EVEvent &ev) {
    if (index < 0 || index >= fEntries) return false;
    ensureRegistered();

    ev = EVEvent();
    ev.eventIndex = index;
    ev.isEvio = true;
    ev.runNumber = fRun;

    const std::vector<uint32_t> &words = fEventBuffers[index];
    fParser.ParseEvent(words.data(), static_cast<uint32_t>(words.size())); // clears srs/hodo, keeps header

    if (fHeader.run > 0) ev.runNumber = fHeader.run;
    ev.trueEventNumber = fHeader.event;

    // Translate ADC + TDC rows (drop hits with no TT entry, as the decoder does).
    std::vector<DecodedAdcRow> adc;
    std::vector<DecodedTdcRow> tdc;
    TranslationTable::Entry e;
    for (const URwellRawHit &h : fSrs.hits())
        if (fUrwTT.find(h.crate, h.slot, h.channel, e))
            adc.push_back({e.sector, e.layer, e.component, e.order, h.adc, h.ts});
    for (const HodoRawHit &h : fHodo.hits())
        if (fHodoTT.find(h.crate, h.slot, h.channel, e))
            tdc.push_back({e.sector, e.layer, e.component, e.order, h.tdc});

    // Build pulses and fill the U/V lists (same filtering as EVReader from HIPO).
    std::vector<uRwellTools::APV25Pulse> uRwellPulses, gemPulses;
    fBuilder.build(adc, uRwellPulses, gemPulses);
    for (const auto &p : uRwellPulses) {
        if (p.hit.sector != uRwellTools::sec_TestProto) continue;
        if (p.hit.layer == uRwellTools::layer_U_TestProto)
            ev.v_U_Pulses.push_back(p);
        else if (p.hit.layer == uRwellTools::layer_V_TestProto)
            ev.v_V_Pulses.push_back(p);
    }

    // In-memory XYHODO::tdc bank for the hodoscope tabs + the flat hit list.
    fHodoBank = std::make_unique<hipo::bank>(fHodoSchema, static_cast<int>(tdc.size()));
    for (int i = 0; i < static_cast<int>(tdc.size()); ++i) {
        const DecodedTdcRow &r = tdc[i];
        fHodoBank->putByte("sector", i, static_cast<int8_t>(r.sector));
        fHodoBank->putByte("layer", i, static_cast<int8_t>(r.layer));
        fHodoBank->putShort("component", i, static_cast<int16_t>(r.component));
        fHodoBank->putByte("order", i, static_cast<int8_t>(r.order));
        fHodoBank->putInt("TDC", i, r.tdc);

        EVEvent::HodoTDCHit hit;
        hit.sector = r.sector;
        hit.layer = r.layer;
        hit.component = r.component;
        hit.order = r.order;
        hit.tdc = r.tdc;
        ev.v_HodoHits.push_back(hit);
    }
    ev.hodoBank = fHodoBank.get();

    // Raw SRS words + per-time-sample common mode for the Raw data tab.
    for (const auto &rh : fSrs.rawHybrids()) {
        EVEvent::RawSRSHybrid H;
        H.slot = rh.slot;
        H.words.assign(rh.words.begin(), rh.words.end());
        for (const auto &fr : rh.frames)
            H.frames.push_back({fr.ts, fr.cmnMode, fr.wordLo, fr.wordHi});
        ev.rawSRS.push_back(std::move(H));
    }

    return true;
}
