/**
 * drawEff_HVScans.cc
 *
 * Calculates pixel-by-pixel detector efficiencies as a function of HV
 * (MESH or Drift) from a set of ROOT files "AnaPulseFits_<RUN>.root".
 *
 * Compile:
 *   g++ -o drawEff_HVScans drawEff_HVScans.cc \
 *       $(root-config --cflags --libs)
 *
 * Usage:
 *   ./drawEff_HVScans MESH  Scan_Settings_1.dat
 *   ./drawEff_HVScans DRIFT Scan_Settings_1.dat
 *
 * Input .dat file format (columns: Run  MESH_HV  Drift_HV):
 *   # comment lines start with #
 *   3067   560   200
 *   3068   560   220
 *   3069   560   240
 *
 * For MESH scan:  X axis = MESH_HV  [V]
 * For DRIFT scan: X axis = Drift Field = (Drift_HV - MESH_HV) / l_drift  [V/cm]
 *
 * Output:
 *   HVScan_<TYPE>_runs<FIRST>-<LAST>.pdf
 *   One page per pixel (ilongBar, ishortBar) that has at least one data point.
 *
 * Histogram naming in AnaPulseFits_<RUN>.root (confirmed from f->ls()):
 *   Numerator:   h_Cross_YXC_Max1_<ishortBar>_<ilongBar>   [TH2D, top-level]
 *   Denominator: h_Hodo_XY_BarID_Tag1                       [TH2D, top-level]
 *
 * Pixel ranges:
 *   ilongBar  : 0 ... 11  (N_LONG  = 12)   -- second index in histogram name
 *   ishortBar : 0 ... 32  (N_SHORT = 33)   -- first  index in histogram name
 */

#include <TFile.h>
#include <TH2D.h>
#include <TGraph.h>
#include <TCanvas.h>
#include <TLatex.h>
#include <TStyle.h>
#include <TAxis.h>

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>
#include <cctype>
#include <cstdlib>   // std::exit

// ── Detector geometry & pixel ranges ─────────────────────────────────────────
static const int    N_LONG   = 12;    // ilongBar  : 0 … 11
static const int    N_SHORT  = 33;    // ishortBar : 0 … 32
static const int    MAX_RUNS = 200;
static const double L_DRIFT  = 0.4;  // drift gap [cm]

// ── Helpers ───────────────────────────────────────────────────────────────────
std::string toUpper(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::toupper(c); });
    return s;
}

// Histogram name: h_Cross_YXC_Max1_<ishortBar>_<ilongBar>
std::string crossHistName(int ishortBar, int ilongBar)
{
    std::ostringstream oss;
    oss << "h_Cross_YXC_Max1_" << ishortBar << "_" << ilongBar;
    return oss.str();
}

// ── Run record ────────────────────────────────────────────────────────────────
struct RunInfo {
    int    run;
    double meshHV;    // V
    double driftHV;   // V
};

// ── Read .dat file ────────────────────────────────────────────────────────────
std::vector<RunInfo> readRunFile(const std::string& path)
{
    std::vector<RunInfo> out;
    std::ifstream fin(path);
    if (!fin.is_open()) {
        std::cerr << "[ERROR] Cannot open run file: " << path << std::endl;
        std::exit(1);
    }
    std::string line;
    int lineNo = 0;
    while (std::getline(fin, line)) {
        ++lineNo;
        // strip leading whitespace
        size_t first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) continue;
        if (line[first] == '#') continue;
        if (line.size() >= 2 && line[first] == '/' && line[first+1] == '/') continue;

        std::istringstream iss(line);
        RunInfo ri;
        if (!(iss >> ri.run >> ri.meshHV >> ri.driftHV)) {
            std::cerr << "[WARNING] Malformed line " << lineNo
                      << " in " << path << ": \"" << line << "\"" << std::endl;
            continue;
        }
        out.push_back(ri);
    }
    return out;
}

// ── Main ──────────────────────────────────────────────────────────────────────
int main(int argc, char* argv[])
{
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <MESH|DRIFT> <run_settings.dat>\n"
                  << "  MESH  scan: X axis = MESH_HV [V]\n"
                  << "  DRIFT scan: X axis = Drift Field = (Drift_HV - MESH_HV) / "
                  << L_DRIFT << " cm  [V/cm]\n";
        return 1;
    }

    // Normalise scan type
    std::string scanType = toUpper(std::string(argv[1]));
    if (scanType != "MESH" && scanType != "DRIFT") {
        std::cerr << "[ERROR] scanType must be MESH or DRIFT. Got: "
                  << argv[1] << std::endl;
        return 1;
    }
    const bool isDrift = (scanType == "DRIFT");

    // Read run list
    std::vector<RunInfo> runs = readRunFile(argv[2]);
    if (runs.empty()) {
        std::cerr << "[ERROR] No valid runs found in " << argv[2] << std::endl;
        return 1;
    }
    const int nRuns = (int)runs.size();
    if (nRuns > MAX_RUNS) {
        std::cerr << "[ERROR] nRuns=" << nRuns << " exceeds MAX_RUNS=" << MAX_RUNS
                  << ". Increase MAX_RUNS in the source." << std::endl;
        return 1;
    }

    // Sort by the X-axis quantity
    std::sort(runs.begin(), runs.end(), [&](const RunInfo& a, const RunInfo& b){
        if (isDrift)
            return (a.driftHV - a.meshHV) < (b.driftHV - b.meshHV);
        else
            return a.meshHV < b.meshHV;
    });

    // Build the X values
    std::vector<double> xVals(nRuns);
    for (int i = 0; i < nRuns; ++i) {
        if (isDrift)
            xVals[i] = (runs[i].driftHV - runs[i].meshHV) / L_DRIFT;
        else
            xVals[i] = runs[i].meshHV;
    }

    std::string xTitle = isDrift ? "Drift Field (V/cm)" : "MESH HV (V)";

    std::cout << "[INFO] Loaded " << nRuns << " run(s) for "
              << scanType << " scan." << std::endl;
    std::cout << "[INFO] X axis: " << xTitle << std::endl;

    // Print run table for reference
    std::cout << std::setw(8)  << "Run"
              << std::setw(12) << "MESH_HV(V)"
              << std::setw(14) << "Drift_HV(V)"
              << std::setw(18) << xTitle << "\n";
    for (int i = 0; i < nRuns; ++i) {
        std::cout << std::setw(8)  << runs[i].run
                  << std::setw(12) << runs[i].meshHV
                  << std::setw(14) << runs[i].driftHV
                  << std::setw(18) << xVals[i] << "\n";
    }

    // Output PDF name
    int runMin = runs[0].run, runMax = runs[0].run;
    for (auto& r : runs) {
        runMin = std::min(runMin, r.run);
        runMax = std::max(runMax, r.run);
    }
    std::ostringstream pdfss;
    pdfss << "Figs/HVScan_" << scanType << "_runs" << runMin << "-" << runMax << ".pdf";
    std::string outPDF = pdfss.str();

    // ── Efficiency array [ilongBar][ishortBar][iRun],  -1 = no data ──────────
    // Using heap allocation because N_LONG * N_SHORT * MAX_RUNS can be large
    // Layout: eff[l][s][r]
    std::vector<double> eff(N_LONG * N_SHORT * MAX_RUNS, -1.0);
    auto EFF = [&](int l, int s, int r) -> double& {
        return eff[l * N_SHORT * MAX_RUNS + s * MAX_RUNS + r];
    };

    // ── Loop over runs, fill efficiency ──────────────────────────────────────
    for (int iRun = 0; iRun < nRuns; ++iRun) {
        std::ostringstream fname;
        fname << "AnaPulseFits_" << runs[iRun].run << ".root";

        TFile* f = TFile::Open(fname.str().c_str(), "READ");
        if (!f || f->IsZombie()) {
            std::cerr << "[WARNING] Cannot open " << fname.str()
                      << " -- skipping run " << runs[iRun].run << std::endl;
            if (f) { f->Close(); delete f; }
            continue;
        }

        TH2D* hHodo = dynamic_cast<TH2D*>(f->Get("h_Hodo_XY_BarID_Tag1"));
        if (!hHodo) {
            std::cerr << "[WARNING] h_Hodo_XY_BarID_Tag1 not found in "
                      << fname.str() << " -- skipping run "
                      << runs[iRun].run << std::endl;
            f->Close(); delete f;
            continue;
        }

        for (int l = 0; l < N_LONG; ++l) {
            for (int s = 0; s < N_SHORT; ++s) {

                // Denominator: hodoscope tag counts
                // X axis = ishortBar, Y axis = ilongBar
                int    binX  = hHodo->GetXaxis()->FindBin(s);
                int    binY  = hHodo->GetYaxis()->FindBin(l);
                double denom = hHodo->GetBinContent(binX, binY);
                if (denom <= 0.0) continue;

                // Numerator: h_Cross_YXC_Max1_<ishortBar>_<ilongBar>
                std::string hname = crossHistName(s, l);
                TH2D* hCross = dynamic_cast<TH2D*>(f->Get(hname.c_str()));
                if (!hCross) continue;

                EFF(l, s, iRun) = hCross->Integral() / denom;
            }
        }

        f->Close(); delete f;
        std::cout << "[INFO] Processed run " << runs[iRun].run
                  << "  (MESH=" << runs[iRun].meshHV
                  << " V, Drift=" << runs[iRun].driftHV
                  << " V, " << xTitle << "=" << xVals[iRun] << ")" << std::endl;
    }

    // ── Draw and write PDF ────────────────────────────────────────────────────
    gStyle->SetOptStat(0);
    gStyle->SetPadGridX(kTRUE);
    gStyle->SetPadGridY(kTRUE);
    gStyle->SetPadLeftMargin(0.13);
    gStyle->SetPadBottomMargin(0.13);

    TCanvas* c = new TCanvas("cEff", "Efficiency vs HV", 800, 600);
    c->Print((outPDF + "[").c_str());

    int nPages = 0;

    for (int l = 0; l < N_LONG; ++l) {
        for (int s = 0; s < N_SHORT; ++s) {

            std::vector<double> xv, yv;
            for (int r = 0; r < nRuns; ++r) {
                if (EFF(l, s, r) >= 0.0) {
                    xv.push_back(xVals[r]);
                    yv.push_back(EFF(l, s, r));
                }
            }
            if (xv.empty()) continue;

            TGraph* gr = new TGraph((int)xv.size(), xv.data(), yv.data());
            gr->SetMarkerStyle(20);
            gr->SetMarkerSize(1.3);
            gr->SetMarkerColor(kBlue + 1);
            gr->SetLineColor(kBlue + 1);
            gr->SetLineWidth(2);

            std::ostringstream title;
            title << scanType << " Scan  |  ilongBar=" << l
                  << "  ishortBar=" << s;
            gr->SetTitle(title.str().c_str());
            gr->GetXaxis()->SetTitle(xTitle.c_str());
            gr->GetXaxis()->SetTitleSize(0.05);
            gr->GetYaxis()->SetTitle("Efficiency");
            gr->GetYaxis()->SetTitleSize(0.05);
            gr->GetYaxis()->SetRangeUser(0.0, 1.15);

            c->cd(); c->Clear();
            gr->Draw("APL");
            c->Print(outPDF.c_str());
            delete gr;
            ++nPages;
        }
    }

    c->Print((outPDF + "]").c_str());
    delete c;

    std::cout << "\n[DONE] " << nPages << " page(s) written to: "
              << outPDF << std::endl;
    return 0;
}
