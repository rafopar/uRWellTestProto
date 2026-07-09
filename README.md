This git repository contains analysis codes/tools for the data collected using the uRwell test prototype.

Check the link https://clasweb.jlab.org/wiki/index.php/Test_Proto_Documents for more details on the uRwell Test Prototype and also
for instructions on Software installation and running.

---

# Table of Contents

1. [Project Overview](#project-overview)
2. [Project Structure](#project-structure)
3. [Installation](#installation)
4. [Analysis Workflow](#analysis-workflow)
   - [Pedestal Runs](#pedestal-runs)
   - [Decoding](#decoding)
   - [Skimming & Pulse Fitting](#skimming--pulse-fitting)
   - [Analysis: AnaPulseFits](#analysis-anapulsefits)
5. [Standalone C++ Decoder (`Decoder/`)](#standalone-c-decoder-decoder)
6. [Automated Running](#automated-running)
7. [Plotting & Visualization Scripts](#plotting--visualization-scripts)
8. [Geometry Utilities in uRwellTools](#geometry-utilities-in-urwelltools)
9. [Key Analysis Parameters](#key-analysis-parameters)
10. [Data File Locations](#data-file-locations)

---

# Project Overview

This project provides a complete data analysis chain for the μRwell (microRwell) test prototype detector,
a micro-pattern gaseous detector to be added in CLAS12 detector in Hall-B at Jefferson Lab.
The detector is currently read out via APV25 chips.

The analysis chain proceeds from raw EVIO files through decoding, zero-suppression with pulse fitting,
clustering and Analysis coded. It also provides several Drawing programs to generate final plots.

**Detector at a glance:**
- 704 U strips + 704 V strips (1408 total channels)
- 12 readout slots (APV25-based)
- Strip pitch: 1 mm, strip angle: 10°
- Active area: X ∈ [−723, +723] mm, Y ∈ [−250, +250] mm (trapezoidal)
- 15 time samples per waveform at 25 ns per sample. This parameter is not always 15 can be changed in Data acquisition

---

# Project Structure

```
uRWellTestProto/
├── AnaCodes/                       # Analysis executables and scripts
│   ├── Skim_PulseFit.cc            # Zero-suppression + Landau pulse fitting
│   ├── AnaPulseFits.cc             # Clustering, cross reconstruction, histogramming
│   ├── CheckDecoding.cc            # Diagnostic check of decoded data
│   ├── AnaClustering.cc            # Legacy clustering (zero-suppressed input)
│   ├── DrawPedestals.cc            # Pedestal extraction and plots (ROOT macro)
│   ├── DrawPulseFitPlots.cc        # Visualization of pulse fit results (ROOT macro)
│   ├── DrawPlotsWithClustering.cc  # Cluster analysis visualization (ROOT macro)
│   ├── DrawHVDependencePlots.cc    # Efficiency vs. HV plots (ROOT macro)
│   ├── DrawEffWithHodo.cc          # Efficiency with hodoscope matching (ROOT macro)
│   ├── DrawNoise_Vs_StripCoorelations.cc  # Noise correlations (ROOT macro)
│   ├── UpdateStripSigmas.cc        # Update per-strip sigma parameters (ROOT macro)
│   ├── RunAnaChain.py              # Automated full-chain analysis script (first prototype)
│   ├── RunSecondProtoAnaChain.py   # Automated full-chain analysis script (second prototype)
│   ├── Decode_Run.py               # Standalone decoding script
│   └── CMakeLists.txt
├── Decoder/                        # Standalone C++ EVIO→HIPO decoder (no coatjava dependency)
│   ├── uRwellDecoder.cc            # Main: decode (URWELL::adc/XYHODO::tdc) or fused --pulse
│   ├── uRwellSRSDecoder.{h,cpp}    # SRS-APV decode (port of coatjava getDataEntries_57631)
│   ├── MarocHodoDecoder.{h,cpp}    # MAROC hodoscope decode (port of getDataEntries_57655)
│   ├── TranslationTable.{h,cpp}    # CCDB translation-table reader (reads the sqlite directly)
│   ├── HipoBankWriter.{h,cpp}      # Writes URWELL::adc / XYHODO::tdc / RUN::config
│   ├── PulseFitWriter.{h,cpp}      # --pulse: writes uRwell::Pulse (port of Skim_PulseFit)
│   ├── CompareDecoded.cc           # Validate URWELL::adc / XYHODO::tdc vs another HIPO file
│   ├── ComparePulse.cc             # Validate uRwell::Pulse vs another HIPO file
│   ├── evio-5.2/                   # Vendored EVIO-5.2 C library
│   └── CMakeLists.txt
├── include/
│   └── uRwellTools.h               # Core data structures, analysis utilities, and geometry tools
├── src/
│   └── uRwellTools.cc              # Implementation of uRwellTools
├── hipo/hipo4/                     # HIPO4 I/O library (header-only, included)
├── cmake_modules/                  # CMake find-modules (FindLZ4, etc.)
├── Doc/                            # Documentation assets (images)
├── PedFiles/                       # Pedestal and noise files (generated)
├── Skims/                          # Skimmed HIPO files (generated)
├── Data/                           # Decoded HIPO files (generated)
├── Figs/                           # Output plots (generated)
├── Pars/                           # Per-strip parameter files (generated)
└── CMakeLists.txt                  # Root build configuration
```

---

# Installation

### Prerequisites

You need to have the **LZ4** library and the **XYHodoTools** package installed.

#### LZ4
Available at https://lz4.org/.
On JLab ifarms or any machine with `/group/clas12` mounted, LZ4 is detected automatically.
Otherwise, point to your installation in `cmake_modules/FindLZ4.cmake`.

#### XYHodoTools
Download from https://code.jlab.org/hallb/XYHodo.

### Installing μRwellTools

1. Clone the repository:
   ```bash
   git clone git@github.com:rafopar/uRWellTestProto.git
   ```
2. Build and install:
   ```bash
   cmake -S . -Bbuild -DCMAKE_INSTALL_PREFIX=/path/to/install/
   cmake --build build
   cmake --install build
   ```

The build also produces the standalone C++ decoder, installed under a separate
`decoder/bin` subdirectory (see [Standalone C++ Decoder](#standalone-c-decoder-decoder)).

---

# Analysis Workflow

## Data Flow

```
Raw EVIO files
      │
      ▼
[coatjava decoder]
      │  Data/decoded_<RUN>_<FileNo>.hipo
      ▼
[Skim_PulseFit.exe]          ← requires PedFiles/Peds_<RUN>
      │  Skims/Skim_PulseFit_<RUN>_<FileNo>.hipo
      ▼
[AnaPulseFits.exe]
      │  AnaPulseFits_<RUN>_File_<FileNo>.root
      ▼
[hadd]
      │  AnaPulseFits_<RUN>.root  (final unified output)
      ▼
[ROOT plotting macros]
```

> The decoding (and optionally the skimming) step above can also be done by the bundled
> standalone C++ decoder (`Decoder/`), with no coatjava/Java dependency and faster runtime.
> Its `--pulse` mode fuses decoding and pulse fitting into a single pass, producing
> `Skims/Skim_PulseFit_<RUN>_<FileNo>.hipo` directly from EVIO. See
> [Standalone C++ Decoder](#standalone-c-decoder-decoder).

---

## Pedestal Runs

Pedestal runs are taken with a random trigger and lower HV so that no real signal is present.
Approximately 2000 events are sufficient (< 1 minute of data taking).

Decode the pedestal run the same way as a production run, then:

```bash
./CheckDecoding.exe <RUN> <FILE>
root -l 'DrawPedestals.cc(<RUN>)'
```

`DrawPedestals.cc` calculates the mean and RMS (noise) for every channel, produces diagnostic
plots in `Figs/`, and writes the results to:
- `PedFiles/Peds_<RUN>` — μRwell pedestals and noise
- `PedFiles/GEM_Peds_<RUN>` — GEM pedestals and noise

**Linking pedestals to a production run:**
If you want production run 3333 to use pedestals from run 1234:
```bash
ln -s Peds_1234 PedFiles/Peds_3333
ln -s GEM_Peds_1234 PedFiles/GEM_Peds_3333
```

---

## Decoding

Raw EVIO files must be decoded using a special branch of coatjava optimised for the μRwell
decoder (the standard branch is ~10× slower):

```bash
git clone git@github.com:JeffersonLab/clas12-offline-software.git
git checkout iss1008-urWellDecoder
```

The CCDB environment variable must point to the sqlite file:
```bash
export CCDB_CONNECTION="sqlite:////group/clas12/users/rafopar/uRWellImportant/clas12SecondProto.sqlite"
```
*(The triple slash before `/group` is intentional.)*

Decode a single file:
```bash
/path/to/decoder -i inpFile.evio -o Data/decoded_<RUN>_<FileNo>.hipo -c 1
```

After decoding, the HIPO file contains two key banks:
- `XYHODO::tdc` — hodoscope hits that pass threshold
- `URWELL::adc` — raw ADC waveforms for all 1408 μRwell channels (no online zero-suppression)

### Alternative: standalone C++ decoder

The bundled `Decoder/` provides a self-contained C++ decoder (`uRwellDecoder.exe`) that produces
the same `URWELL::adc` and `XYHODO::tdc` banks without the coatjava/Java dependency and runs
faster. It can also fuse the skim step into the same pass (`--pulse`). See
[Standalone C++ Decoder](#standalone-c-decoder-decoder).

---

## Skimming & Pulse Fitting

Because all 1408 channels are read out every event, the raw files are large and slow to process.
`Skim_PulseFit.exe` applies zero-suppression and fits each above-threshold pulse with a Landau function,
producing compact HIPO files for downstream analysis.

**Run:**
```bash
./Skim_PulseFit.exe -r <RUN> -f <FileNo>
```

**Input:** `Data/decoded_<RUN>_<FileNo>.hipo` + `PedFiles/Peds_<RUN>`

**Output:** `Skims/Skim_PulseFit_<RUN>_<FileNo>.hipo`

> **Note:** The `Skims/` directory must exist before running.

### Pulse Fitting Details

For each channel, the waveform (up to 15 time samples, 25 ns each) is compared to the pedestal.
Bins where ADC > 3σ (pedestal RMS) are considered hits. All qualifying pulses are fitted with:

```
f(x) = A · Landau(x, MPV, σ)
```

where **A** is the amplitude, **MPV** is the Most Probable Value, and **σ** is the width.

### Format of the Skimmed File (`uRwell::Pulse` bank)

The picture below shows one example event from the `uRwell::Pulse` bank:

![An example of the uRwell::Pulse bank](Doc/uRwell_PulseBank.png)

| Field | Description |
|---|---|
| `sec` | Sector: 6 = μRwell, 8 = GEM |
| `layer` | 1 = U layer, 2 = V layer |
| `strip` | Global strip number (1–704) |
| `stripLocal` | Strip number within the readout chip (1–128) |
| `adc` | Total ADC of the signal divided by number of time samples |
| `adcRel` | ADC divided by the strip noise σ (signal-to-noise ratio) |
| `ts` | Time sample index of the highest ADC bin (0 to n_ts − 1) |
| `slot` | Readout board (slot) number |
| `ped_rms` | RMS (noise) of the strip — constant across events |
| `pulse_A0` | Amplitude A of the Landau fit |
| `pulse_MPV` | MPV of the Landau fit |
| `pulse_Sigma` | Width σ of the Landau fit |
| `pulse_Chi2` | Chi² of the fit |
| `pulse_NDF` | Number of degrees of freedom |
| `pulse_ADCN` | Raw ADC in time sample N (N = 0–14) |

---

## Analysis: AnaPulseFits

`AnaPulseFits.exe` performs clustering, U×V cross reconstruction, hodoscope correlation, and
timing analysis on the skimmed data.

**Run:**
```bash
./AnaPulseFits.exe -r <RUN> -f <FileNo>
```

**Input:** `Skims/Skim_PulseFit_<RUN>_<FileNo>.hipo`

**Output:** `AnaPulseFits_<RUN>_File_<FileNo>.root`

### Analysis Steps

1. **Hodoscope analysis** — reconstruct matched crosses from the XYHodoscope TDC bank.
   A "clean" event requires exactly 1 left-right matched cross (`nLR_MatchedCross == 1`).

2. **Pulse reading** — loop over `uRwell::Pulse` bank entries for sector 6 (μRwell),
   separating hits into U-layer and V-layer pulse lists.

3. **Clustering** — adjacent pulses within 2-strip gaps are merged into `PulseCluster` objects.
   For each cluster the following are computed:
   - Seed pulse (highest integral)
   - Integral-weighted cluster center (strip coordinate)
   - Integral-weighted cluster MPV and σ
   - Cluster start time = Cluster MPV − Cluster σ

4. **Quality cuts** — applied to the seed pulse:
   - Pulse sigma: 1.2 ≤ σ ≤ 5.2

5. **Cross reconstruction** — the highest-integral U cluster and V cluster are paired to form
   a μRwell cross. X and Y detector coordinates are computed from the strip numbers
   using the 10° strip geometry.

6. **Active area cut** — crosses are required to be within the detector active area
   (used as numerator of the detection efficiency with the Hodo tag as denominator).

7. **Histogram filling** — 1D and 2D histograms are filled covering:
   - Pulse parameter distributions (MPV, σ, integral, χ²/NDF)
   - Cluster and seed properties vs. strip/slot
   - Cross X-Y positions
   - Timing differences Δt between U and V clusters
   - Correlations with hodoscope bar IDs
   - Per-pixel timing maps (coarse 20×10 grid)

### Efficiency Calculation

The detection efficiency per hodoscope pixel is.

For each hodoscope pixel, the μRwell detection efficiency is calculated the following way

```
efficiency[shortBarID][longBarID] =
    h_Cross_YXC_Max1_[shortBarID][longBarID]   (U∧V cluster + active area + Hodo tag)
    ─────────────────────────────────────────
    h_Hodo_XY_BarID_Tag1[shortBarID][longBarID] (Hodo tag only)
```
NOTE: with only single hodoscope, there will be non-negligible amount of cosmic tracks passing therough
the hodoscope, but missing the μRwell. So actuall efficiency of the μRwell close to the edges will be higher
the ratio defined above.

---

# Standalone C++ Decoder (`Decoder/`)

`Decoder/` is a self-contained C++ EVIO→HIPO decoder for the μRwell test-prototype data
(single crate / single FEC). It reproduces the relevant coatjava decoding **without the
coatjava/Java dependency**, and it is faster. It vendors a minimal EVIO-5.2 C library and
reads the CCDB translation tables directly from the sqlite file (no `libccdb`).

> **One intentional deviation from coatjava:** for the half-populated edge APVs (slots 0 and
> 6, which carry 64 μRwell sector-6 strips + 64 sector-7 strips), the common mode is built
> only from the sector-6 strips (`mask >= 64`) for **both** slots. The coatjava
> `getDataEntries_57631` used the sector-7 half for slot 6, biasing its common mode high; this
> decoder fixes that, so its decoded slot-6 ADC values differ from coatjava by design.

It builds as part of the normal `cmake --build build` and installs to its own subdirectory
`${CMAKE_INSTALL_PREFIX}/decoder/bin`:

| Executable | Purpose |
|---|---|
| `uRwellDecoder.exe` | The decoder (standard mode and fused `--pulse` mode) |
| `CompareDecoded.exe` | Compare `URWELL::adc` / `XYHODO::tdc` of two HIPO files (order-insensitive multiset) |
| `ComparePulse.exe` | Compare `uRwell::Pulse` of two HIPO files (multiset, keyed by `RUN::config.event`) |

## Decoding (standard mode)

```bash
uRwellDecoder.exe -i inpFile.evio -o Data/decoded_<RUN>_<FileNo>.hipo
```

| Option | Default | Description |
|---|---|---|
| `-i <file>` | — | Input EVIO file (**required**) |
| `-o <file>` | — | Output HIPO file (**required**) |
| `-r <run>` | parsed from filename `urwell_maroc_<run>` | Run number (for CCDB resolution) |
| `-n <N>` | all | Decode at most N events |
| `-v <variation>` | `default` | CCDB variation |
| `--ccdb <conn>` | `sqlite:////group/clas12/users/rafopar/uRWellImportant/clas12SecondProto.sqlite` | CCDB connection string |
| `-c <n>` | ignored | Accepted for coatjava CLI compatibility (compression type) |

The output banks (`URWELL::adc`, `XYHODO::tdc`, `RUN::config`) match the coatjava decoder
**when the same CCDB is used**, except for the intentional slot-0/6 common-mode fix noted
above (which shifts slot-6 `URWELL::adc` values). Verified for run 3208 with the default
`clas12SecondProto.sqlite` via `CompareDecoded.exe`.

> The CCDB must contain the μRwell translation table. The built-in default
> `clas12SecondProto.sqlite` is the one used by `RunSecondProtoAnaChain.py`; override with
> `--ccdb` if you need a different one.

## Fused Pulse-Fit Mode (`--pulse`)

`--pulse` performs the decoding **and** the `Skim_PulseFit` step in a single pass. Instead of
writing the large `URWELL::adc` bank it builds the strip waveforms, keeps only strips above the
3σ threshold, Landau-fits each one, and writes the compact `uRwell::Pulse` bank directly (plus an
empty `RAW::adc`, `RUN::config`, and `XYHODO::tdc`). This avoids writing and re-reading the
multi-GB decoded intermediate file.

```bash
uRwellDecoder.exe -i inpFile.evio -o Skims/Skim_PulseFit_<RUN>_<FileNo>.hipo \
    --pulse --peds-dir PedFiles
```

| Option | Default | Description |
|---|---|---|
| `--pulse` | off | Write `uRwell::Pulse` instead of `URWELL::adc` |
| `--peds-dir <dir>` | `PedFiles` | Directory holding `Peds_<RUN>` and `GEM_Peds_<RUN>` |

Requirements and behaviour:
- Needs `<peds-dir>/Peds_<RUN>` (μRwell, sector 6) and `<peds-dir>/GEM_Peds_<RUN>` (GEM,
  sector 8) — exactly the files `Skim_PulseFit` uses. The GEM file may be empty if the run has
  no GEM channels.
- The `uRwell::Pulse` output is a direct port of `AnaCodes/Skim_PulseFit.cc`, so the threshold,
  fit function, and bank format are the same (see
  [Skimming & Pulse Fitting](#skimming--pulse-fitting)). The result is **bit-identical** to the
  two-step `decode → Skim_PulseFit` (verified with `ComparePulse.exe` on run 3208: 167 events,
  619 pulses, 0 mismatching rows).

**Performance (per 1000 events, run 3208, warm cache):** the fused path is ~31% faster in
wall-clock (~27.5 s vs ~22.5 s decode + ~17.4 s skim) and eliminates the ~55 MB-per-1000-events
`URWELL::adc` intermediate (~2.8 GB for a full EVIO file). The Landau fit is the dominant cost and
is unchanged; the saving comes from not writing/reading the giant bank.

> **Trade-off:** because the `URWELL::adc` bank is never written, you cannot re-skim with a
> different threshold or different pedestals without re-decoding from EVIO. Use the standard mode
> if you also need the full decoded file for other analyses.

## Validation

```bash
# decoded banks vs another decoded file (e.g. C++ vs coatjava)
CompareDecoded.exe fileA.hipo fileB.hipo

# uRwell::Pulse of the fused output vs a decode → Skim_PulseFit output
ComparePulse.exe merged.hipo Skims/Skim_PulseFit_<RUN>_<FileNo>.hipo
```

Both tools report the number of mismatching events/rows and print `IDENTICAL` on a perfect match.

---

# Automated Running

`RunSecondProtoAnaChain.py` runs the full analysis chain in parallel on a multi-core machine.
It is **not recommended on JLab ifarms** (use batch jobs there instead).

```bash
python3 RunSecondProtoAnaChain.py <RUN> [START_TASK] [--ana=pulsefit|doublehodo|both]
```

The script launches up to 18 parallel jobs, waits until ≤ 8 are still running, then submits
the next batch. Decoding and skimming always run first; after `TASK_SkimPulseFit` the
analysis selected with `--ana` is run (and its per-file ROOT outputs merged). The available
tasks are:

| Task name | Description | Typical time per file |
|---|---|---|
| `TASK_DECODE` | Decode EVIO → HIPO with coatjava | ~30 min |
| `TASK_SkimPulseFit` | Run `Skim_PulseFit.exe` | ~30 min |
| `TASK_AnaPulseFit` | Run `AnaPulseFits.exe` → `AnaPulseFits_<RUN>_File_<FileNo>.root` | < 20 s |
| `TASK_Hadd_pulsefit` | Merge `AnaPulseFits_<RUN>_File_*.root` → `AnaPulseFits_<RUN>.root`, then remove inputs | seconds |
| `TASK_AnaDoubleHodo` | Run `AnaSecondProtoDoubleHodo.exe` → `AnaSecondHodoDoubleHodo_<RUN>_File_<FileNo>.root` | < 20 s |
| `TASK_Hadd_doublehodo` | Merge `AnaSecondHodoDoubleHodo_<RUN>_File_*.root` → `AnaSecondHodoDoubleHodo_<RUN>.root`, then remove inputs | seconds |

**Selecting the analysis (`--ana`):** choose which analysis to run after skimming.
- `--ana=pulsefit` — only `AnaPulseFits.exe`
- `--ana=doublehodo` — only `AnaSecondProtoDoubleHodo.exe`
- `--ana=both` — both, one after another (**default**)

`AnaPulseFits.exe` and `AnaSecondProtoDoubleHodo.exe` are independent programs; each task is
followed by its own `hadd` step that merges the matching per-file ROOT files.

The `[START_TASK]` argument is optional. If omitted, the chain starts from `TASK_DECODE`.
To resume from a later step (e.g. if decoding is already done):

```bash
# Skim + both analyses, starting from skimming
python3 RunSecondProtoAnaChain.py 3333 TASK_SkimPulseFit

# Only the double-hodo analysis, starting from its task (skim already done)
python3 RunSecondProtoAnaChain.py 3333 TASK_AnaDoubleHodo --ana=doublehodo
```

---

# Plotting & Visualization Scripts

These ROOT macros are run interactively with `root -l`. They read the output of the analysis
executables and produce PDF/PNG plots in the `Figs/` directory.

| Script | Input | Description |
|---|---|---|
| `DrawPedestals.cc` | `CheckDecoding_<RUN>_0.root` | Fits pedestal distributions, writes `PedFiles/Peds_<RUN>` |
| `DrawPulseFitPlots.cc` | `AnaPulseFits_<RUN>.root` | Pulse fit parameter distributions |
| `DrawPlotsWithClustering.cc` | `AnaPulseFits_<RUN>.root` | Cluster size, position, and charge |
| `DrawHVDependencePlots.cc` | multiple runs | Efficiency vs. detector HV |
| `DrawEffWithHodo.cc` | `AnaPulseFits_<RUN>.root` | Detection efficiency with hodoscope tagging; also produces a TF2-based 2D map of the normalized U−V strip RO-length difference across the detector face (`Figs/Str_ROLength_Diff.*`) |
| `DrawNoise_Vs_StripCoorelations.cc` | `CheckDecoding_<RUN>_0.root` | Strip-to-strip noise correlations |
| `UpdateStripSigmas.cc` | `AnaPulseFits_<RUN>.root` | Updates per-strip σ in `Pars/Pulse_Sigmas_<RUN>.dat` |

---

# Geometry Utilities in uRwellTools

Two functions in `uRwellTools` compute the distance along the strip direction from a hit
position `(x, y)` to the detector edge where the readout (RO) connectors are located:

```cc
double uRwellTools::getROLength_U(double x, double y);
double uRwellTools::getROLength_V(double x, double y);
```

Both functions:
- Return `-1` if `(x, y)` is outside the active detector area.
- Account for the strip flip point at strip 448.5, where the readout side switches:
  U strips switch from the **left** edge to the **right** edge, and V strips switch from
  the **right** edge to the **left** edge.
- Return the Euclidean distance in **mm** from `(x, y)` along the strip to the boundary
  where the connector sits.

These functions are used in `DrawEffWithHodo.cc` to define a ROOT `TF2` object
(`f_UVStrROLengthDiff`) that maps the normalized (to the speed of light) difference

```
f(x, y) = (getROLength_U(x, y) − getROLength_V(x, y)) / 300
```

across the full detector face, and saves it as `Figs/Str_ROLength_Diff.*`. This visualization
helps study any signal amplitude or timing dependence on the distance to the RO connector.

---

# Key Analysis Parameters

| Parameter | Value | Location | Description                                                   |
|---|---|---|---------------------------------------------------------------|
| Hit threshold | 3σ | `Skim_PulseFit.cc` | Minimum ADC above pedestal to consider a hit                  |
| `PulseSigmaMin` | 1.2 | `AnaPulseFits.cc` | Minimum acceptable Landau σ                                   |
| `PulseSigmaMax` | 5.2 | `AnaPulseFits.cc` | Maximum acceptable Landau σ                                   |
| `chi2NDF_cut` | 40 | `AnaPulseFits.cc` | Maximum χ²/NDF for a pulse (under study), Not being used yet. |
| Cluster gap | 2 strips | `uRwellTools.cc` | Maximum strip gap within a cluster                            |
| `minHits` | 2 | `AnaPulseFits.cc` | Minimum pulses in a cluster to be used                        |
| `T_OverThrCut` | 20 ns | `AnaPulseFits.cc` | Hodoscope over-threshold time cut                             |
| `deltaT_Cut_Cross` | 20 ns | `AnaPulseFits.cc` | Hodoscope cross Δt cut                                        |
| `deltaT_Cut_PMT12_Match` | 20 ns | `AnaPulseFits.cc` | Hodoscope PMT1/2 match Δt cut                                 |
| `sec_uRwell` | 6 | `AnaPulseFits.cc` | Sector ID of the μRwell detector                              |
| `sec_GEM` | 8 | `AnaPulseFits.cc` | Sector ID of the GEM detector                                 |
| Strip flip point | 448.5 | `uRwellTools.cc` | Strip number above which RO connector side switches (U and V) |

---

# Data File Locations

| Type              | Path                                                                        |
|-------------------|-----------------------------------------------------------------------------|
| EVIO              | `Up to the user`                                                            |
| Decoded HIPO      | `Data/decoded_<RUN>_<FileNo>.hipo`                                          |
| Skimmed HIPO      | `Skims/Skim_PulseFit_<RUN>_<FileNo>.hipo`                                   |
| Per-file analysis | `AnaPulseFits_<RUN>_File_<FileNo>.root`                                     |
| Merged analysis   | `AnaPulseFits_<RUN>.root`                                                   |
| Pedestals         | `PedFiles/Peds_<RUN>`, `PedFiles/GEM_Peds_<RUN>`                            |
| Per-strip σ       | `Pars/Pulse_Sigmas_<RUN>.dat`                                               |
| Plots             | `Figs/`                                                                     |
| CCDB sqlite       | `Up to the user: Just make sure to properly set the CCDB_CONNECTUION to it` |


[comment]: <> ( ## Clone the package )

[comment]: <> (Package has a dependency on hipo library, which is a submodule. )
[comment]: <> (Use the command: )
[comment]: <> ( )
[comment]: <> (``` )
[comment]: <> (git clone --recurse-submodules git@github.com:rafopar/uRWellTestProto.git )
[comment]: <> (``` )
[comment]: <> ( )
[comment]: <> (to clone the distribution. )
