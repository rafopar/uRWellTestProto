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
| `TOP_HV_Sections_<gas>.dat` | measured max stable HV (+ trip limit), TOP detector, one file per gas |
| `BOT_HV_Sections_<gas>.dat` | measured max stable HV (+ trip limit), BOTTOM detector, one file per gas |
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
  ~150 V more than the Ar mixture, a common scale would flatten both.  By default
  it runs from the lowest HV - 10 V to the highest HV + 10 V of the gas.
  `--vmin`/`--vmax` apply to every gas drawn, so use them together with `--gas`.
* `--delta DELTA` gives every gas a scale of the same width instead, so that the
  colors of different gases can be compared: from the highest HV of the gas (no
  +10 V margin, the highest sections get the top color) down to highest - DELTA.
  `--delta` without a value takes the widest range (highest - lowest + 10 V) of all
  gases that have files, drawn or not, so no gas is clipped and a figure does not
  depend on `--gas` (80 V as of 2026-10-28, set by the Ar mixture).  It can not be
  combined with `--vmin`/`--vmax`.

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

Whitespace separated columns, `#` starts a comment (also at the end of a line,
which is used to record observations about individual sections):

```
# SECTION	MAX_STABLE_HV [V]	TRIP_LIMIT [uA]
1		490		15
9		-1             <- negative = not measured yet, drawn hatched as 'n/a'
20		470		10  # held 510 for ~30 min, later did not hold 480 for long
```

The third column, the HV trip limit the section was measured with, is optional (the
Ar files do not have it, a not measured section leaves it empty).

## HV trip limit (red 'x')

During the N2 scan the trip limit was raised from 10 uA to 15 uA on 2026-09-25, the
first setting with 15 uA is setting #6.  With 10 uA a section can trip below the HV
it could hold, so its value is only a lower limit.  `plot_HV_sections.py` puts a
small red 'x' at the top of every measured section whose trip limit is lower than
the highest one in the two files of that gas (no legend entry, deliberately).

Which section had which limit comes from the lab log `EEL_Runs.xlsx` (not in git),
tab "HV settings " (with a trailing space), N2 block from row 29 on:

* Settings #1-#5 (rows 30-34, 10 uA) measured pairs: TOP 1-10 and 17-26, BOTTOM
  2-10 and 17-26.
* From setting #9 (row 38) sections are measured one by one, all with 15 uA.  A
  section of a 10 uA pair that was re-measured there is 15 uA, its partner stays
  10 uA unless it was re-measured too (e.g. BOTTOM 3 & 4: 640 V in #2, then BOTTOM 3
  alone 670 V in #10 -> 3 became 15 uA, 4 stayed 10 uA until it was re-measured
  alone later, 680 V with 15 uA).
* Not counted as re-measurements: setting #16 TOP right (section 8, not connected)
  and setting #17 (60 V on every channel, not a max HV measurement).

Result as of 2026-11-02, still 10 uA: TOP 10, 18, 21, 22, 26 and BOTTOM 2, 24, 26
(TOP 5, 6, 8 and BOTTOM 4, 10, 20, 22 were re-measured with 15 uA in the meantime).
When one of them is re-measured, update its HV **and** set its trip limit to 15 in
the `.dat` file, the 'x' then disappears.

The same kind of trap exists for the pair measurements in general: when a section
is re-measured alone, its old pair value has to be replaced (BOTTOM 14 with the Ar
mixture kept the 440 V of the 13 & 14 pair, setting #7, although it held 450 V
alone in setting #10 -- fixed in commit b61dce2).

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

## BOTTOM detector as mounted (`--flip-bottom`)

The BOTTOM detector is mounted flipped in x with respect to the TOP one.  By default
both are drawn with the same orientation (section 1 on the right), `--flip-bottom`
draws the BOTTOM one as it actually sits under the TOP detector: section 17 under
section 1, 18 under 2, ..., 31 under 15, 16 stays in the center.  This is a plain x
mirror of the BOTTOM polygons (`mirror_x()`), which is exact because the geometry is
symmetric around x = 0 (checked: the centroids of 1+k and 17+k mirror onto each other
to 0.1 mm).  The "powered from this side" labels swap sides with it.

* The BOTTOM panel title says that it is flipped and how the sections line up, that
  is the requested "the figure has to show clearly that it is flipped".
* The figures get `_BotFlipped` appended (`FLIP_SUFFIX`):
  `Figs/HV_Section_MaxStableHV_<gas>_BotFlipped.png/.pdf`, so they never overwrite the
  default figures that the README snapshots in `Doc/` are taken from.
* Combined with `--mirror` both panels are mirrored first, i.e. the BOTTOM one ends up
  in its original orientation; "17 under 1" stays true.

## Usage

```bash
./plot_HV_sections.py                                  # every gas -> Figs/HV_Section_MaxStableHV_<gas>.png/.pdf
./plot_HV_sections.py --gas UHPN2                      # only the given gas(es)
./plot_HV_sections.py --delta                          # same color scale width for every gas (widest range)
./plot_HV_sections.py --delta 100                      # ... with a given width [V]
./plot_HV_sections.py --flip-bottom                    # BOTTOM detector as mounted -> ..._<gas>_BotFlipped.png/.pdf
./plot_HV_sections.py --gas 90Ar_7CO2_3Iso --output-dir SomeOtherDir --cmap RdYlGn --vmin 440 --vmax 530
./extract_section_geometry.py [dxf] [out]              # only if the CAD changed
```

Needs `numpy` + `matplotlib` only (Agg backend, no display required).

## Status / next steps

As of 2026-11-02:

* 90% Ar + 7% CO2 + 3% iso-C4H10: done.  All sections measured except BOTTOM
  section 1 (initially entered as 500 V, later set back to -1).  TOP 460-520 V,
  BOTTOM 450-520 V.
* UHP N2: all sections measured except BOTTOM section 1.  TOP 660-680 V, BOTTOM
  640-680 V.  Settings #1-#8 measured pairs of adjacent sections, from #9 on single
  sections.  Still only with the 10 uA trip limit: the 8 sections listed in
  "HV trip limit" above.

To add new measurements, edit the `.dat` files in the repository, re-run
`plot_HV_sections.py` (or `make install` and run it in the install directory), and
refresh the README figures in `Doc/` (table above) if they should show the new values.
