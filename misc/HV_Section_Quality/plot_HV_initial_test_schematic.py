#!/usr/bin/env python3
"""
Schematic of the *initial* HV tests of the uRwell 2nd prototype.

These are NOT the section by section measurements (see plot_HV_sections.py).
Here all HV jumpers are in place, so one complete half of a detector - all the
sections that are powered from the same side - hangs on a single CAEN HV
channel:

    CAEN HV pin  --->  MESH of the half
    RESIST of the half  --->  input of a Keithley picoammeter
    Keithley ground  --->  earth ground

Two picoammeters were available, so two halves were measured at a time, and
they were always taken from different detectors and from opposite sides:

    configuration A :  TOP-LEFT   +  BOTTOM-RIGHT
    configuration B :  TOP-RIGHT  +  BOTTOM-LEFT

Section numbering (see CLAUDE.md / extract_section_geometry.py): sections 1-16
are powered from one side of the foil, sections 17-31 from the other one, both
counted from the outer edge inwards.  On the TOP detector 1-16 sit on the right
and 17-31 on the left; the BOTTOM detector uses the same foil flipped left <->
right, so there 1-16 sit on the left and 17-31 on the right.  Therefore
"TOP-LEFT + BOTTOM-RIGHT" is the 17-31 group of both detectors and
"TOP-RIGHT + BOTTOM-LEFT" is the 1-16 group of both.

A third figure shows the *production* connection: the left and the right side
of a detector are tied together by a 2 wire cable (MESH-MESH, RESIST-RESIST),
so all 31 sections hang on one single CAEN channel per detector.  The core of
the HV cable is soldered onto the MESH, the RESIST goes to earth ground; there
is no picoammeter any more.  The TOP detector is fed from its right side, the
BOTTOM detector from its left side.  That figure is the 3D sketch alone.

Each figure has a 3D sketch of the two stacked detector planes (true trapezoid
shape and section pitch, TOP drawn blue, BOTTOM drawn red, the parts that are
under HV filled with the strong color); the two initial test figures also have
the wiring of the measuring chains underneath.

The 3D view is an orthographic projection done by hand (see 'project') and
drawn into an ordinary 2D axes.  mplot3d is not used on purpose: it forces its
axes into a square and clips everything that falls outside of it, which wastes
most of the width for an object as elongated as this detector.

Usage:
    ./plot_HV_initial_test_schematic.py
    ./plot_HV_initial_test_schematic.py --output-dir SomeDir --elev 20 --azim 18
"""

import argparse
import os
import sys

import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon, Rectangle, FancyBboxPatch, Patch, FancyArrowPatch

CAEN = "CAEN A1536HDM"       # the HV module both channels come from

DEF_GEOM = "uRwell_HV_section_geometry.dat"
DEF_OUTDIR = "Figs"

TOP_COLOR = "#1f5fbf"        # blue  - TOP detector
BOT_COLOR = "#c8202e"        # red   - BOTTOM detector
OFF_COLOR = "#dedede"        # grey  - half that is not powered in this test
EDGE_COLOR = "#404040"

Z_TOP, Z_BOT = 170., -170.   # [mm] drawing distance of the two planes, not to scale
ZP_TOP, ZP_BOT = 300., -300.  # the same for the production sketch, which needs room
                              # for the link cable running in front of each plane

PAD_Y = (60., -50.)          # [mm] height of the MESH / RESIST solder points
PAD_OUT = 20.                # [mm] they sit this far outside the active area

LEFT_SECS = tuple(range(17, 32))    # 15 sections, outer edge -> center
RIGHT_SECS = tuple(range(1, 17))    # 16 sections, outer edge -> center (16 = central)
ALL_SECS = set(LEFT_SECS) | set(RIGHT_SECS)


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


# ----------------------------------------------------------------- small helpers
def outline(sections):
    """The 4 corners of the trapezoidal active area, counter clockwise."""
    v = np.vstack(list(sections.values()))
    ytop, ybot = v[:, 1].max(), v[:, 1].min()
    top = v[np.abs(v[:, 1] - ytop) < 1.]
    bot = v[np.abs(v[:, 1] - ybot) < 1.]
    return np.array([(bot[:, 0].min(), ybot), (bot[:, 0].max(), ybot),
                     (top[:, 0].max(), ytop), (top[:, 0].min(), ytop)])


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


# ------------------------------------------------------------------ the 3D sketch
def project(pts, elev, azim):
    """Orthographic projection of (N, 3) points, the detector seen from above.

    'azim' turns the detector around the vertical axis, 'elev' lifts the eye
    above the detector plane; both in degrees.
    """
    pts = np.atleast_2d(np.asarray(pts, float))
    a, e = np.radians(azim), np.radians(elev)
    x, y, z = pts[:, 0], pts[:, 1], pts[:, 2]
    return np.column_stack((x * np.cos(a) + y * np.sin(a),
                            (-x * np.sin(a) + y * np.cos(a)) * np.sin(e)
                            + z * np.cos(e)))


def draw_plane(ax, sections, z, powered, color, name, view, zorder, name_x=0.):
    """Draw one detector plane at height z, the sections in 'powered' filled."""
    for n in sorted(sections):
        p = sections[n]
        xyz = np.column_stack((p[:, 0], p[:, 1], np.full(len(p), z)))
        ax.add_patch(Polygon(project(xyz, *view), closed=True,
                             facecolor=color if n in powered else OFF_COLOR,
                             edgecolor=EDGE_COLOR, linewidth=.4, zorder=zorder))

    # outer contour of the active area, heavier than the section borders
    corner = outline(sections)
    xyz = np.column_stack((corner[:, 0], corner[:, 1], np.full(len(corner), z)))
    ax.add_patch(Polygon(project(xyz, *view), closed=True, fill=False,
                         edgecolor="black", linewidth=1.5, zorder=zorder + .1))

    # label the two halves rather than the 31 single sections, and keep the
    # labels small so that the section pattern stays visible underneath
    for group in (LEFT_SECS, RIGHT_SECS):
        col = color if group[0] in powered else "0.3"
        cx = np.mean([centroid(sections[n])[0] for n in group])
        sx, sy = project((cx, 0., z), *view)[0]
        ax.text(sx, sy, "sections %d-%d" % (min(group), max(group)),
                ha="center", va="center", fontsize=11, fontweight="bold",
                color=col, zorder=zorder + .2,
                bbox=dict(boxstyle="round,pad=0.25", fc="white", ec=col, alpha=.85))

    off = [n for n in sections if n not in powered]
    if off:
        # name of the detector, off the outer end of the half that is not powered
        xoff = [centroid(sections[n])[0] for n in off]
        xname = max(xoff) + 260. if np.mean(xoff) > 0. else min(xoff) - 260.
        sxy = project((xname, 0., z), *view)[0]
    else:
        # everything is powered (production connection) -> no free end, put the
        # name behind the wide edge, the link cable runs in front of the plane
        sxy = project((name_x, corner[:, 1].max() + 130., z), *view)[0]
    ax.text(sxy[0], sxy[1], name, ha="center", va="center", fontsize=15,
            fontweight="bold", color=color, zorder=zorder + .2)

    return np.vstack((project(xyz, *view), sxy))


def edge_x(sections, side):
    """x of the slanted outer edge of the given side, as a function of y."""
    corner = outline(sections)
    (x0, y0), (x1, y1) = (corner[1], corner[2]) if side > 0 else (corner[0], corner[3])
    return lambda y: x0 + (y - y0) / (y1 - y0) * (x1 - x0)


def pad_x(sections, side, y):
    """x of a solder point at height y, PAD_OUT outside the active area."""
    return edge_x(sections, side)(y) + side * PAD_OUT


def solder_point(ax, xy, color, zorder):
    ax.plot(xy[0], xy[1], "o", ms=6., mfc="white", mec=color, mew=1.6, zorder=zorder)


def draw_leads(ax, sections, z, powered, color, side, ch, pico, view, zorder):
    """The two leads of a powered half: CAEN -> MESH and RESIST -> picoammeter.

    Both solder points sit next to each other on the outer edge of the half, the
    way they do on the detector; the two leads only separate once they have left
    the active area.
    """
    xe = edge_x(sections, side)
    leads = ((PAD_Y[0], 150., "MESH  " + r"$\leftarrow$" + "  %s, %s" % (CAEN, ch)),
             (PAD_Y[1], -150., "RESIST  " + r"$\rightarrow$" + "  Keithley %s" % pico))
    anchors = []
    for y, dz, txt in leads:
        path = project([(pad_x(sections, side, y), y, z), (xe(y) + side * 110., y, z),
                        (xe(y) + side * 230., y, z + dz)], *view)
        ax.plot(path[:, 0], path[:, 1], color=color, lw=2.,
                solid_capstyle="round", zorder=zorder + .3)
        solder_point(ax, path[0], color, zorder + .3)
        ax.text(path[-1, 0] + side * 25., path[-1, 1], txt,
                ha="left" if side > 0 else "right", va="center",
                fontsize=10.5, color=color, zorder=zorder + .3)
        anchors.append((path[-1, 0] + side * 25., path[-1, 1]))
    return np.array(anchors)


def earth_symbol(ax, xy, color, zorder, w=170.):
    """Earth ground symbol hanging from the end of a wire."""
    x, y = xy
    ax.plot([x, x], [y, y - 70.], color=color, lw=2., solid_capstyle="round",
            zorder=zorder)
    for k, f in enumerate((1., .62, .3)):
        ax.plot([x - .5 * f * w, x + .5 * f * w], [y - 70. - 32. * k] * 2,
                color=color, lw=2.4 - .55 * k, solid_capstyle="butt", zorder=zorder)


def draw_hv_cable(ax, sections, z, color, side, ch, view, zorder):
    """Production connection: the HV cable soldered onto one side of the foil.

    The core of the cable goes to the MESH, so the detector needs one single CAEN
    channel; the RESIST goes to earth ground.
    """
    xe = edge_x(sections, side)
    leads = ((PAD_Y[0], 150., "MESH  " + r"$\leftarrow$" + "  HV core\n%s, %s"
              % (CAEN, ch)),
             (PAD_Y[1], -150., "RESIST  " + r"$\rightarrow$" + "  earth ground"))
    anchors = []
    for y, dz, txt in leads:
        path = project([(pad_x(sections, side, y), y, z), (xe(y) + side * 110., y, z),
                        (xe(y) + side * 230., y, z + dz)], *view)
        ax.plot(path[:, 0], path[:, 1], color=color, lw=2.,
                solid_capstyle="round", zorder=zorder + .3)
        anchors.append((path[-1, 0] + side * 30., path[-1, 1]))
        if dz > 0.:
            ax.text(anchors[-1][0], anchors[-1][1], txt, va="center",
                    ha="left" if side > 0 else "right",
                    fontsize=10.5, color=color, zorder=zorder + .3)
        else:
            earth_symbol(ax, path[-1], color, zorder + .3)
            ax.text(anchors[-1][0], anchors[-1][1] + 12., txt, va="bottom",
                    ha="left" if side > 0 else "right",
                    fontsize=10.5, color=color, zorder=zorder + .3)
            anchors.append((path[-1, 0], path[-1, 1] - 145.))
    return np.array(anchors)


def draw_link_cable(ax, sections, z, color, view, zorder):
    """The 2 wire cable that ties the two sides of a detector together.

    MESH(left) - MESH(right) and RESIST(left) - RESIST(right), so that all 31
    sections end up on the same pair of nodes.  It is drawn running in front of
    the plane, one lane per wire.
    """
    ylo = outline(sections)[:, 1].min()
    pts = []
    for k, y in enumerate(PAD_Y):
        xr, xl = pad_x(sections, +1, y), pad_x(sections, -1, y)
        # the cable hangs a bit below the foil, so that it passes under the
        # leads of the HV cable instead of running into them
        lane, zc = ylo - 90. - 50. * k, z - 55.
        path = project([(xr, y, z), (xr + 60., y, zc), (xr + 60., lane, zc),
                        (xl - 60., lane, zc), (xl - 60., y, zc), (xl, y, z)], *view)
        ax.plot(path[:, 0], path[:, 1], color=color, lw=1.8,
                solid_capstyle="round", zorder=zorder + .25)
        solder_point(ax, path[0], color, zorder + .3)
        solder_point(ax, path[-1], color, zorder + .3)
        pts.append(path)

    # the label runs parallel to the cable, otherwise it cuts through the wires
    p0, p1 = project([(0., 0., z), (100., 0., z)], *view)
    ang = np.degrees(np.arctan2(p1[1] - p0[1], p1[0] - p0[0]))
    sxy = project((0., ylo - 340., z), *view)[0]
    ax.text(sxy[0], sxy[1], "link cable, 2 wires:   MESH " + r"$-$" + " MESH,   "
                            "RESIST " + r"$-$" + " RESIST",
            ha="center", va="top", rotation=ang, rotation_mode="anchor",
            fontsize=10.5, color=color, zorder=zorder + .3)
    return np.vstack(pts + [sxy])


def fit_limits(ax, pts, padx, pady):
    """Frame 'pts' with the given margins and stretch to the shape of the panel.

    The limits are matched to the physical aspect ratio of the axes by hand, so
    that x and y keep the same scale.  set_aspect('equal') is deliberately not
    used: with adjustable='box' it shrinks the panel, with adjustable='datalim'
    the result depends on the data limits of the patches and the two
    configurations then come out at different scales.
    """
    xlo, xhi = pts[:, 0].min() - padx, pts[:, 0].max() + padx
    ylo, yhi = pts[:, 1].min() - pady, pts[:, 1].max() + pady

    fw, fh = ax.figure.get_size_inches()
    bb = ax.get_position()
    panel = (bb.width * fw) / (bb.height * fh)

    if (xhi - xlo) / (yhi - ylo) < panel:
        pad = .5 * ((yhi - ylo) * panel - (xhi - xlo))
        xlo, xhi = xlo - pad, xhi + pad
    else:
        pad = .5 * ((xhi - xlo) / panel - (yhi - ylo))
        ylo, yhi = ylo - pad, yhi + pad

    ax.set_xlim(xlo, xhi)
    ax.set_ylim(ylo, yhi)
    ax.set_axis_off()


def make_sketch(ax, sec_top, sec_bot, cfg, view):
    """The 3D view; the plane that is farther away is drawn first."""
    if cfg["mode"] == "production":
        pts = [draw_plane(ax, sec_bot, ZP_BOT, ALL_SECS, BOT_COLOR, "BOTTOM", view, 2,
                          name_x=-600. * cfg["side_bot"]),
               draw_link_cable(ax, sec_bot, ZP_BOT, BOT_COLOR, view, 2),
               draw_hv_cable(ax, sec_bot, ZP_BOT, BOT_COLOR, cfg["side_bot"],
                             "ch. B", view, 2),
               draw_plane(ax, sec_top, ZP_TOP, ALL_SECS, TOP_COLOR, "TOP", view, 4,
                          name_x=-600. * cfg["side_top"]),
               draw_link_cable(ax, sec_top, ZP_TOP, TOP_COLOR, view, 4),
               draw_hv_cable(ax, sec_top, ZP_TOP, TOP_COLOR, cfg["side_top"],
                             "ch. A", view, 4)]
    else:
        pts = [draw_plane(ax, sec_bot, Z_BOT, cfg["powered"], BOT_COLOR, "BOTTOM", view, 2),
               draw_leads(ax, sec_bot, Z_BOT, cfg["powered"], BOT_COLOR, cfg["side_bot"],
                          "ch. B", "# 2", view, 2),
               draw_plane(ax, sec_top, Z_TOP, cfg["powered"], TOP_COLOR, "TOP", view, 4),
               draw_leads(ax, sec_top, Z_TOP, cfg["powered"], TOP_COLOR, cfg["side_top"],
                          "ch. A", "# 1", view, 4)]

    # padx has to hold the lead label, it is not decoration
    fit_limits(ax, np.vstack(pts), padx=560. if cfg["mode"] == "production" else 420.,
               pady=70.)


# ------------------------------------------------------------------ the wiring
def box(ax, x, y, w, h, label, fc="#f2f2f2", ec="0.25", fs=9., lw=1.3):
    ax.add_patch(FancyBboxPatch((x, y), w, h, boxstyle="round,pad=0,rounding_size=1.2",
                                facecolor=fc, edgecolor=ec, linewidth=lw, zorder=3))
    ax.text(x + .5 * w, y + .5 * h, label, ha="center", va="center",
            fontsize=fs, zorder=4)


def wire(ax, pts, color="0.15", lw=1.6, ls="-", arrow=True):
    pts = np.asarray(pts, float)
    ax.plot(pts[:, 0], pts[:, 1], color=color, lw=lw, ls=ls,
            solid_capstyle="round", zorder=2)
    if arrow:
        ax.add_patch(FancyArrowPatch(pts[-2], pts[-1], arrowstyle="-|>",
                                     mutation_scale=12, color=color, lw=lw, zorder=2))


def ground(ax, x, y, color="0.15", label=None):
    """Earth ground symbol, the stem ends at (x, y)."""
    for k, w in enumerate((6.0, 3.8, 1.8)):
        ax.plot([x - .5 * w, x + .5 * w], [y - 2.2 * k, y - 2.2 * k],
                color=color, lw=2.0 - .45 * k, solid_capstyle="butt", zorder=3)
    if label:
        ax.text(x, y - 8.5, label, ha="center", va="top", fontsize=9., color=color)


def draw_chain(ax, yc, color, det, half, secs, nsec, ch, pico):
    """One measuring chain: CAEN channel -> MESH / RESIST -> picoammeter -> earth."""
    box(ax, 1., yc - 9., 19., 18., "%s\n%s" % (CAEN, ch), fs=9.5)

    # the detector half, drawn as its two electrodes with the gas gap in between
    ax.add_patch(Rectangle((31., yc - 11.), 30., 22., facecolor=color, alpha=.09,
                           edgecolor=color, linewidth=1.1, linestyle="--", zorder=1))
    ax.add_patch(Rectangle((36., yc + 4.), 20., 2.4, facecolor="0.35",
                           edgecolor="0.15", linewidth=.8, zorder=3))
    ax.add_patch(Rectangle((36., yc - 6.4), 20., 2.4, facecolor=color,
                           edgecolor="0.15", linewidth=.8, zorder=3))
    ax.text(46., yc + 7.2, "MESH", ha="center", va="bottom", fontsize=9.5,
            fontweight="bold")
    ax.text(46., yc - 7.6, "RESIST", ha="center", va="top", fontsize=9.5,
            fontweight="bold", color=color)
    ax.text(46., yc, "amplification gap", ha="center", va="center",
            fontsize=8., color="0.35", style="italic")
    ax.text(46., yc + 14.5, "%s detector, %s half" % (det, half), ha="center",
            va="bottom", fontsize=11.5, fontweight="bold", color=color)
    ax.text(46., yc + 12., "sections %s - all %d bridged by the HV jumpers, "
                           "so they form one single node" % (secs, nsec),
            ha="center", va="bottom", fontsize=9., color="0.35")

    # CAEN -> MESH
    wire(ax, [(20., yc + 5.2), (36., yc + 5.2)])
    ax.text(28., yc + 6.4, "HV", ha="center", va="bottom", fontsize=9.5)

    # RESIST -> picoammeter input
    wire(ax, [(56., yc - 5.2), (68., yc - 5.2), (68., yc), (73., yc)])
    box(ax, 73., yc - 9., 22., 18., "Keithley\npicoammeter %s" % pico, fs=10.)
    ax.text(71.5, yc + 1., "IN", ha="right", va="bottom", fontsize=8.5, color="0.3")

    # the ground of the picoammeter goes to earth - the two chains are otherwise
    # completely independent, nothing else is common between them
    wire(ax, [(84., yc - 9.), (84., yc - 17.)], arrow=False)
    ground(ax, 84., yc - 17., label="earth ground")


def make_wiring(ax, cfg):
    ax.set_xlim(0., 100.)
    ax.set_ylim(0., 100.)
    ax.set_axis_off()

    draw_chain(ax, 71., TOP_COLOR, "TOP", cfg["top_half"], cfg["top_secs"],
               cfg["nsec"], "channel A", "# 1")
    draw_chain(ax, 30., BOT_COLOR, "BOTTOM", cfg["bot_half"], cfg["bot_secs"],
               cfg["nsec"], "channel B", "# 2")

    ax.text(50., 2., "The two chains are independent: separate CAEN channel, separate "
                     "picoammeter.  Only the picoammeter grounds go to earth.",
            ha="center", va="bottom", fontsize=9.5, style="italic", color="0.35")


# ------------------------------------------------------------------------- main
def make_figure(sections, cfg, args):
    # the BOTTOM detector is the same foil, flipped left <-> right
    sec_top = sections
    sec_bot = {n: p * np.array([-1., 1.]) for n, p in sections.items()}

    if cfg["mode"] == "production":
        # no measuring chain to draw -> the 3D sketch is the whole figure
        fig = plt.figure(figsize=(16., 8.))
        ax3 = fig.add_axes([.01, .02, .98, .82])
        handles = [Patch(facecolor=TOP_COLOR, edgecolor=EDGE_COLOR,
                         label="TOP detector, all 31 sections under HV"),
                   Patch(facecolor=BOT_COLOR, edgecolor=EDGE_COLOR,
                         label="BOTTOM detector, all 31 sections under HV")]
    else:
        fig = plt.figure(figsize=(16., 11.))
        ax3 = fig.add_axes([.01, .42, .98, .48])
        make_wiring(fig.add_axes([.02, .015, .96, .40]), cfg)
        handles = [Patch(facecolor=TOP_COLOR, edgecolor=EDGE_COLOR,
                         label="TOP detector, half under HV"),
                   Patch(facecolor=BOT_COLOR, edgecolor=EDGE_COLOR,
                         label="BOTTOM detector, half under HV"),
                   Patch(facecolor=OFF_COLOR, edgecolor=EDGE_COLOR,
                         label="not powered in this test")]

    make_sketch(ax3, sec_top, sec_bot, cfg, (args.elev, args.azim))

    fig.suptitle("uRwell 2$^{nd}$ prototype - %s" % cfg["title"],
                 fontsize=17, fontweight="bold", y=.99)

    fig.legend(handles=handles, loc="upper center",
               bbox_to_anchor=(.5, .90 if cfg["mode"] == "production" else .935),
               ncol=len(handles), fontsize=10.5, frameon=False)

    os.makedirs(args.output_dir, exist_ok=True)
    for ext in ("png", "pdf"):
        fname = os.path.join(args.output_dir, "%s.%s" % (cfg["out"], ext))
        fig.savefig(fname, dpi=args.dpi)
        print("Wrote %s" % fname)
    plt.close(fig)


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--geometry", default=DEF_GEOM, help="section geometry file")
    ap.add_argument("--output-dir", "-o", default=DEF_OUTDIR,
                    help="directory the figures are written into (created if missing)")
    ap.add_argument("--elev", type=float, default=20.,
                    help="height of the eye above the detector plane [deg]")
    ap.add_argument("--azim", type=float, default=17.,
                    help="rotation of the detector around the vertical axis [deg]")
    ap.add_argument("--dpi", type=int, default=200, help="resolution of the png")
    args = ap.parse_args()

    if not os.path.exists(args.geometry):
        sys.exit("ERROR: can not find '%s'" % args.geometry)
    sections = read_geometry(args.geometry)

    # 'side' = +1 if that half sits at x > 0 in the drawing, -1 if at x < 0.
    # TOP:    sections 17-31 left  (x < 0),  1-16 right (x > 0)
    # BOTTOM: mirrored, so       17-31 right (x > 0),  1-16 left  (x < 0)
    configs = [
        dict(out="HV_InitialTest_TopLeft_BotRight", mode="half",
             title="initial HV test, all HV jumpers in place\nconfiguration A:   "
                   "TOP-LEFT  +  BOTTOM-RIGHT   (sections 17-31 of both detectors)",
             powered=set(LEFT_SECS), nsec=len(LEFT_SECS), side_top=-1, side_bot=+1,
             top_half="LEFT", bot_half="RIGHT", top_secs="17-31", bot_secs="17-31"),
        dict(out="HV_InitialTest_TopRight_BotLeft", mode="half",
             title="initial HV test, all HV jumpers in place\nconfiguration B:   "
                   "TOP-RIGHT  +  BOTTOM-LEFT   (sections 1-16 of both detectors)",
             powered=set(RIGHT_SECS), nsec=len(RIGHT_SECS), side_top=+1, side_bot=-1,
             top_half="RIGHT", bot_half="LEFT", top_secs="1-16", bot_secs="1-16"),
        # production: left and right side of a detector tied together by a 2 wire
        # cable, one CAEN channel per detector, TOP fed from the right side and
        # BOTTOM from the left side
        dict(out="HV_Production_Connection", mode="production",
             title="production HV connection\nboth sides bridged by a 2 wire cable, "
                   "one %s channel per detector, no picoammeter" % CAEN,
             powered=ALL_SECS, side_top=+1, side_bot=-1),
    ]

    for cfg in configs:
        make_figure(sections, cfg, args)


if __name__ == "__main__":
    main()
