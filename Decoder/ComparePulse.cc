// Validation: compare uRwell::Pulse banks between two HIPO files as multisets,
// keyed by RUN::config.event.  Reports any field that is not bit-identical.
//
//   ComparePulse <test.hipo> <reference.hipo> [maxEvents]
//
// The comparison is driven by the FIRST (test) file: every event in <test> must
// be present in <reference> with an identical uRwell::Pulse multiset.  Events
// that exist only in <reference> are reported but not counted as mismatches, so
// a short test file (e.g. decoded with -n 100) can be checked against a full
// reference skim.  [maxEvents] limits how many events are read from <test>.
//
// Exit code 0 = identical, 1 = mismatch, 2 = bad input (e.g. no uRwell::Pulse).
#include <reader.h>
#include <dictionary.h>
#include <event.h>
#include <bank.h>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <vector>
#include <string>
#include <algorithm>

struct Row {
    std::vector<float>  f;   // all float fields, in schema order
    std::vector<short>  s;   // all short fields, in schema order
    bool operator<(const Row& o) const {
        if (s != o.s) return s < o.s;
        // bitwise compare of floats so equality means identical bits
        for (size_t i = 0; i < f.size(); ++i)
            if (f[i] != o.f[i]) return f[i] < o.f[i];
        return false;
    }
    bool operator==(const Row& o) const { return s == o.s && f == o.f; }
};

// Returns false if the file has no uRwell::Pulse schema (so we error out with a
// clear message instead of dereferencing a missing schema).
static bool load(const char* path, std::map<int, std::vector<Row>>& out, int maxEvents) {
    hipo::reader r; r.open(path);
    hipo::dictionary d; r.readDictionary(d);
    bool hasPulse = false;
    for (const auto& n : d.getSchemaList()) if (n == "uRwell::Pulse") hasPulse = true;
    if (!hasPulse) {
        fprintf(stderr, "error: \"%s\" has no uRwell::Pulse bank "
                        "(is it a decoded URWELL::adc file? use CompareDecoded.exe)\n", path);
        return false;
    }
    hipo::schema sp = d.getSchema("uRwell::Pulse");
    hipo::bank p(sp), cfg(d.getSchema("RUN::config"));
    std::vector<int> fi, si;
    for (int i = 0; i < sp.getEntries(); ++i)
        (sp.getEntryType(i) == 4 ? fi : si).push_back(i);   // 4=FLOAT, else SHORT(2)
    hipo::event ev;
    int nev = 0;
    while (r.next()) {
        if (maxEvents > 0 && nev >= maxEvents) break;
        r.read(ev); ev.getStructure(p); ev.getStructure(cfg);
        int evtn = cfg.getRows() > 0 ? cfg.getInt("event", 0) : -1;
        auto& v = out[evtn];
        for (int row = 0; row < p.getRows(); ++row) {
            Row R;
            for (int i : fi) R.f.push_back(p.getFloat(i, row));
            for (int i : si) R.s.push_back(p.getShort(i, row));
            v.push_back(R);
        }
        nev++;
    }
    return true;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("usage: ComparePulse <test.hipo> <reference.hipo> [maxEvents]\n");
        return 2;
    }
    int maxEvents = (argc > 3) ? atoi(argv[3]) : -1;

    std::map<int, std::vector<Row>> A, B;            // A = test, B = reference
    if (!load(argv[1], A, maxEvents)) return 2;       // limit only the test file
    if (!load(argv[2], B, -1))        return 2;

    long rowsA = 0, mismatchEvents = 0, mismatchRows = 0, aOnly = 0;
    int firstBad = 1;
    for (auto& kv : A) {                               // driven by the test file
        int e = kv.first;
        auto a = kv.second;
        rowsA += a.size();
        std::sort(a.begin(), a.end());

        if (!B.count(e)) {                            // event only in the test file
            aOnly++; mismatchEvents++; mismatchRows += a.size();
            if (firstBad) { firstBad = 0;
                printf("first mismatch: event %d present in test but missing from reference "
                       "(%zu pulses)\n", e, a.size()); }
            continue;
        }
        auto b = B[e];
        std::sort(b.begin(), b.end());
        if (a.size() != b.size() || !(a == b)) {
            mismatchEvents++;
            size_t n = std::min(a.size(), b.size());
            size_t bad = 0;
            for (size_t i = 0; i < n; ++i) if (!(a[i] == b[i])) bad++;
            mismatchRows += bad + (a.size() > n ? a.size() - n : b.size() - n);
            if (firstBad) { firstBad = 0;
                printf("first mismatching event %d: test rows=%zu reference rows=%zu, "
                       "%zu differing rows\n", e, a.size(), b.size(), bad); }
        }
    }
    long refOnly = (long)B.size() - ((long)A.size() - aOnly);

    printf("test events: %zu   test pulses: %ld   (reference events: %zu)\n",
           A.size(), rowsA, B.size());
    printf("reference-only events (ignored): %ld\n", refOnly);
    printf("mismatching events: %ld   mismatching rows: %ld   (events only in test: %ld)\n",
           mismatchEvents, mismatchRows, aOnly);
    printf("RESULT: %s\n", mismatchEvents == 0 ? "IDENTICAL" : "MISMATCH");
    return mismatchEvents == 0 ? 0 : 1;
}
