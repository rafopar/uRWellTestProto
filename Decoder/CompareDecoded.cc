// Validation: compare two decoded HIPO files event-by-event, checking that the
// URWELL::adc, XYHODO::tdc and RUN::config.event contents are identical as
// order-insensitive multisets. Intended to compare the C++ decoder output
// against the Java coatjava reference.
//
//   CompareDecoded <fileA.hipo> <fileB.hipo> [maxEvents]
//
// Exit code 0 = identical, 1 = mismatch / error.

#include "reader.h"
#include "dictionary.h"
#include "event.h"
#include "bank.h"

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <array>
#include <algorithm>
#include <string>

struct Side {
    hipo::reader reader;
    hipo::dictionary factory;
    hipo::bank urw, hodo, cfg;
    hipo::event ev;
    explicit Side(const char *f) {
        reader.open(f);
        reader.readDictionary(factory);
        urw  = hipo::bank(factory.getSchema("URWELL::adc"));
        hodo = hipo::bank(factory.getSchema("XYHODO::tdc"));
        cfg  = hipo::bank(factory.getSchema("RUN::config"));
        ev   = hipo::event(8 * 1024 * 1024);
    }
    bool next() { return reader.next(); }
    void load() {
        reader.read(ev);
        ev.getStructure(cfg);
        ev.getStructure(urw);
        ev.getStructure(hodo);
    }
    int cfgEvent() { return cfg.getRows() > 0 ? cfg.getInt("event", 0) : -1; }
};

static std::vector<std::array<int,6>> urwRows(hipo::bank &b) {
    std::vector<std::array<int,6>> v;
    v.reserve(b.getRows());
    for (int i = 0; i < b.getRows(); ++i)
        v.push_back({ b.getInt("sector",i), b.getInt("layer",i), b.getInt("component",i),
                      b.getInt("order",i), b.getInt("ADC",i), b.getInt("ped",i) });
    std::sort(v.begin(), v.end());
    return v;
}
static std::vector<std::array<int,5>> hodoRows(hipo::bank &b) {
    std::vector<std::array<int,5>> v;
    v.reserve(b.getRows());
    for (int i = 0; i < b.getRows(); ++i)
        v.push_back({ b.getInt("sector",i), b.getInt("layer",i), b.getInt("component",i),
                      b.getInt("order",i), b.getInt("TDC",i) });
    std::sort(v.begin(), v.end());
    return v;
}

int main(int argc, char **argv) {
    if (argc < 3) { printf("usage: CompareDecoded <A.hipo> <B.hipo> [maxEvents]\n"); return 1; }
    int maxev = (argc > 3) ? atoi(argv[3]) : -1;

    Side A(argv[1]), B(argv[2]);

    long nev = 0, mism = 0;
    long totURW = 0, totHODO = 0;
    while (true) {
        bool a = A.next(), b = B.next();
        if (a != b) { printf("MISMATCH: event count differs (A has %s, B has %s) at event %ld\n",
                             a?"more":"fewer", b?"more":"fewer", nev); mism++; break; }
        if (!a) break;
        if (maxev > 0 && nev >= maxev) break;
        A.load(); B.load();

        bool bad = false;
        if (A.cfgEvent() != B.cfgEvent()) {
            printf("event %ld: RUN::config.event differs A=%d B=%d\n", nev, A.cfgEvent(), B.cfgEvent());
            bad = true;
        }
        auto ua = urwRows(A.urw), ub = urwRows(B.urw);
        if (ua != ub) {
            printf("event %ld: URWELL::adc differs (A rows=%zu, B rows=%zu)\n", nev, ua.size(), ub.size());
            // show first differing row
            size_t n = std::min(ua.size(), ub.size());
            for (size_t i = 0; i < n; ++i) if (ua[i] != ub[i]) {
                printf("   first diff @sorted #%zu: A[s%d,l%d,c%d,o%d,ADC%d,ped%d] B[s%d,l%d,c%d,o%d,ADC%d,ped%d]\n",
                       i, ua[i][0],ua[i][1],ua[i][2],ua[i][3],ua[i][4],ua[i][5],
                          ub[i][0],ub[i][1],ub[i][2],ub[i][3],ub[i][4],ub[i][5]);
                break;
            }
            bad = true;
        }
        auto ha = hodoRows(A.hodo), hb = hodoRows(B.hodo);
        if (ha != hb) {
            printf("event %ld: XYHODO::tdc differs (A rows=%zu, B rows=%zu)\n", nev, ha.size(), hb.size());
            size_t n = std::min(ha.size(), hb.size());
            for (size_t i = 0; i < n; ++i) if (ha[i] != hb[i]) {
                printf("   first diff @sorted #%zu: A[s%d,l%d,c%d,o%d,TDC%d] B[s%d,l%d,c%d,o%d,TDC%d]\n",
                       i, ha[i][0],ha[i][1],ha[i][2],ha[i][3],ha[i][4],
                          hb[i][0],hb[i][1],hb[i][2],hb[i][3],hb[i][4]);
                break;
            }
            bad = true;
        }
        totURW += ua.size(); totHODO += ha.size();
        if (bad) mism++;
        nev++;
    }

    printf("\nCompared %ld events: %ld mismatching.  (URWELL rows=%ld, XYHODO rows=%ld)\n",
           nev, mism, totURW, totHODO);
    printf(mism == 0 ? "RESULT: IDENTICAL bank contents\n" : "RESULT: MISMATCH\n");
    return mism == 0 ? 0 : 1;
}
