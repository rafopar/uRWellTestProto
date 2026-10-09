# HV Section Quality

Characterization of the 31 HV sections of the uRwell 2nd prototype.  Every section
is ramped up separately and the highest voltage at which it stays stable (no
leakage current developing, no trips) is recorded.  This is repeated for every gas.
This directory holds the measurements and the code that draws them on top of the
real detector geometry.

Documented in the top level `README.md`, section "HV Section Quality" -- keep the
two in sync when something here changes.

## Files

| file | what it is |
|------|------------|
| `TOP_HV_Sections_<gas>.dat` | measured max stable HV, TOP detector, one file per gas |
| `BOT_HV_Sections_<gas>.dat` | measured max stable HV, BOTTOM detector, one file per gas |
| `uRwell_HV_section_geometry.dat` | polygons of the 31 sections, generated (see below) |
| `extract_section_geometry.py` | CAD (DXF) -> `uRwell_HV_section_geometry.dat` |
| `plot_HV_sections.py` | the plot: both detectors, color coded by max stable HV, one figure per gas |
| `Figs/` | output figures, git ignored, created by the script if missing |
| `CMakeLists.txt` | installs the scripts, geometry and measurement files under `<prefix>/misc/HV_Section_Quality` |

`<gas>` is `90Ar_7CO2_3Iso` (90% Ar + 7% CO2 + 3% iso-C4H10) or `UHPN2` (ultra
high purity N2).

## Gases

The known gases are listed in the `GASES` table at the top of `plot_HV_sections.py`:
the name used in the file names -> the gas as it is written in the figure title.
A new gas needs its two `.dat` files plus one line in that table, and the two files
added to the `install(FILES ...)` list in `CMakeLists.txt`.

* Without `--gas` every gas of the table whose two files exist is drawn, one figure
  per gas (`Figs/HV_Section_MaxStableHV_<gas>.png/.pdf`), a gas without files is
  skipped with a message.  `--gas A [B ...]` draws only those, and then missing
  files are an error.
* The gases are deliberately taken from the table and **not** found by globbing
  `TOP_HV_Sections_*.dat`: the install directory can hold stale files, because
  `make install` never removes anything (e.g. the pre-rename `*_90Ar_7Iso_3CO2.dat`,
  which had the CO2 and iso fractions swapped in the name), and a glob would
  silently plot them as an extra gas.
* TOP and BOTTOM share one color scale, but every gas gets its own: N2 needs
  ~150 V more than the Ar mixture, a common scale would flatten both.
  `--vmin`/`--vmax` apply to every gas drawn, so use them together with `--gas`.

## Where things are edited (easy to trip over)

**The measurement files are installed, so edit them in the repository, not in the
install directory.**  Every `make install` overwrites the installed copies with the
repository ones (CMake copies whenever the time stamps differ, so an edited installed
copy is always overwritten).  An earlier version deliberately did not install them,
because they used to be edited in place in the install directory and a
`make install` had already wiped such edits once.  If values are ever entered in the
install directory again, copy them back into the repository **before** the next
`make install`.

The figures embedded in the README are manual snapshots in `Doc/` and have to be
refreshed by hand when the measurements change:

| README figure | snapshot of |
|---------------|-------------|
| `Doc/HVSection_MaxStableHV.png` | `Figs/HV_Section_MaxStableHV_90Ar_7CO2_3Iso.png` |
| `Doc/HVSection_MaxStableHV_UHPN2.png` | `Figs/HV_Section_MaxStableHV_UHPN2.png` |

## Data file format

Two whitespace separated columns, `#` starts a comment (also at the end of a line,
which is used to record observations about individual sections):

```
# SECTION	MAX_STABLE_HV [V]
1		490
9		-1     <- negative = not measured yet, drawn hatched as 'n/a'
20		470  # held 510 for ~30 min, later did not hold 480 for long
```

## Geometry

Source of truth is the CAD export, **not** in this repo:

```
/home/rafopar/work/CLAS12/uRWell/Docs/2ndPrototype/DFS3381_export_v2-18-12-2024/
    DFS3381_activearea.dxf   <- section polygons, used by extract_section_geometry.py
    DFS3381_TOP.pdf          <- HV traces / connections, used to fix the numbering
```

The active area is a trapezoid, 1453 x 495 mm2, wide edge at the top, cut into 31
vertical sections of ~41 mm pitch.  The 2 outermost sections on each side are
clipped by the slanted edges into a triangle / trapezoid.

In the DXF the sections appear as **32** closed POLYLINEs, because the central
section is drawn as two half polygons split at x = 0 (the seam is only ~0.5 mm,
much thinner than the ~1.2 mm gap between real sections).
`extract_section_geometry.py` merges those two halves back into one polygon, which
is why section 16 has 56 vertices while the others have ~30.

`uRwell_HV_section_geometry.dat` is committed, so the plotting does **not** need the
CAD file at runtime.  Re-run the extractor only if the CAD export changes.

## Section numbering (important, easy to get wrong)

```
sec:   17  18  19  ...  30  31 | 16  15  14  ...   2   1
x:   -727 -602 -538     -81 -40 |  0  +40  +81      +602 +727   [mm]
```

* Sections **1-16** are powered from the **right** side of `DFS3381_TOP.pdf`,
  sections **17-31** from the **left** side (16 vs 15 sections).
* **Both groups are counted from the outer edge of the detector inwards**, i.e. the
  numbering is mirror symmetric.  Section 1 is the right most, section 17 the left
  most, and the two inner most sections 16 (the central one) and 31 are neighbours.

What is actually verifiable from the CAD: the 16/15 split, and that the central
section's HV trace runs to the *right* (visible when zooming into the top center of
`DFS3381_TOP.pdf`), which makes the central section number 16.  The gerber/DXF carry
**no** section labels, so the direction within each side is a lab convention and
cannot be derived from the drawing.

The first implementation got this wrong: it numbered straight through from right to
left, putting section 17 next to 16 and section 31 at the left edge.  That is **not**
the convention -- it is the other way around, see the sketch above.  The mapping is
built in `build_sections()` of `extract_section_geometry.py`.

`plot_HV_sections.py --mirror` flips x if the detector is viewed from the other side.

## Usage

```bash
./plot_HV_sections.py                                  # every gas -> Figs/HV_Section_MaxStableHV_<gas>.png/.pdf
./plot_HV_sections.py --gas UHPN2                      # only the given gas(es)
./plot_HV_sections.py --gas 90Ar_7CO2_3Iso --output-dir SomeOtherDir --cmap RdYlGn --vmin 440 --vmax 530
./extract_section_geometry.py [dxf] [out]              # only if the CAD changed
```

Needs `numpy` + `matplotlib` only (Agg backend, no display required).

## Status / next steps

As of 2026-09-28:

* 90% Ar + 7% CO2 + 3% iso-C4H10: done.  All sections measured except BOTTOM
  section 1 (initially entered as 500 V, later set back to -1).  TOP 460-520 V,
  BOTTOM 440-520 V.
* UHP N2: TOP 640-670 V, BOTTOM 630-670 V.  Still to do on both detectors:
  sections **13-16** and **29-31**, plus BOTTOM section 1.  So far measured in pairs
  of adjacent sections sharing the same value (1&2, 3&4, ... , 17&18, ...).

To add new measurements, edit the `.dat` files in the repository, re-run
`plot_HV_sections.py` (or `make install` and run it in the install directory), and
refresh the README figures in `Doc/` (table above) if they should show the new values.
