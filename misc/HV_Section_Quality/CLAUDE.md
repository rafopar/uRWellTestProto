# HV Section Quality

Characterization of the 31 HV sections of the uRwell 2nd prototype.  Every section
is ramped up separately and the highest voltage at which it stays stable (no
leakage current developing, no trips) is recorded.  This directory holds the
measurements and the code that draws them on top of the real detector geometry.

Documented in the top level `README.md`, section "HV Section Quality" -- keep the
two in sync when something here changes.

## Files

| file | what it is |
|------|------------|
| `TOP_HV_Sections_90Ar_7Iso_3CO2.dat` | measured max stable HV, TOP detector |
| `BOT_HV_Sections_90Ar_7Iso_3CO2.dat` | measured max stable HV, BOTTOM detector |
| `uRwell_HV_section_geometry.dat` | polygons of the 31 sections, generated (see below) |
| `extract_section_geometry.py` | CAD (DXF) -> `uRwell_HV_section_geometry.dat` |
| `plot_HV_sections.py` | the plot: both detectors, color coded by max stable HV |
| `plot_HV_initial_test_schematic.py` | schematic of the *initial* HV test (whole half of a detector on one channel) |
| `Figs/` | output figures, git ignored, created by the script if missing |
| `CMakeLists.txt` | installs the scripts + geometry under `<prefix>/misc/HV_Section_Quality` |

## Where things are edited (easy to trip over)

**The two measurement files are deliberately NOT installed.**  They are edited in
place in the install directory while measuring, and having them in `install(FILES ...)`
made every `make install` overwrite those edits (this already happened once and the
edits were lost).  Only the scripts, the generated geometry and an empty `Figs/` are
installed.

The consequence: the repository copy and the install copy of the `.dat` files can
drift apart silently.  Values measured in the install directory have to be copied
back into the repository by hand before they can be committed.

The figure embedded in the README lives in `Doc/HVSection_MaxStableHV.png`.  It is a
manual snapshot of `Figs/HV_Section_MaxStableHV.png` and has to be refreshed by hand
when the measurements change.

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

## Initial HV test (not the section by section measurement)

Before the sections were characterized one by one, both detectors were tested with
**all HV jumpers in place**, i.e. one complete half of a detector -- all the sections
that are powered from the same side -- hanging on a single CAEN channel:

```
CAEN HV pin -> MESH ,  RESIST -> Keithley picoammeter input ,  Keithley ground -> earth
```

Two picoammeters were available, so two halves were measured at a time, always one
per detector and on opposite sides.  Because the BOTTOM detector is the same uRwell
foil flipped left <-> right (BOTTOM: 1-16 on the left, 17-31 on the right, opposite
to the TOP), the two configurations are

| configuration | halves | sections |
|---|---|---|
| A | TOP-LEFT + BOTTOM-RIGHT | 17-31 of both detectors (15 each) |
| B | TOP-RIGHT + BOTTOM-LEFT | 1-16 of both detectors (16 each) |

`plot_HV_initial_test_schematic.py` draws one figure per configuration: a 3D sketch of
the two stacked planes (TOP blue, BOTTOM red, the energized half filled, the rest grey)
plus the wiring of both chains underneath.

The 3D view is an orthographic projection computed by hand (`project()`) and drawn into
an ordinary 2D axes.  **mplot3d is not used on purpose**: `Axes3D.apply_aspect` forces
its axes into a square and clips everything outside it, which throws away most of the
width for an object as elongated as this detector (the leads simply disappeared).  For
the same kind of reason the axis limits are fitted by hand in `fit_limits()` instead of
`set_aspect("equal")`: with `adjustable="datalim"` the two configurations came out at
different scales.

## Usage

```bash
./plot_HV_sections.py                                  # -> Figs/HV_Section_MaxStableHV.png/.pdf
./plot_HV_sections.py --output-dir SomeOtherDir --cmap RdYlGn --vmin 440 --vmax 530
./plot_HV_initial_test_schematic.py                    # -> Figs/HV_InitialTest_*.png/.pdf
./plot_HV_initial_test_schematic.py --elev 20 --azim 17   # viewing angle of the 3D sketch
./extract_section_geometry.py [dxf] [out]              # only if the CAD changed
```

Both detectors share one color scale so they can be compared directly.  Needs
`numpy` + `matplotlib` only (Agg backend, no display required).

The two schematics are also snapshotted into `Doc/HVSection_InitialTest_*.png` for the
README, by hand, exactly like `Doc/HVSection_MaxStableHV.png`.

## Status / next steps

As of 2026-08-17, with 90% Ar + 7% iso-C4H10 + 3% CO2 (unchanged since 2026-08-13):

* TOP: sections 1-8 and 17-24 measured, 470-520 V.
* BOTTOM: sections 2-8 and 17-24 measured, 450-510 V.  Section 1 was initially
  entered as 500 V and later set back to -1.
* Still to do on both detectors: the whole central band, sections **9-16** and
  **25-31**.

Measurements so far come in pairs of adjacent sections sharing the same value
(1&2, 3&4, ... , 17&18, ...).

To add new measurements, edit the two `.dat` files and re-run `plot_HV_sections.py`.
If they were edited in the install directory, copy them back into the repository
first (see "Where things are edited" above), and refresh `Doc/HVSection_MaxStableHV.png`
if the README figure should show the new values.
