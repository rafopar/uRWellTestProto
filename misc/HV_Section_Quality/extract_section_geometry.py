#!/usr/bin/env python3
"""
Extract the geometry of the 31 uRwell HV sections from the CERN/Rui gerber-export
DXF file 'DFS3381_activearea.dxf' and write it into a plain text file that is used
by plot_HV_sections.py.

The active area of the 2nd prototype is a trapezoid (wide edge at the top, narrow
edge at the bottom) which is segmented into 31 vertical HV sections.  In the DXF
the sections show up as 32 closed POLYLINEs: the central section is drawn as two
half polygons split at x = 0, all other sections are single polygons.

Section numbering convention (see DFS3381_TOP.pdf):
    sections  1 - 16  are powered from the RIGHT side of the drawing (x > 0)
    sections 17 - 31  are powered from the LEFT  side of the drawing (x < 0)
Both groups are counted from the outer edge of the detector towards the center,
i.e. the numbering is mirror symmetric:

    sec:   17  18  19 ...  30  31 | 16  15  14 ...   2   1
    x:   -727                 -40 | +0               +565 +727

Section  1 is the right most one, section 17 the left most one, and the two
inner most sections 16 (center, its HV trace runs to the right) and 31 are
neighbours.

Usage:
    ./extract_section_geometry.py [DXF file] [output file]
"""

import sys
import os

DEFAULT_DXF = ("/home/rafopar/work/CLAS12/uRWell/Docs/2ndPrototype/"
               "DFS3381_export_v2-18-12-2024/DFS3381_activearea.dxf")
DEFAULT_OUT = "uRwell_HV_section_geometry.dat"

N_SECTIONS = 31
N_RIGHT = 16     # sections 1 ... 16 sit on the right half (x > 0) of the drawing
SEAM_TOL = 0.5   # [mm] vertices closer than this to x = 0 belong to the central seam


def read_dxf_polylines(fname):
    """Return the list of closed POLYLINEs (list of (x, y) vertices) of a DXF file."""
    with open(fname) as f:
        lines = [l.rstrip("\n") for l in f]

    polys, cur, x = [], None, None
    for i in range(0, len(lines) - 1, 2):
        code, val = lines[i].strip(), lines[i + 1].strip()
        if code == "0":
            if val == "POLYLINE":
                cur = []
            elif val == "SEQEND" and cur is not None:
                polys.append(cur)
                cur = None
        if cur is not None:
            if code == "10":
                x = float(val)
            elif code == "20":
                cur.append((x, float(val)))
    if cur:
        polys.append(cur)
    return polys


def merge_central_halves(right, left):
    """Glue the two half polygons of the central section into a single polygon.

    Both halves are snapped onto x = 0 first, then the left half is rotated so
    that it starts on the seam and can simply be appended to the right half.
    """
    def snap(poly):
        return [(0.0 if abs(px) < SEAM_TOL else px, py) for px, py in poly]

    right, left = snap(right), snap(left)

    # index of the vertex where the left half comes back to the seam (x = 0)
    # while running along the bottom edge -> start the left half there
    seam = [k for k, (px, py) in enumerate(left) if px == 0.0]
    ybot = min(py for px, py in left)
    start = min((k for k in seam if left[k][1] < 0.5 * ybot), key=lambda k: left[k][1] > 0)
    left = left[start:] + left[:start]

    return right + left


def build_sections(polys):
    """Sort the polygons from right to left and return {section number: polygon}."""
    def xcenter(p):
        return sum(px for px, py in p) / len(p)

    polys = sorted(polys, key=xcenter, reverse=True)

    # the two narrow central polygons are the two halves of one single section
    widths = [max(px for px, _ in p) - min(px for px, _ in p) for p in polys]
    half = [k for k, w in enumerate(widths) if w < 0.6 * max(widths[3:-3])]
    if len(half) != 2 or half[1] != half[0] + 1:
        raise RuntimeError("could not identify the two central half sections: %s" % half)

    merged = merge_central_halves(polys[half[0]], polys[half[1]])
    polys = polys[:half[0]] + [merged] + polys[half[1] + 1:]

    if len(polys) != N_SECTIONS:
        raise RuntimeError("found %d sections instead of %d" % (len(polys), N_SECTIONS))

    # 'polys' is now ordered right -> left.  The right half (x > 0) is numbered
    # 1 ... 16 from the right edge inwards, the left half is numbered 17 ... 31
    # from the LEFT edge inwards, so it has to be counted backwards here.
    sections = {}
    for i, p in enumerate(polys):
        sections[i + 1 if i < N_RIGHT else (N_SECTIONS + N_RIGHT) - i] = p
    return sections


def write_geometry(sections, fname, dxf):
    with open(fname, "w") as f:
        f.write("# uRwell 2nd prototype - geometry of the %d HV sections\n" % N_SECTIONS)
        f.write("# Extracted with extract_section_geometry.py from\n")
        f.write("#   %s\n" % dxf)
        f.write("# Units: mm, origin at the center of the active area,\n")
        f.write("# +x = right side of DFS3381_TOP.pdf (HV side of sections 1-16).\n")
        f.write("# Numbering: 1 ... 16 from the right edge inwards (16 = central\n")
        f.write("# section), 17 ... 31 from the left edge inwards (31 next to 16).\n")
        f.write("# Format:  'SECTION <n> <n vertices>' followed by the <x y> vertices\n")
        f.write("# of the closed polygon (last vertex is NOT repeated).\n")
        for n in sorted(sections):
            poly = sections[n]
            f.write("SECTION %d %d\n" % (n, len(poly)))
            for px, py in poly:
                f.write("  %12.4f %12.4f\n" % (px, py))


def main():
    dxf = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_DXF
    out = sys.argv[2] if len(sys.argv) > 2 else DEFAULT_OUT

    if not os.path.exists(dxf):
        sys.exit("ERROR: can not find the DXF file '%s'" % dxf)

    sections = build_sections(read_dxf_polylines(dxf))
    write_geometry(sections, out, dxf)

    print("Wrote geometry of %d sections into '%s'" % (len(sections), out))
    for n in sorted(sections):
        xs = [px for px, _ in sections[n]]
        print("  section %2d :  x = [%8.2f, %8.2f] mm  (%2d vertices)"
              % (n, min(xs), max(xs), len(sections[n])))


if __name__ == "__main__":
    main()
