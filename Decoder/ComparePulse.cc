// Validation: compare uRwell::Pulse banks between two HIPO files as multisets,
// keyed by RUN::config.event.  Reports any field that is not bit-identical.
#include <reader.h>
#include <dictionary.h>
#include <event.h>
#include <bank.h>
#include <cstdio>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>

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

static void load(const char* path, std::map<int, std::vector<Row>>& out) {
    hipo::reader r; r.open(path);
    hipo::dictionary d; r.readDictionary(d);
    hipo::schema sp = d.getSchema("uRwell::Pulse");
    hipo::bank p(sp), cfg(d.getSchema("RUN::config"));
    std::vector<int> fi, si;
    for (int i = 0; i < sp.getEntries(); ++i)
        (sp.getEntryType(i) == 4 ? fi : si).push_back(i);   // 4=FLOAT, else SHORT(2)
    hipo::event ev;
    while (r.next()) {
        r.read(ev); ev.getStructure(p); ev.getStructure(cfg);
        int evtn = cfg.getRows() > 0 ? cfg.getInt("event", 0) : -1;
        auto& v = out[evtn];
        for (int row = 0; row < p.getRows(); ++row) {
            Row R;
            for (int i : fi) R.f.push_back(p.getFloat(i, row));
            for (int i : si) R.s.push_back(p.getShort(i, row));
            v.push_back(R);
        }
    }
}

int main(int argc, char** argv) {
    std::map<int, std::vector<Row>> A, B;
    load(argv[1], A);
    load(argv[2], B);

    long rowsA = 0, mismatchEvents = 0, mismatchRows = 0;
    // union of event keys
    std::map<int, int> keys;
    for (auto& kv : A) keys[kv.first] = 1;
    for (auto& kv : B) keys[kv.first] = 1;

    int firstBad = 1;
    for (auto& kv : keys) {
        int e = kv.first;
        auto a = A.count(e) ? A[e] : std::vector<Row>();
        auto b = B.count(e) ? B[e] : std::vector<Row>();
        rowsA += a.size();
        std::sort(a.begin(), a.end());
        std::sort(b.begin(), b.end());
        if (a.size() != b.size() || !(a == b)) {
            mismatchEvents++;
            size_t n = std::min(a.size(), b.size());
            size_t bad = 0;
            for (size_t i = 0; i < n; ++i) if (!(a[i] == b[i])) bad++;
            mismatchRows += bad + (a.size() > n ? a.size() - n : b.size() - n);
            if (firstBad) {
                firstBad = 0;
                printf("first mismatching event %d: A rows=%zu B rows=%zu, %zu differing rows\n",
                       e, a.size(), b.size(), bad);
            }
        }
    }
    printf("events compared: %zu   A rows total: %ld\n", keys.size(), rowsA);
    printf("mismatching events: %ld   mismatching rows: %ld\n", mismatchEvents, mismatchRows);
    printf("RESULT: %s\n", mismatchEvents == 0 ? "IDENTICAL" : "MISMATCH");
    return mismatchEvents == 0 ? 0 : 1;
}
