#!/usr/bin/env python3
"""
Draw the 31 HV sections of the uRwell 2nd prototype with their true shape and
proportions (taken from DFS3381_activearea.dxf, see extract_section_geometry.py)
and color code every section with the maximum HV it can hold in a stable way.

The measurements are done separately for every gas.  For each gas the maximum
stable HV is read from two column wise text files, one for the TOP and one for
the BOTTOM detector, named TOP_HV_Sections_<gas>.dat and BOT_HV_Sections_<gas>.dat:

    # SECTION   MAX_STABLE_HV [V]
    1           490
    ...
    9           -1        <-- negative value = section not measured yet

The known gases are listed in GASES below, a new gas needs one more line there.
Without --gas every gas whose two files exist is drawn, one figure per gas.  The
figures are written into the 'Figs' sub directory, which is created if it does
not exist yet, as HV_Section_MaxStableHV_<gas>.png/.pdf.

Usage:
    ./plot_HV_sections.py                                # all gases
    ./plot_HV_sections.py --gas UHPN2                    # only one gas
    ./plot_HV_sections.py --gas 90Ar_7CO2_3Iso --cmap RdYlGn --vmin 440 --vmax 530
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
DEF_OUTDIR = "Figs"

# gas as it appears in the file names  ->  gas as it is written in the title
GASES = {
    "90Ar_7CO2_3Iso": r"90% Ar + 7% CO$_2$ + 3% iso-C$_4$H$_{10}$",
    "UHPN2":          r"ultra high purity N$_2$",
}

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


def hv_files(gas):
    """The TOP and BOTTOM measurement files of the given gas."""
    return "TOP_HV_Sections_%s.dat" % gas, "BOT_HV_Sections_%s.dat" % gas


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


def plot_gas(gas, sections, cmap, args):
    """One figure with both detectors, measured with the given gas."""
    top_file, bot_file = hv_files(gas)
    hv_top, hv_bot = read_hv(top_file), read_hv(bot_file)

    measured = [v for v in list(hv_top.values()) + list(hv_bot.values()) if v > 0.]
    if not measured:
        print("WARNING: %s: none of the sections has a measured HV value, skipped" % gas)
        return

    # one common color scale for both detectors, so that they can be compared.
    # Different gases need very different voltages, every gas gets its own scale
    vmin = args.vmin if args.vmin is not None else min(measured) - 10.
    vmax = args.vmax if args.vmax is not None else max(measured) + 10.
    norm = Normalize(vmin=vmin, vmax=vmax)

    # the active area is 1453 x 495 mm^2, keep the true aspect ratio of the
    # detector and size the canvas such that no space is wasted
    fig, axes = plt.subplots(2, 1, figsize=(17., 13.2), layout="constrained")
    draw_detector(axes[0], sections, hv_top, cmap, norm, "TOP detector", args.mirror)
    draw_detector(axes[1], sections, hv_bot, cmap, norm, "BOTTOM detector", args.mirror)

    fig.suptitle("uRwell 2$^{nd}$ prototype - maximum stable HV per section\n%s"
                 % GASES[gas], fontsize=15, fontweight="bold")

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
        fname = os.path.join(args.output_dir, "%s_%s.%s" % (args.out, gas, ext))
        fig.savefig(fname, dpi=200)
        print("Wrote %s" % fname)
    plt.close(fig)

    n_top = sum(1 for v in hv_top.values() if v > 0.)
    n_bot = sum(1 for v in hv_bot.values() if v > 0.)
    print("%s: measured sections TOP %d/31, BOTTOM %d/31   (color scale %.0f - %.0f V)"
          % (gas, n_top, n_bot, vmin, vmax))


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--gas", nargs="+", choices=list(GASES), default=None,
                    help="gas(es) to draw, default: every gas whose two files exist")
    ap.add_argument("--geometry", default=DEF_GEOM, help="section geometry file")
    ap.add_argument("--output-dir", "-o", default=DEF_OUTDIR,
                    help="directory the figures are written into (created if missing)")
    ap.add_argument("--out", default="HV_Section_MaxStableHV",
                    help="output file name prefix, '_<gas>' and the extension are appended")
    ap.add_argument("--cmap", default="viridis", help="matplotlib color map")
    ap.add_argument("--vmin", type=float, default=None,
                    help="lower end of the color scale [V], applies to every gas drawn")
    ap.add_argument("--vmax", type=float, default=None,
                    help="upper end of the color scale [V], applies to every gas drawn")
    ap.add_argument("--mirror", action="store_true",
                    help="mirror the detector in x, i.e. draw section 1 on the left "
                         "(use it if you look at the chamber from the other side)")
    args = ap.parse_args()

    if not os.path.exists(args.geometry):
        sys.exit("ERROR: can not find '%s'" % args.geometry)

    # without --gas every known gas is drawn for which the measurements exist,
    # a gas that was asked for explicitly has to exist
    gases = []
    for gas in (args.gas or GASES):
        missing = [f for f in hv_files(gas) if not os.path.exists(f)]
        if not missing:
            gases.append(gas)
        elif args.gas:
            sys.exit("ERROR: can not find '%s'" % "', '".join(missing))
        else:
            print("Skipping %s, can not find '%s'" % (gas, "', '".join(missing)))
    if not gases:
        sys.exit("ERROR: no measurement files found for any of the gases %s"
                 % ", ".join(GASES))

    sections = read_geometry(args.geometry)
    if args.mirror:
        sections = {n: p * np.array([-1., 1.]) for n, p in sections.items()}
    cmap = plt.get_cmap(args.cmap)

    # the output directory is created on the fly, so that a fresh checkout or a
    # fresh install directory works right away
    if args.output_dir:
        os.makedirs(args.output_dir, exist_ok=True)

    for gas in gases:
        plot_gas(gas, sections, cmap, args)


if __name__ == "__main__":
    main()
