//
// HV_Scan_SecondProtoDoubleHodo.cc
//
// Builds high-voltage dependence plots for the second prototype
// double-hodoscope runs.
//
// For a given scan "series" the runs and their HV settings are listed in
//   SecondProtoHVScan_<Series>.dat
// with the columns
//   <Run>  <HV_MESH_Top>  <HV_MESH_Bot>  <HV_Cathode_Top>  <HV_Cathode_Bot>
//
// For every run the analysis file  AnaSecondHodoDoubleHodo_<Run>.root  is read
// and a set of observables is extracted from its histograms (the mean of a
// histogram, the peak-bin position, the MPV of a Landau fit, ...). Each
// observable is plotted as a function of the high voltage, where the meaning of
// the HV axis depends on the scan type:
//   MESH       : the (common) mesh HV            (x = HV_MESH_Top == HV_MESH_Bot)
//   Drift      : the (common) drift HV           (x = HV_Cathode_Top - HV_MESH_Top)
//   MESH_Top   : the top mesh HV                 (x = HV_MESH_Top)
//   MESH_Bot   : the bottom mesh HV              (x = HV_MESH_Bot)
//   Drift_Top  : the top drift HV                (x = HV_Cathode_Top - HV_MESH_Top)
//   Drift_Bot  : the bottom drift HV             (x = HV_Cathode_Bot - HV_MESH_Bot)
// where Drift HV = Cathode HV - MESH HV.
//
// Two kinds of observable are produced:
//   * "U/V" observables  : the "h_U..." histogram comes from the Top detector
//     and the "h_V..." from the Bottom detector. The U and V graphs of the same
//     observable are drawn together on one figure (TMultiGraph). Registered in
//     buildObservables().
//   * "combined" observables : a single quantity computed from several
//     histograms of the same file (e.g. an efficiency that is the ratio of two
//     histogram entry counts). Only one graph is drawn. Registered in
//     buildCombinedObservables().
//
// All figures are saved to the Figs/ sub-directory.
//
// Every histogram that an observable is extracted from is also collected into a
// single multi-page document  Figs/Distributions_HV_Scan_<Series>.pdf  (the
// histogram, its fit if any, and the extracted value) so the inputs of the HV
// scan can be inspected at a glance.
//
// Whenever a point can not be produced (the file/histogram is missing, the
// histogram is empty, or a fit does not converge) the point is dropped, the
// user is clearly notified on the spot and in a final summary, and -- for a
// failed fit -- a diagnostic figure of the histogram with the attempted fit is
// written to  Figs/diagnostic_<Variable>_<ScanType>_<HVValue>.pdf  so that the
// failure can be understood.
//
// The list of observables is built in buildObservables() / buildCombined
// Observables(); adding a new variable is just one more entry there.
//
// Usage:
//   ./HV_Scan_SecondProtoDoubleHodo.exe  <Series>  <ScanType>  [inputDir]
// e.g. (run from the install dir that holds the .root and .dat files):
//   ./HV_Scan_SecondProtoDoubleHodo.exe  1  MESH
//

#include <TCanvas.h>
#include <TF1.h>
#include <TFile.h>
#include <TGraph.h>
#include <TH1.h>
#include <TH2.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TMultiGraph.h>
#include <TROOT.h>
#include <TString.h>
#include <TStyle.h>
#include <TSystem.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// The maximum value of the (unnormalised) Landau density, reached at the MPV.
// A * Landau(x, MPV, sigma) therefore peaks at A * 0.180655, which is how the
// initial amplitude is chosen for the fits below.
static constexpr double kLandauPeak = 0.180655;

// ----------------------------------------------------------------------------
// Scan types
// ----------------------------------------------------------------------------
enum class ScanType { MESH, Drift, MESH_Top, MESH_Bot, Drift_Top, Drift_Bot };

bool parseScanType(const string &s, ScanType &out) {
    if (s == "MESH") { out = ScanType::MESH; return true; }
    if (s == "Drift") { out = ScanType::Drift; return true; }
    if (s == "MESH_Top") { out = ScanType::MESH_Top; return true; }
    if (s == "MESH_Bot") { out = ScanType::MESH_Bot; return true; }
    if (s == "Drift_Top") { out = ScanType::Drift_Top; return true; }
    if (s == "Drift_Bot") { out = ScanType::Drift_Bot; return true; }
    return false;
}

// ----------------------------------------------------------------------------
// One row of the SecondProtoHVScan_<Series>.dat file
// ----------------------------------------------------------------------------
struct RunInfo {
    int run;
    double hv_mesh_top;
    double hv_mesh_bot;
    double hv_cathode_top;
    double hv_cathode_bot;

    double hvAxis(ScanType type) const {
        switch (type) {
            case ScanType::MESH: return hv_mesh_top;
            case ScanType::MESH_Top: return hv_mesh_top;
            case ScanType::MESH_Bot: return hv_mesh_bot;
            case ScanType::Drift: return hv_cathode_top - hv_mesh_top;
            case ScanType::Drift_Top: return hv_cathode_top - hv_mesh_top;
            case ScanType::Drift_Bot: return hv_cathode_bot - hv_mesh_bot;
        }
        return 0.;
    }
};

string hvAxisTitle(ScanType type) {
    switch (type) {
        case ScanType::MESH: return "MESH HV [V]";
        case ScanType::MESH_Top: return "Top MESH HV [V]";
        case ScanType::MESH_Bot: return "Bottom MESH HV [V]";
        case ScanType::Drift: return "Drift HV [V]";
        case ScanType::Drift_Top: return "Top Drift HV [V]";
        case ScanType::Drift_Bot: return "Bottom Drift HV [V]";
    }
    return "HV [V]";
}

// ----------------------------------------------------------------------------
// Result of running an extractor on one histogram.
//   ok   : whether a usable value was produced
//   fit  : the attempted fit function (nullptr for non-fit extractors). It is
//          owned by the caller, which uses it for the diagnostic plot and then
//          deletes it.
// ----------------------------------------------------------------------------
struct ExtractResult {
    double value = nan("");
    bool ok = false;
    TF1 *fit = nullptr;
};

using Extractor = function<ExtractResult(TH1 *)>;

struct Observable {
    string tag; // short, file-name friendly id, e.g. "ClusterSize_Mean"
    string title; // axis / legend title, e.g. "Cluster size mean"
    string histU; // Top-detector histogram name
    string histV; // Bottom-detector histogram name
    Extractor extract;
};

// A "combined" observable: a single value computed from one or more histograms
// of the same file (so only one graph is drawn). The extractor is handed the
// open TFile. inputHists lists the histograms it uses; they are shown in the
// Distributions document so the inputs of the combined quantity stay inspectable.
using CombinedExtractor = function<ExtractResult(TFile &)>;

struct CombinedObservable {
    string tag; // short, file-name friendly id, e.g. "Efficiency"
    string title; // axis title, e.g. "Efficiency [%]"
    string graphLabel; // legend label of the single graph, e.g. "Efficiency"
    vector<string> inputHists; // histograms shown in the Distributions document
    CombinedExtractor extract;
};

// A point that could not be produced — collected for the final notification.
struct DroppedPoint {
    string variable; // e.g. "PulseHeight_LandauMPV_U"
    int run;
    double hvValue;
    string hvTitle;
    string reason;
    string diagFile; // diagnostic PDF, empty if none was produced
};

// ----------------------------------------------------------------------------
// A lazily-opened multi-page PDF. The document is only opened on the first
// page() call, so no empty file is produced if nothing is ever drawn. While a
// document is open NO other PDF must be written (ROOT keeps a single global
// PostScript stream), so the distributions document is produced on its own.
// ----------------------------------------------------------------------------
struct MultiPagePdf {
    TCanvas *c = nullptr;
    string name;
    bool opened = false;

    void page() {
        if (c == nullptr) {
            return;
        }
        if (!opened) {
            c->Print((name + "[").c_str());
            opened = true;
        }
        c->Print(name.c_str());
    }
    void close() {
        if (opened) {
            c->Print((name + "]").c_str());
        }
    }
};

// ---- Reusable extractors ---------------------------------------------------

// Mean of the histogram.
static ExtractResult extractMean(TH1 *h) {
    ExtractResult res;
    if (h != nullptr) {
        res.value = h->GetMean();
        res.ok = true;
    }
    return res;
}

// X position (bin centre) of the bin with the largest content.
static ExtractResult extractPeakBinCenter(TH1 *h) {
    ExtractResult res;
    if (h != nullptr) {
        res.value = h->GetXaxis()->GetBinCenter(h->GetMaximumBin());
        res.ok = true;
    }
    return res;
}

// MPV of an A*Landau(x, MPV, sigma) fit. When xLo < xHi the fit is restricted to
// that range, otherwise the full histogram range is used. Initial parameters
// follow the task description:
//   A     = h->GetMaximum() / 0.180655
//   MPV   = bin centre of the maximum bin
//   sigma = half of that bin centre
// The (attempted) fit function is always returned so that, on failure, the
// caller can draw a diagnostic of what went wrong.
static ExtractResult extractLandauMPV(TH1 *h, double xLo, double xHi) {
    ExtractResult res;
    if (h == nullptr) {
        return res;
    }

    const bool useRange = (xLo < xHi);
    const double peakX = h->GetXaxis()->GetBinCenter(h->GetMaximumBin());

    // Give the function an explicit range, otherwise a TF1 built from a formula
    // string defaults to the [0, 1] range and the curve drawn with "same" would
    // be an invisible sliver near zero.
    const double rLo = useRange ? xLo : h->GetXaxis()->GetXmin();
    const double rHi = useRange ? xHi : h->GetXaxis()->GetXmax();

    auto *f = new TF1("f_Landau", "[0]*TMath::Landau(x, [1], [2])", rLo, rHi);
    f->SetNpx(500);
    f->SetParameter(0, h->GetMaximum() / kLandauPeak);
    f->SetParameter(1, peakX);
    f->SetParameter(2, 0.5 * peakX);
    f->SetParLimits(2, 0, 20000);
    f->SetLineColor(kRed);
    f->SetLineWidth(2);

    const int status = useRange ? int(h->Fit(f, "QN", "", xLo, xHi))
                                : int(h->Fit(f, "QN"));

    res.fit = f;
    res.value = f->GetParameter(1);
    res.ok = (status == 0);
    return res;
}

// Efficiency, in percent: the ratio of the number of reconstructed crosses
// inside the fiducial region to the detector occupancy inside the same region,
//   eff [%] = 100 * h_Cross_YXc_MaxIntegral_Fiducial1->GetEntries()
//                 / h_Det0_Occupancy_Fiducial1->GetEntries()
static ExtractResult extractEfficiency(TFile &f) {
    ExtractResult res;
    TH1 *hNum = dynamic_cast<TH1 *>(f.Get("h_Cross_YXc_MaxIntegral_Fiducial1"));
    TH1 *hDen = dynamic_cast<TH1 *>(f.Get("h_Det0_Occupancy_Fiducial1"));
    if (hNum == nullptr || hDen == nullptr) {
        return res;
    }
    const double den = hDen->GetEntries();
    if (den <= 0.) {
        return res;
    }
    res.value = 100. * hNum->GetEntries() / den;
    res.ok = true;
    return res;
}

// ----------------------------------------------------------------------------
// Observable registry  —  ADD NEW U/V VARIABLES HERE
// ----------------------------------------------------------------------------
vector<Observable> buildObservables() {
    vector<Observable> obs;

    obs.push_back({"ClusterSize_Mean", "Cluster size mean",
                   "h_UCl_Size2", "h_VCl_Size2", extractMean});

    obs.push_back({"ClusterSize_PeakBin", "Cluster size peak position",
                   "h_UCl_Size2", "h_VCl_Size2", extractPeakBinCenter});

    obs.push_back({"PulseIntegral_LandauMPV", "Pulse integral Landau MPV",
                   "h_U_PulseIntegral2", "h_V_PulseIntegral2",
                   [](TH1 *h) { return extractLandauMPV(h, 0., -1.); }}); // full range

    obs.push_back({"PulseHeight_LandauMPV", "Pulse height Landau MPV",
                   "h_U_PulseHeight2", "h_V_PulseHeight2",
                   [](TH1 *h) { return extractLandauMPV(h, 0., 1000.); }});

    return obs;
}

// ----------------------------------------------------------------------------
// Combined observable registry  —  ADD NEW COMBINED VARIABLES HERE
// ----------------------------------------------------------------------------
vector<CombinedObservable> buildCombinedObservables() {
    vector<CombinedObservable> obs;

    obs.push_back({"Efficiency", "Efficiency [%]", "Efficiency",
                   {"h_Cross_YXc_MaxIntegral_Fiducial1", "h_Det0_Occupancy_Fiducial1"},
                   extractEfficiency});

    return obs;
}

// ----------------------------------------------------------------------------
// Read SecondProtoHVScan_<Series>.dat
// ----------------------------------------------------------------------------
vector<RunInfo> readSeries(const string &path) {
    vector<RunInfo> out;
    ifstream in(path);
    if (!in) {
        cerr << "Cannot open series file: " << path << endl;
        return out;
    }
    string line;
    while (getline(in, line)) {
        auto firstChar = line.find_first_not_of(" \t\r\n");
        if (firstChar == string::npos || line[firstChar] == '#') {
            continue;
        }
        stringstream ss(line);
        RunInfo r;
        if (ss >> r.run >> r.hv_mesh_top >> r.hv_mesh_bot >> r.hv_cathode_top >> r.hv_cathode_bot) {
            out.push_back(r);
        }
    }
    return out;
}

// Warn (but do not abort) if the series does not satisfy the symmetry that the
// requested scan type assumes.
void checkScanConsistency(const vector<RunInfo> &runs, ScanType type) {
    for (const auto &r : runs) {
        if (type == ScanType::MESH && r.hv_mesh_top != r.hv_mesh_bot) {
            cerr << "WARNING: run " << r.run << " has HV_MESH_Top != HV_MESH_Bot for a MESH scan."
                 << endl;
        }
        if (type == ScanType::Drift && r.hv_cathode_top != r.hv_cathode_bot) {
            cerr << "WARNING: run " << r.run
                 << " has HV_Cathode_Top != HV_Cathode_Bot for a Drift scan." << endl;
        }
    }
}

// Draw the histogram together with the (failed) fit and save it as a diagnostic
// PDF. Returns the file name that was written.
string writeFitDiagnostic(TCanvas *c, TH1 *h, TF1 *fit, const string &varLabel,
                          const string &scanTypeStr, double hvValue) {

    const string fileName = Form("Figs/diagnostic_%s_%s_%g.pdf",
                                 varLabel.c_str(), scanTypeStr.c_str(), hvValue);

    c->Clear();
    c->cd();
    h->Draw();
    if (fit != nullptr) {
        fit->Draw("same");
    }

    TLatex lat;
    lat.SetNDC();
    lat.SetTextColor(kRed + 1);
    lat.SetTextFont(62);
    lat.DrawLatex(0.13, 0.86, "FIT DID NOT CONVERGE");
    if (fit != nullptr) {
        lat.SetTextColor(kBlack);
        lat.SetTextFont(42);
        lat.SetTextSize(0.035);
        lat.DrawLatex(0.13, 0.81, Form("A = %.3g, MPV = %.3g, #sigma = %.3g",
                                       fit->GetParameter(0), fit->GetParameter(1),
                                       fit->GetParameter(2)));
        lat.DrawLatex(0.13, 0.76, Form("#chi^{2}/ndf = %.3g / %d",
                                       fit->GetChisquare(), fit->GetNDF()));
    }

    c->Update();
    c->Print(fileName.c_str());
    return fileName;
}

// Append one page to the "Distributions" document showing the histogram an
// extractor was run on, its fit (if any) and the extracted value.
void drawDistributionPage(MultiPagePdf &pdf, TH1 *h, const ExtractResult &res,
                          const string &dispTitle, const string &layerLabel,
                          int run, const string &hvTitle, double hv) {

    TCanvas *c = pdf.c;
    c->Clear();
    c->cd();

    h->SetTitle(Form("%s  [%s]   run %d,  %s = %g",
                     dispTitle.c_str(), layerLabel.c_str(), run, hvTitle.c_str(), hv));
    h->Draw();
    if (res.fit != nullptr) {
        res.fit->Draw("same");
    }

    TLatex lat;
    lat.SetNDC();
    lat.SetTextFont(42);
    lat.SetTextSize(0.035);

    if (res.ok) {
        lat.SetTextColor(kBlue + 2);
        lat.DrawLatex(0.55, 0.62, Form("extracted = %.4g", res.value));
    } else {
        lat.SetTextColor(kRed + 1);
        lat.SetTextFont(62);
        lat.DrawLatex(0.45, 0.66, "EXTRACTION FAILED");
        lat.SetTextFont(42);
    }
    if (res.fit != nullptr) {
        lat.SetTextColor(kBlack);
        lat.DrawLatex(0.55, 0.56, Form("A = %.3g, MPV = %.3g, #sigma = %.3g",
                                       res.fit->GetParameter(0), res.fit->GetParameter(1),
                                       res.fit->GetParameter(2)));
        lat.DrawLatex(0.55, 0.50, Form("#chi^{2}/ndf = %.3g / %d",
                                       res.fit->GetChisquare(), res.fit->GetNDF()));
    }

    c->Update();
    pdf.page();
}

// Append one page to the "Distributions" document showing an input histogram of
// a combined observable. The entry count (which is what the combined quantity is
// built from) is written on the page. 2-D maps are drawn with COLZ.
void drawCombinedInputPage(MultiPagePdf &pdf, TH1 *h, const string &dispTitle,
                           const string &histName, int run, const string &hvTitle,
                           double hv) {

    TCanvas *c = pdf.c;
    c->Clear();
    c->cd();

    h->SetTitle(Form("%s : %s   run %d,  %s = %g",
                     dispTitle.c_str(), histName.c_str(), run, hvTitle.c_str(), hv));
    if (dynamic_cast<TH2 *>(h) != nullptr) {
        h->Draw("COLZ");
    } else {
        h->Draw();
    }

    TLatex lat;
    lat.SetNDC();
    lat.SetTextFont(42);
    lat.SetTextSize(0.035);
    lat.SetTextColor(kBlue + 2);
    lat.DrawLatex(0.13, 0.92, Form("entries = %g", h->GetEntries()));

    c->Update();
    pdf.page();
}

// ----------------------------------------------------------------------------
// Collect every histogram an observable is extracted from into one multi-page
// PDF. This is done as its own phase (no other PDF open) to keep the ROOT PDF
// stream consistent.
// ----------------------------------------------------------------------------
void writeDistributions(const vector<RunInfo> &runs, const string &inputDir,
                        const vector<Observable> &observables,
                        const vector<CombinedObservable> &combinedObservables,
                        ScanType type, const string &distPdfName, TCanvas *cDist) {

    MultiPagePdf distPdf{cDist, distPdfName, false};
    const string hvTitle = hvAxisTitle(type);

    struct LayerHist {
        string hist;
        string label;
    };

    // ---- U/V observables ----
    for (const auto &obs : observables) {
        const LayerHist layers[2] = {{obs.histU, "U (Top)"}, {obs.histV, "V (Bottom)"}};
        for (const auto &lay : layers) {
            for (const auto &r : runs) {
                const double hv = r.hvAxis(type);
                const string fn = inputDir + "/AnaSecondHodoDoubleHodo_" + to_string(r.run) + ".root";
                TFile f(fn.c_str(), "READ");
                if (f.IsZombie()) {
                    continue;
                }
                TH1 *h = dynamic_cast<TH1 *>(f.Get(lay.hist.c_str()));
                if (h == nullptr || h->GetEntries() == 0) {
                    continue;
                }
                ExtractResult res = obs.extract(h);
                drawDistributionPage(distPdf, h, res, obs.title, lay.label, r.run, hvTitle, hv);
                delete res.fit;
            }
        }
    }

    // ---- combined observables: show their input histograms ----
    for (const auto &cobs : combinedObservables) {
        for (const auto &histName : cobs.inputHists) {
            for (const auto &r : runs) {
                const double hv = r.hvAxis(type);
                const string fn = inputDir + "/AnaSecondHodoDoubleHodo_" + to_string(r.run) + ".root";
                TFile f(fn.c_str(), "READ");
                if (f.IsZombie()) {
                    continue;
                }
                TH1 *h = dynamic_cast<TH1 *>(f.Get(histName.c_str()));
                if (h == nullptr) {
                    continue;
                }
                drawCombinedInputPage(distPdf, h, cobs.title, histName, r.run, hvTitle, hv);
            }
        }
    }

    distPdf.close();
    if (distPdf.opened) {
        cout << "Wrote " << distPdfName << "  (all extraction input distributions)" << endl;
    }
}

// ----------------------------------------------------------------------------
// Build a graph of one extractor over all runs, for a single histogram name.
// Dropped points are appended to "dropped" and, for failed fits, a diagnostic
// PDF is written using cDiag.
// ----------------------------------------------------------------------------
TGraph *buildGraph(const vector<RunInfo> &runs, const string &inputDir,
                   const string &histName, const Extractor &extract, ScanType type,
                   const string &scanTypeStr, const string &varLabel,
                   TCanvas *cDiag, vector<DroppedPoint> &dropped) {

    const string hvTitle = hvAxisTitle(type);

    vector<double> xs;
    vector<double> ys;

    for (const auto &r : runs) {
        const double hv = r.hvAxis(type);

        auto drop = [&](const string &reason, const string &diag = "") {
            cerr << "  *** DROPPED: " << varLabel << "  run " << r.run << "  "
                 << hvTitle << " = " << hv << "  -- " << reason;
            if (!diag.empty()) {
                cerr << "  (diagnostic: " << diag << ")";
            }
            cerr << endl;
            dropped.push_back({varLabel, r.run, hv, hvTitle, reason, diag});
        };

        const string fn = inputDir + "/AnaSecondHodoDoubleHodo_" + to_string(r.run) + ".root";
        TFile f(fn.c_str(), "READ");
        if (f.IsZombie()) {
            drop("can not open ROOT file " + fn);
            continue;
        }

        TH1 *h = dynamic_cast<TH1 *>(f.Get(histName.c_str()));
        if (h == nullptr) {
            drop("histogram '" + histName + "' not found");
            continue;
        }
        if (h->GetEntries() == 0) {
            drop("histogram '" + histName + "' is empty");
            continue;
        }

        ExtractResult res = extract(h);

        if (!res.ok) {
            string diag;
            if (res.fit != nullptr) {
                diag = writeFitDiagnostic(cDiag, h, res.fit, varLabel, scanTypeStr, hv);
            }
            drop(res.fit != nullptr ? "fit did not converge" : "extractor failed", diag);
        } else {
            xs.push_back(hv);
            ys.push_back(res.value);
        }

        delete res.fit;
    }

    if (xs.empty()) {
        return nullptr;
    }
    return new TGraph(int(xs.size()), xs.data(), ys.data());
}

// ----------------------------------------------------------------------------
// Build the single graph of a combined observable over all runs. The extractor
// is handed the whole file. Dropped points are appended to "dropped".
// ----------------------------------------------------------------------------
TGraph *buildCombinedGraph(const vector<RunInfo> &runs, const string &inputDir,
                           const CombinedExtractor &extract, ScanType type,
                           const string &varLabel, vector<DroppedPoint> &dropped) {

    const string hvTitle = hvAxisTitle(type);

    vector<double> xs;
    vector<double> ys;

    for (const auto &r : runs) {
        const double hv = r.hvAxis(type);

        auto drop = [&](const string &reason) {
            cerr << "  *** DROPPED: " << varLabel << "  run " << r.run << "  "
                 << hvTitle << " = " << hv << "  -- " << reason << endl;
            dropped.push_back({varLabel, r.run, hv, hvTitle, reason, ""});
        };

        const string fn = inputDir + "/AnaSecondHodoDoubleHodo_" + to_string(r.run) + ".root";
        TFile f(fn.c_str(), "READ");
        if (f.IsZombie()) {
            drop("can not open ROOT file " + fn);
            continue;
        }

        ExtractResult res = extract(f);
        if (!res.ok) {
            drop("could not compute " + varLabel +
                 " (missing input histogram or zero denominator)");
            continue;
        }

        xs.push_back(hv);
        ys.push_back(res.value);
    }

    if (xs.empty()) {
        return nullptr;
    }
    return new TGraph(int(xs.size()), xs.data(), ys.data());
}

// ----------------------------------------------------------------------------
// Main
// ----------------------------------------------------------------------------
int main(int argc, char *argv[]) {

    if (argc < 3) {
        cerr << "Usage: ./HV_Scan_SecondProtoDoubleHodo.exe  <Series>  <ScanType>  [inputDir]" << endl;
        cerr << "  ScanType = MESH | Drift | MESH_Top | MESH_Bot | Drift_Top | Drift_Bot" << endl;
        cerr << "  e.g.   ./HV_Scan_SecondProtoDoubleHodo.exe  1  MESH" << endl;
        return 1;
    }

    const int series = atoi(argv[1]);
    const string scanTypeStr = argv[2];
    const string inputDir = (argc >= 4) ? argv[3] : ".";

    ScanType scanType;
    if (!parseScanType(scanTypeStr, scanType)) {
        cerr << "Unknown scan type '" << scanTypeStr << "'." << endl;
        cerr << "  ScanType = MESH | Drift | MESH_Top | MESH_Bot | Drift_Top | Drift_Bot" << endl;
        return 1;
    }

    const string seriesFile = inputDir + "/SecondProtoHVScan_" + to_string(series) + ".dat";
    const vector<RunInfo> runs = readSeries(seriesFile);
    if (runs.empty()) {
        cerr << "No runs found in " << seriesFile << endl;
        return 1;
    }
    checkScanConsistency(runs, scanType);

    gROOT->SetBatch(kTRUE);
    gStyle->SetOptStat(1110);
    gSystem->Exec("mkdir -p Figs");

    const vector<Observable> observables = buildObservables();
    const vector<CombinedObservable> combinedObservables = buildCombinedObservables();
    const string xTitle = hvAxisTitle(scanType);

    auto *c1 = new TCanvas("c1", "", 1000, 700);
    auto *cDiag = new TCanvas("cDiag", "", 1000, 700);
    auto *cDist = new TCanvas("cDist", "", 1000, 700);

    vector<DroppedPoint> dropped;

    // ------------------------------------------------------------------
    // Phase 1a: build the U/V HV-dependence graphs (and the per-fit
    // diagnostics).
    // ------------------------------------------------------------------
    for (const auto &obs : observables) {

        TGraph *gU = buildGraph(runs, inputDir, obs.histU, obs.extract, scanType,
                                scanTypeStr, obs.tag + "_U", cDiag, dropped);
        TGraph *gV = buildGraph(runs, inputDir, obs.histV, obs.extract, scanType,
                                scanTypeStr, obs.tag + "_V", cDiag, dropped);

        if (gU == nullptr && gV == nullptr) {
            cerr << "No valid points for observable " << obs.tag << " — skipping." << endl;
            continue;
        }

        auto *mg = new TMultiGraph();
        mg->SetTitle(Form("%s;%s;%s", obs.title.c_str(), xTitle.c_str(), obs.title.c_str()));

        auto *leg = new TLegend(0.70, 0.78, 0.89, 0.89);
        leg->SetBorderSize(0);
        leg->SetFillStyle(0);

        if (gU != nullptr) {
            gU->SetMarkerStyle(20);
            gU->SetMarkerColor(kRed);
            gU->SetLineColor(kRed);
            gU->SetMarkerSize(1.2);
            mg->Add(gU, "PL");
            leg->AddEntry(gU, "U (Top)", "pl");
        }
        if (gV != nullptr) {
            gV->SetMarkerStyle(21);
            gV->SetMarkerColor(kBlue);
            gV->SetLineColor(kBlue);
            gV->SetMarkerSize(1.2);
            mg->Add(gV, "PL");
            leg->AddEntry(gV, "V (Bottom)", "pl");
        }

        c1->Clear();
        c1->cd();
        mg->Draw("A");
        leg->Draw();
        c1->Update();

        const string base = "Figs/HVScan_" + obs.tag + "_" + scanTypeStr
                            + "_Series" + to_string(series);
        c1->Print((base + ".pdf").c_str());
        c1->Print((base + ".png").c_str());
        c1->Print((base + ".root").c_str());

        cout << "Wrote " << base << ".{pdf,png,root}" << endl;

        delete mg; // also deletes the graphs it owns
        delete leg;
    }

    // ------------------------------------------------------------------
    // Phase 1b: build the combined HV-dependence graphs (one graph each).
    // ------------------------------------------------------------------
    for (const auto &cobs : combinedObservables) {

        TGraph *g = buildCombinedGraph(runs, inputDir, cobs.extract, scanType,
                                       cobs.tag, dropped);
        if (g == nullptr) {
            cerr << "No valid points for observable " << cobs.tag << " — skipping." << endl;
            continue;
        }

        g->SetTitle(Form("%s;%s;%s", cobs.title.c_str(), xTitle.c_str(), cobs.title.c_str()));
        g->SetMarkerStyle(20);
        g->SetMarkerColor(kBlue);
        g->SetLineColor(kBlue);
        g->SetMarkerSize(1.2);

        auto *leg = new TLegend(0.24, 0.78, 0.35, 0.89);
        leg->SetBorderSize(0);
        leg->SetFillStyle(0);
        leg->AddEntry(g, cobs.graphLabel.c_str(), "pl");

        c1->Clear();
        c1->cd();
        g->Draw("APL");
        leg->Draw();
        c1->Update();

        const string base = "Figs/HVScan_" + cobs.tag + "_" + scanTypeStr
                            + "_Series" + to_string(series);
        c1->Print((base + ".pdf").c_str());
        c1->Print((base + ".png").c_str());
        c1->Print((base + ".root").c_str());

        cout << "Wrote " << base << ".{pdf,png,root}" << endl;

        delete g;
        delete leg;
    }

    // ------------------------------------------------------------------
    // Phase 2: collect all the input distributions into one multi-page PDF.
    // Kept separate so no other PDF is open while it is being written.
    // ------------------------------------------------------------------
    const string distPdfName = "Figs/Distributions_HV_Scan_" + to_string(series) + ".pdf";
    writeDistributions(runs, inputDir, observables, combinedObservables, scanType,
                       distPdfName, cDist);

    // ------------------------------------------------------------------
    // Final, clearly-visible notification about the dropped points.
    // ------------------------------------------------------------------
    cout << "\n========================================================\n";
    if (dropped.empty()) {
        cout << "  All requested points were produced — none dropped.\n";
    } else {
        cout << "  DROPPED POINTS: " << dropped.size() << " point(s) were NOT plotted\n";
        cout << "========================================================\n";
        for (const auto &d : dropped) {
            cout << "  - " << d.variable << "  run " << d.run << "  "
                 << d.hvTitle << " = " << d.hvValue << "  : " << d.reason;
            if (!d.diagFile.empty()) {
                cout << "  -> " << d.diagFile;
            }
            cout << "\n";
        }
    }
    cout << "========================================================\n";

    return 0;
}
