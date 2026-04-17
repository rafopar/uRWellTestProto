// HVScanAnalyzer.cc
//
// Usage:
//   ./HVScanAnalyzer  <config.cfg>  <Series_N.txt>  <MESH|DRIFT>  [inputDir]
//
// Example:
//   ./HVScanAnalyzer  config.cfg  Series_1.txt  MESH  ./
//
// Input files must be named  AnaPulseFits_<RUN>.root  and live in <inputDir>.
// Output PDFs are written to  Figs/<extractor>_<histoType>_<HVmode>_Scan_<SeriesTag>.pdf
//
// One pixel per PDF page. Pixels with fewer than 2 valid points are skipped.

#include <TFile.h>
#include <TH1.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TF1.h>
#include <TGraph.h>
#include <TCanvas.h>
#include <TSystem.h>
#include <TString.h>
#include <TROOT.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <algorithm>
#include <cmath>

// ----------------------------------------------------------------------------
// Geometry — hardcoded as requested
// ----------------------------------------------------------------------------
constexpr int    n_Xpixels = 20;
constexpr int    n_Ypixels = 10;
constexpr double l_drift   = 0.4;  // cm (for DRIFT field calculation)

// ----------------------------------------------------------------------------
// Types
// ----------------------------------------------------------------------------
struct RunInfo {
    int    run;
    double mesh_HV;      // V
    double cathode_HV;   // V
    double drift_field() const { return (cathode_HV - mesh_HV) / l_drift; }
};

struct HistoTypeCfg {
    std::string name;           // section name, e.g. "h_U_clSize"
    std::string pattern;        // e.g. "h_U_clSize_%d_%d"
    std::string className;      // "TH1D" or "TH2D"
    std::vector<std::string> extractors;
};

// An extractor takes a histogram and returns a value (NaN if it cannot be computed)
using Extractor = std::function<double(TH1*)>;

// ----------------------------------------------------------------------------
// Extractor registry  —  ADD NEW EXTRACTORS HERE
// ----------------------------------------------------------------------------
std::map<std::string, Extractor> buildExtractorRegistry() {
    std::map<std::string, Extractor> R;

    R["mean"]    = [](TH1* h){ return h ? h->GetMean()   : std::nan(""); };
    R["rms"]     = [](TH1* h){ return h ? h->GetRMS()    : std::nan(""); };
    R["entries"] = [](TH1* h){ return h ? (double)h->GetEntries() : std::nan(""); };
    R["peakPos"] = [](TH1* h){
        if (!h) return std::nan("");
        int bin = h->GetMaximumBin();
        return h->GetXaxis()->GetBinCenter(bin);
    };

    // Gaussian fit extractors — fit once, cached by histogram pointer wouldn't be trivial,
    // so we just refit each call. For typical dataset sizes this is fine.
    auto gausFit = [](TH1* h, int param) -> double {
        if (!h || h->GetEntries() < 10) return std::nan("");
        // silent fit, in range [mean-2*rms, mean+2*rms]
        double m = h->GetMean(), s = h->GetRMS();
        if (s <= 0.) return std::nan("");
        TF1 f("gfit", "gaus", m - 2*s, m + 2*s);
        f.SetParameters(h->GetMaximum(), m, s);
        int status = h->Fit(&f, "QNR");
        if (status != 0) return std::nan("");
        return f.GetParameter(param);  // 0=amp, 1=mean, 2=sigma
    };
    R["gausAmp"]   = [gausFit](TH1* h){ return gausFit(h, 0); };
    R["gausMean"]  = [gausFit](TH1* h){ return gausFit(h, 1); };
    R["gausSigma"] = [gausFit](TH1* h){ return gausFit(h, 2); };

    return R;
}

// ----------------------------------------------------------------------------
// Config file parser  (simple INI-style)
// ----------------------------------------------------------------------------
std::vector<HistoTypeCfg> readConfig(const std::string &path) {
    std::vector<HistoTypeCfg> out;
    std::ifstream in(path);
    if (!in) { std::cerr << "Cannot open config: " << path << std::endl; return out; }

    HistoTypeCfg cur;
    bool have = false;
    std::string line;
    while (std::getline(in, line)) {
        // strip leading/trailing whitespace
        auto lws = line.find_first_not_of(" \t\r\n");
        if (lws == std::string::npos) continue;
        line = line.substr(lws);
        auto rws = line.find_last_not_of(" \t\r\n");
        line = line.substr(0, rws + 1);
        if (line.empty() || line[0] == '#') continue;

        if (line.front() == '[' && line.back() == ']') {
            if (have) out.push_back(cur);
            cur = HistoTypeCfg();
            cur.name = line.substr(1, line.size() - 2);
            have = true;
            continue;
        }

        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        // trim
        auto trim = [](std::string &s){
            auto a = s.find_first_not_of(" \t"); if (a == std::string::npos) { s.clear(); return; }
            auto b = s.find_last_not_of(" \t");
            s = s.substr(a, b - a + 1);
        };
        trim(key); trim(val);

        if      (key == "pattern")    cur.pattern = val;
        else if (key == "class")      cur.className = val;
        else if (key == "extractors") {
            std::stringstream ss(val);
            std::string tok;
            while (std::getline(ss, tok, ',')) { trim(tok); if (!tok.empty()) cur.extractors.push_back(tok); }
        }
    }
    if (have) out.push_back(cur);
    return out;
}

// ----------------------------------------------------------------------------
// Series file parser
// ----------------------------------------------------------------------------
std::vector<RunInfo> readSeries(const std::string &path) {
    std::vector<RunInfo> out;
    std::ifstream in(path);
    if (!in) { std::cerr << "Cannot open series file: " << path << std::endl; return out; }
    std::string line;
    while (std::getline(in, line)) {
        auto a = line.find_first_not_of(" \t\r\n");
        if (a == std::string::npos) continue;
        if (line[a] == '#') continue;
        std::stringstream ss(line);
        RunInfo r;
        if (ss >> r.run >> r.mesh_HV >> r.cathode_HV) out.push_back(r);
    }
    return out;
}

// Extract tag from a series filename — e.g. "Series_1.txt" -> "Series_1"
std::string seriesTag(const std::string &path) {
    auto slash = path.find_last_of("/\\");
    std::string base = (slash == std::string::npos) ? path : path.substr(slash + 1);
    auto dot = base.find_last_of('.');
    if (dot != std::string::npos) base = base.substr(0, dot);
    return base;
}

// ----------------------------------------------------------------------------
// Main
// ----------------------------------------------------------------------------
int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0]
                  << "  <config.cfg>  <Series_N.txt>  <MESH|DRIFT>  [inputDir]" << std::endl;
        return 1;
    }
    std::string cfgPath    = argv[1];
    std::string seriesPath = argv[2];
    std::string hvMode     = argv[3];
    std::string inputDir   = (argc >= 5) ? argv[4] : ".";

    if (hvMode != "MESH" && hvMode != "DRIFT") {
        std::cerr << "HV mode must be MESH or DRIFT (got: " << hvMode << ")" << std::endl;
        return 1;
    }

    auto histoTypes = readConfig(cfgPath);
    auto runs       = readSeries(seriesPath);
    auto registry   = buildExtractorRegistry();
    std::string tag = seriesTag(seriesPath);

    if (histoTypes.empty()) { std::cerr << "No histogram types in config." << std::endl; return 1; }
    if (runs.empty())       { std::cerr << "No runs in series file."       << std::endl; return 1; }

    // Ensure output dir exists
    gSystem->Exec("mkdir -p Figs");

    // Batch mode — no X windows
    gROOT->SetBatch(kTRUE);

    // ------------------------------------------------------------------
    // Open all files up front, keep them open for the whole run
    // ------------------------------------------------------------------
    std::map<int, TFile*> files;
    for (auto &r : runs) {
        std::string fn = inputDir + "/AnaPulseFits_" + std::to_string(r.run) + ".root";
        TFile *f = TFile::Open(fn.c_str(), "READ");
        if (!f || f->IsZombie()) {
            std::cerr << "WARNING: cannot open " << fn << " — skipping run " << r.run << std::endl;
            if (f) delete f;
            continue;
        }
        files[r.run] = f;
    }

    // Convenience: x-axis value for a run
    auto xValue = [&](const RunInfo &r) -> double {
        return (hvMode == "MESH") ? r.mesh_HV : r.drift_field();
    };

    std::string xTitle = (hvMode == "MESH")
                         ? "MESH HV [V]"
                         : "Drift field [V/cm]";

    // ------------------------------------------------------------------
    // Main loop
    // ------------------------------------------------------------------
    for (const auto &ht : histoTypes) {
        for (const auto &extName : ht.extractors) {
            auto itExt = registry.find(extName);
            if (itExt == registry.end()) {
                std::cerr << "WARNING: unknown extractor '" << extName
                          << "' for " << ht.name << " — skipping." << std::endl;
                continue;
            }
            Extractor extract = itExt->second;

            // Build PDF filename
            std::string pdfName = "Figs/" + extName + "_" + ht.name
                                + "_" + hvMode + "_Scan_" + tag + ".pdf";

            TCanvas c("c", "c", 800, 600);

            // First, count how many pixels have enough points — needed for "[" / "]" logic
            // We'll do a single pass and open/close the PDF at the right moments.
            bool pdfOpen = false;
            int nPages = 0;

            for (int ix = 0; ix < n_Xpixels; ix++) {
                for (int iy = 0; iy < n_Ypixels; iy++) {
                    std::string hname = Form(ht.pattern.c_str(), ix, iy);

                    std::vector<double> xs, ys;
                    xs.reserve(runs.size());
                    ys.reserve(runs.size());

                    for (const auto &r : runs) {
                        auto itF = files.find(r.run);
                        if (itF == files.end()) continue;
                        TH1 *h = dynamic_cast<TH1*>(itF->second->Get(hname.c_str()));
                        if (!h) continue;
                        if (h->GetEntries() == 0) continue;  // empty -> skip this point

                        double y = extract(h);
                        if (std::isnan(y)) continue;

                        xs.push_back(xValue(r));
                        ys.push_back(y);
                    }

                    if (xs.size() < 2) continue;  // not enough points to make a graph

                    TGraph g((int)xs.size(), xs.data(), ys.data());
                    g.SetTitle(Form("%s  pixel (%d,%d);%s;%s",
                                    ht.name.c_str(), ix, iy,
                                    xTitle.c_str(), extName.c_str()));
                    g.SetMarkerStyle(20);
                    g.SetMarkerSize(1.0);
                    g.SetLineWidth(1);

                    c.cd();
                    c.Clear();
                    g.Draw("APL");

                    // Open PDF on first successful page
                    if (!pdfOpen) {
                        c.Print((pdfName + "[").c_str());
                        pdfOpen = true;
                    }
                    c.Print(pdfName.c_str());
                    nPages++;
                }
            }

            if (pdfOpen) {
                c.Print((pdfName + "]").c_str());
                std::cout << "Wrote " << pdfName << "  (" << nPages << " pages)" << std::endl;
            } else {
                std::cout << "No valid pixels for " << ht.name << " / " << extName
                          << " — no PDF written." << std::endl;
            }
        }
    }

    // Cleanup
    for (auto &kv : files) { kv.second->Close(); delete kv.second; }

    return 0;
}
