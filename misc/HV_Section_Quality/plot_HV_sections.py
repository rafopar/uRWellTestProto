#!/usr/bin/env python3
"""
Draw the 31 HV sections of the uRwell 2nd prototype with their true shape and
proportions (taken from DFS3381_activearea.dxf, see extract_section_geometry.py)
and color code every section with the maximum HV it can hold in a stable way.

The measurements are done separately for every gas.  For each gas the maximum
stable HV is read from two column wise text files, one for the TOP and one for
the BOTTOM detector, named TOP_HV_Sections_<gas>.dat and BOT_HV_Sections_<gas>.dat:

    # SECTION   MAX_STABLE_HV [V]   TRIP_LIMIT [uA]
    1           490                 15
    ...
    9           -1                        <-- negative value = section not measured yet

The third column, the HV trip limit the section was measured with, is optional.
A section measured with a lower trip limit than the highest one used for that gas
may have tripped below what it can really hold, it gets a small red 'x'.

The known gases are listed in GASES below, a new gas needs one more line there.
Without --gas every gas whose two files exist is drawn, one figure per gas.  The
figures are written into the 'Figs' sub directory, which is created if it does
not exist yet, as HV_Section_MaxStableHV_<gas>.png/.pdf.

Every gas gets its own color scale, by default from its lowest HV - 10 V up to its
highest HV + 10 V.  With --delta the scale of every gas has the same width instead,
from its highest HV down to highest HV - DELTA, so the colors of different gases can
be compared.  --delta without a value takes the widest range of all gases.

Usage:
    ./plot_HV_sections.py                                # all gases
    ./plot_HV_sections.py --gas UHPN2                    # only one gas
    ./plot_HV_sections.py --delta                        # same color scale width for all gases
    ./plot_HV_sections.py --delta 100                    # ... with a given width [V]
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
    """Read 'section  max_stable_HV  [trip_limit]' lines, negative HV means 'not
    measured'.  Returns the HV and the trip limit dicts, the latter only holds the
    sections for which the (optional) trip limit is given."""
    hv, trip = {}, {}
    with open(fname) as f:
        for line in f:
            line = line.split("#")[0].strip()
            if not line:
                continue
            w = line.split()
            hv[int(w[0])] = float(w[1])
            if len(w) > 2:
                trip[int(w[0])] = float(w[2])
    return hv, trip


def hv_files(gas):
    """The TOP and BOTTOM measurement files of the given gas."""
    return "TOP_HV_Sections_%s.dat" % gas, "BOT_HV_Sections_%s.dat" % gas


def measured_hv(gas):
    """All measured HV values of the given gas, TOP and BOTTOM together."""
    return [v for f in hv_files(gas) for v in read_hv(f)[0].values() if v > 0.]


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


def delta_value(s):
    """Value of --delta: 'auto' or a positive width of the color scale in V."""
    if s == "auto":
        return s
    d = float(s)
    if d <= 0.:
        raise argparse.ArgumentTypeError("must be positive, got %s" % s)
    return d


# ------------------------------------------------------------------- the drawing
def draw_detector(ax, sections, hv, cmap, norm, title, mirror=False, low_trip=()):
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

        # measured with a low trip limit -> small red 'x' at the top of the section,
        # centered on its top edge (the outer sections are clipped triangles)
        if measured and n in low_trip:
            ytop = poly[:, 1].max()
            xtop = poly[poly[:, 1] > ytop - 1., 0]
            ax.plot(0.5 * (xtop.min() + xtop.max()), ytop - 25., "x", color="red",
                    markersize=8., markeredgewidth=2., zorder=3)

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
    (hv_top, trip_top), (hv_bot, trip_bot) = read_hv(top_file), read_hv(bot_file)

    measured = [v for v in list(hv_top.values()) + list(hv_bot.values()) if v > 0.]
    if not measured:
        print("WARNING: %s: none of the sections has a measured HV value, skipped" % gas)
        return

    # one common color scale for both detectors, so that they can be compared.
    # Different gases need very different voltages, every gas gets its own scale,
    # with --delta all of them have the same width, from their maximum HV downwards
    if args.delta is not None:
        vmax = max(measured)
        vmin = vmax - args.delta
    else:
        vmin = args.vmin if args.vmin is not None else min(measured) - 10.
        vmax = args.vmax if args.vmax is not None else max(measured) + 10.
    norm = Normalize(vmin=vmin, vmax=vmax)

    # a section measured with a lower trip limit than the highest one used for this
    # gas may have tripped before reaching the HV it can really hold
    max_trip = max(list(trip_top.values()) + list(trip_bot.values()), default=0.)
    low_top = sorted(n for n, t in trip_top.items() if t < max_trip and hv_top[n] > 0.)
    low_bot = sorted(n for n, t in trip_bot.items() if t < max_trip and hv_bot[n] > 0.)

    # the active area is 1453 x 495 mm^2, keep the true aspect ratio of the
    # detector and size the canvas such that no space is wasted
    fig, axes = plt.subplots(2, 1, figsize=(17., 13.2), layout="constrained")
    draw_detector(axes[0], sections, hv_top, cmap, norm, "TOP detector", args.mirror, low_top)
    draw_detector(axes[1], sections, hv_bot, cmap, norm, "BOTTOM detector", args.mirror, low_bot)

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
    if low_top or low_bot:
        print("%s: measured with a trip limit below %g uA (red 'x'): TOP %s, BOTTOM %s"
              % (gas, max_trip, low_top or "-", low_bot or "-"))


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
    ap.add_argument("--delta", nargs="?", const="auto", default=None, type=delta_value,
                    metavar="DELTA",
                    help="same width of the color scale [V] for every gas, from the "
                         "maximum HV of the gas down to maximum - DELTA.  Without a "
                         "value the widest range (max - min + 10 V) of all gases is "
                         "used, so that no gas gets clipped")
    ap.add_argument("--mirror", action="store_true",
                    help="mirror the detector in x, i.e. draw section 1 on the left "
                         "(use it if you look at the chamber from the other side)")
    args = ap.parse_args()

    if args.delta is not None and (args.vmin is not None or args.vmax is not None):
        ap.error("--delta can not be combined with --vmin/--vmax")

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

    # --delta without a value: the widest range of all gases with measurements, drawn
    # or not, so that a figure does not depend on which other gases are drawn
    if args.delta == "auto":
        ranges = []
        for gas in GASES:
            if all(os.path.exists(f) for f in hv_files(gas)):
                values = measured_hv(gas)
                if values:
                    ranges.append(max(values) - min(values) + 10.)
        args.delta = max(ranges, default=10.)
        print("Same color scale width for every gas: %.0f V" % args.delta)

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
