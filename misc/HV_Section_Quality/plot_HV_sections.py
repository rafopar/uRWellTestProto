#!/usr/bin/env python3
"""
Draw the 31 HV sections of the uRwell 2nd prototype with their true shape and
proportions (taken from DFS3381_activearea.dxf, see extract_section_geometry.py)
and color code every section with the maximum HV it can hold in a stable way.

The maximum stable HV is read from two column wise text files, one for the TOP
and one for the BOTTOM detector:

    # SECTION   MAX_STABLE_HV [V]
    1           490
    ...
    9           -1        <-- negative value = section not measured yet

Usage:
    ./plot_HV_sections.py                                # uses the default files
    ./plot_HV_sections.py --top TOP_...dat --bot BOT_...dat --out HV_sections
    ./plot_HV_sections.py --cmap RdYlGn --vmin 440 --vmax 530
"""

import argparse
import os
import sys

import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon, Patch
from matplotlib.colors import Normalize, to_rgba
from matplotlib.cm import ScalarMappable

DEF_GEOM = "uRwell_HV_section_geometry.dat"
DEF_TOP = "TOP_HV_Sections_90Ar_7Iso_3CO2.dat"
DEF_BOT = "BOT_HV_Sections_90Ar_7Iso_3CO2.dat"
DEF_GAS = r"90% Ar + 7% iso-C$_4$H$_{10}$ + 3% CO$_2$"

NA_COLOR = "0.87"       # fill color of the sections that are not measured yet


# ----------------------------------------------------------------------------- I/O
def read_geometry(fname):
    """Read the section polygons written by extract_section_geometry.py."""
    sections, cur = {}, None
    with open(fname) as f:
        for line in f:
            line = line.split("#")[0].strip()
            if not line:
                continue
            if line.startswith("SECTION"):
                cur = int(line.split()[1])
                sections[cur] = []
            else:
                x, y = line.split()
                sections[cur].append((float(x), float(y)))
    return {n: np.array(p) for n, p in sections.items()}


def read_hv(fname):
    """Read 'section  max_stable_HV' pairs, negative HV means 'not measured'."""
    hv = {}
    with open(fname) as f:
        for line in f:
            line = line.split("#")[0].strip()
            if not line:
                continue
            w = line.split()
            hv[int(w[0])] = float(w[1])
    return hv


# ----------------------------------------------------------------- small helpers
def centroid(poly):
    """Area weighted centroid of a closed polygon."""
    x, y = poly[:, 0], poly[:, 1]
    xn, yn = np.roll(x, -1), np.roll(y, -1)
    cross = x * yn - xn * y
    area = 0.5 * cross.sum()
    if abs(area) < 1e-9:
        return x.mean(), y.mean()
    return (((x + xn) * cross).sum() / (6. * area),
            ((y + yn) * cross).sum() / (6. * area))


def text_color(rgba):
    """Black or white, whichever is more readable on top of the given color."""
    r, g, b = rgba[:3]
    return "black" if (0.299 * r + 0.587 * g + 0.114 * b) > 0.55 else "white"


# ------------------------------------------------------------------- the drawing
def draw_detector(ax, sections, hv, cmap, norm, title, mirror=False):
    allx = np.concatenate([p[:, 0] for p in sections.values()])
    ally = np.concatenate([p[:, 1] for p in sections.values()])
    xlo, xhi, ylo, yhi = allx.min(), allx.max(), ally.min(), ally.max()

    for n in sorted(sections):
        poly = sections[n]
        value = hv.get(n, -1.)
        measured = value > 0.

        color = cmap(norm(value)) if measured else to_rgba(NA_COLOR)
        if measured:
            ax.add_patch(Polygon(poly, closed=True, facecolor=color,
                                 edgecolor="black", linewidth=0.6, zorder=2))
        else:
            # light gray hatch (the hatch takes the edge color) plus a separate
            # black outline, so that the 'n/a' label stays readable
            ax.add_patch(Polygon(poly, closed=True, facecolor=color,
                                 edgecolor="0.62", linewidth=0., hatch="///", zorder=2))
            ax.add_patch(Polygon(poly, closed=True, fill=False,
                                 edgecolor="black", linewidth=0.6, zorder=2.1))

        tcol = text_color(color) if measured else "0.15"
        cx, cy = centroid(poly)
        ax.text(cx, cy + 32., "%d" % n, ha="center", va="center",
                fontsize=7., color=tcol, zorder=3)
        ax.text(cx, cy - 14., "%d" % round(value) if measured else "n/a",
                ha="center", va="center", fontsize=9., fontweight="bold",
                color=tcol, zorder=3)

    # the bottom corners of the trapezoid are empty -> remind the reader there
    # on which side of the detector the sections are powered
    left, right = ("1-16", "17-31") if mirror else ("17-31", "1-16")
    ax.text(xlo + 15., ylo + 45., "sections %s\npowered from\nthis side" % left,
            ha="left", va="bottom", fontsize=8.5, style="italic", color="0.30")
    ax.text(xhi - 15., ylo + 45., "sections %s\npowered from\nthis side" % right,
            ha="right", va="bottom", fontsize=8.5, style="italic", color="0.30")

    ax.set_xlim(xlo - 25., xhi + 25.)
    ax.set_ylim(ylo - 25., yhi + 25.)
    ax.set_aspect("equal")
    ax.set_title(title, fontsize=13, fontweight="bold", pad=6)
    ax.set_xlabel("x [mm]", fontsize=10)
    ax.set_ylabel("y [mm]", fontsize=10)
    ax.tick_params(labelsize=9)


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--geometry", default=DEF_GEOM, help="section geometry file")
    ap.add_argument("--top", default=DEF_TOP, help="max stable HV of the TOP detector")
    ap.add_argument("--bot", default=DEF_BOT, help="max stable HV of the BOTTOM detector")
    ap.add_argument("--out", default="HV_Section_MaxStableHV",
                    help="output file name without extension")
    ap.add_argument("--cmap", default="viridis", help="matplotlib color map")
    ap.add_argument("--vmin", type=float, default=None, help="lower end of the color scale [V]")
    ap.add_argument("--vmax", type=float, default=None, help="upper end of the color scale [V]")
    ap.add_argument("--gas", default=DEF_GAS, help="gas mixture shown in the title")
    ap.add_argument("--mirror", action="store_true",
                    help="mirror the detector in x, i.e. draw section 1 on the left "
                         "(use it if you look at the chamber from the other side)")
    args = ap.parse_args()

    for f in (args.geometry, args.top, args.bot):
        if not os.path.exists(f):
            sys.exit("ERROR: can not find '%s'" % f)

    sections = read_geometry(args.geometry)
    if args.mirror:
        sections = {n: p * np.array([-1., 1.]) for n, p in sections.items()}
    hv_top, hv_bot = read_hv(args.top), read_hv(args.bot)

    measured = [v for v in list(hv_top.values()) + list(hv_bot.values()) if v > 0.]
    if not measured:
        sys.exit("ERROR: none of the sections has a measured HV value")

    # one common color scale for both detectors, so that they can be compared
    vmin = args.vmin if args.vmin is not None else min(measured) - 10.
    vmax = args.vmax if args.vmax is not None else max(measured) + 10.
    norm = Normalize(vmin=vmin, vmax=vmax)
    cmap = plt.get_cmap(args.cmap)

    # the active area is 1453 x 495 mm^2, keep the true aspect ratio of the
    # detector and size the canvas such that no space is wasted
    fig, axes = plt.subplots(2, 1, figsize=(17., 13.2), layout="constrained")
    draw_detector(axes[0], sections, hv_top, cmap, norm, "TOP detector", args.mirror)
    draw_detector(axes[1], sections, hv_bot, cmap, norm, "BOTTOM detector", args.mirror)

    fig.suptitle("uRwell 2$^{nd}$ prototype - maximum stable HV per section\n%s"
                 % args.gas, fontsize=15, fontweight="bold")

    sm = ScalarMappable(norm=norm, cmap=cmap)
    sm.set_array([])
    cbar = fig.colorbar(sm, ax=axes.tolist(), orientation="vertical",
                        fraction=0.025, pad=0.01, aspect=40)
    cbar.set_label("Maximum stable HV [V]", fontsize=11)
    cbar.ax.tick_params(labelsize=10)

    fig.legend(handles=[Patch(facecolor=NA_COLOR, edgecolor="0.62",
                              hatch="///", label="section not measured yet")],
               loc="outside lower left", fontsize=10, frameon=False)

    for ext in ("png", "pdf"):
        fig.savefig("%s.%s" % (args.out, ext), dpi=200)
        print("Wrote %s.%s" % (args.out, ext))

    n_top = sum(1 for v in hv_top.values() if v > 0.)
    n_bot = sum(1 for v in hv_bot.values() if v > 0.)
    print("Measured sections: TOP %d/31, BOTTOM %d/31   (color scale %.0f - %.0f V)"
          % (n_top, n_bot, vmin, vmax))


if __name__ == "__main__":
    main()
