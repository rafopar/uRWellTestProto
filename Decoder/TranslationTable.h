#ifndef URWELL_TRANSLATION_TABLE_H
#define URWELL_TRANSLATION_TABLE_H

#include <string>
#include <unordered_map>
#include <cstdint>

// Loads one CLAS12 translation table (e.g. "/daq/tt/clasdev/Hodo") directly
// from the CCDB SQLite database, replicating CCDB's run/variation resolution:
// among the assignments for (table, variation) whose run range covers `run`,
// the most recent one wins. This keeps the mapping in sync with CCDB (and
// run-dependent) without a static text dump.
//
// Maps (crate,slot,chan) -> (sector,layer,component,order), matching the Java
// DetectorEventDecoder.translate() lookups.
class TranslationTable
{
public:
    struct Entry { int sector, layer, component, order; };

    // Loads the table. connection is a CCDB-style string
    // "sqlite:///<abs-path>" or a bare filesystem path. Returns false on error.
    bool load(const std::string &connection, const std::string &tablePath,
              int run, const std::string &variation = "default");

    // Returns true and fills out if (crate,slot,chan) has an entry.
    bool find(int crate, int slot, int chan, Entry &out) const {
        auto it = fMap.find(key(crate, slot, chan));
        if (it == fMap.end()) return false;
        out = it->second;
        return true;
    }

    size_t size() const { return fMap.size(); }

private:
    static uint64_t key(int crate, int slot, int chan) {
        return (static_cast<uint64_t>(crate) << 40) |
               (static_cast<uint64_t>(slot)  << 20) |
                static_cast<uint64_t>(chan);
    }
    std::unordered_map<uint64_t, Entry> fMap;
};

#endif
