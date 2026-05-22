#!/usr/bin/env python3
"""
HVScanPlotter.py  –  Analyze and visualize uRWell HV scan data.

Columns in the input file:
  Date  Time  B_uRWell:2:imon  B_uRWell:3:imon
  B_HW_HVURWELL0_Sl04_Ch06:vmon  B_HW_HVURWELL0_Sl04_Ch07:vmon

Usage:
  python HVScanPlotter.py [input_file] [--output-dir Figs] [--no-interactive]
"""

import os
import argparse
from datetime import datetime

import numpy as np
import pandas as pd
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from matplotlib.backends.backend_pdf import PdfPages


# ─── Color coding (fixed per instructions) ────────────────────────────────────
COLOR = {
    'imon2': 'blue',
    'imon3': 'red',
    'hv1':   'black',
    'hv2':   'darkgreen',
}

COLS = ['datetime', 'imon2', 'imon3', 'hv1', 'hv2']

CURRENT_YLIM = (-35, 35)   # nA, applied to all current axes


# ─── Data loading ─────────────────────────────────────────────────────────────
def load_data(path: str) -> pd.DataFrame:
    """
    Read the whitespace-delimited scan file.  The header 'Date' spans two
    space-separated tokens in the data rows (date + time), so we name them
    explicitly and merge them.
    """
    df = pd.read_csv(
        path, sep=r'\s+', header=0,
        names=['date', 'time', 'imon2', 'imon3', 'hv1', 'hv2'],
    )
    df['datetime'] = pd.to_datetime(df['date'] + ' ' + df['time'])
    df = df[COLS].sort_values('datetime').reset_index(drop=True)
    return df


# ─── User prompts ─────────────────────────────────────────────────────────────
def ask_keyword() -> str:
    """Ask for a short dataset keyword used as a prefix on all output filenames."""
    raw = input('\nDataset keyword (used as filename prefix): ').strip()
    # Replace whitespace with underscores so the keyword is safe in filenames
    return raw.replace(' ', '_')


def ask_titles() -> dict:
    defaults = {
        'imon2': 'B_uRWell:2:imon',
        'imon3': 'B_uRWell:3:imon',
        'hv1':   'B_HW_HVURWELL0_Sl04_Ch06:vmon',
        'hv2':   'B_HW_HVURWELL0_Sl04_Ch07:vmon',
    }
    print('\n=== Enter a title for each variable (Enter = keep default) ===')
    return {k: (input(f'  {v}: ').strip() or v) for k, v in defaults.items()}


# ─── Filename helper ──────────────────────────────────────────────────────────
def prefixed(keyword: str, name: str) -> str:
    """Prepend keyword_ to name, or return name unchanged if keyword is empty."""
    return f'{keyword}_{name}' if keyword else name


# ─── Mask / period helpers ────────────────────────────────────────────────────
def transition_mask(
    hv: np.ndarray,
    times: np.ndarray,
    n_consec: int = 3,
    delta: float = 5.0,
    margin_s: int = 60,
) -> np.ndarray:
    """
    Return a boolean array (True = keep) that excludes data within ±margin_s
    seconds of any run of n_consec consecutive readings each showing an HV
    step larger than delta Volts.
    """
    n = len(hv)
    diffs = np.abs(np.diff(hv))          # length n-1
    n_diffs = n_consec - 1               # how many consecutive diffs to check
    margin = np.timedelta64(margin_s, 's')
    bad = np.zeros(n, dtype=bool)

    for i in range(len(diffs) - n_diffs + 1):
        if np.all(diffs[i: i + n_diffs] > delta):
            t0 = times[i] - margin
            t1 = times[i + n_diffs] + margin
            bad |= (times >= t0) & (times <= t1)

    return ~bad


def stable_periods(
    hv: np.ndarray,
    times: np.ndarray,
    delta: float = 2.0,
    margin_s: int = 60,
):
    """
    Generator: yield (hv_mean, boolean_mask) for each plateau where
    consecutive HV readings change by at most delta V, with ±margin_s
    seconds trimmed from each end.
    """
    n = len(hv)
    margin = np.timedelta64(margin_s, 's')
    boundaries = (
        [0]
        + list(np.where(np.abs(np.diff(hv)) > delta)[0] + 1)
        + [n]
    )

    for seg_start, seg_end in zip(boundaries[:-1], boundaries[1:]):
        t_lo = times[seg_start] + margin
        t_hi = times[seg_end - 1] - margin
        if t_lo >= t_hi:
            continue
        mask = (times >= t_lo) & (times <= t_hi)
        if mask.sum() < 5:
            continue
        yield float(np.mean(hv[seg_start:seg_end])), mask


# ─── Plot helpers ─────────────────────────────────────────────────────────────
def fmt_time_axis(ax):
    locator = mdates.AutoDateLocator()
    ax.xaxis.set_major_locator(locator)
    ax.xaxis.set_major_formatter(mdates.ConciseDateFormatter(locator))


def save_fig(fig, out_dir: str, fname: str):
    os.makedirs(out_dir, exist_ok=True)
    path = os.path.join(out_dir, fname)
    fig.savefig(path, dpi=150)
    plt.close(fig)
    print(f'  Saved: {path}')


# ─── Plot 1 & 2: Current + HV vs time (two separate figures) ─────────────────
def plot_time_panels(df: pd.DataFrame, titles: dict, keyword: str, out_dir: str):
    """
    Two dual-panel figures (current on top, HV on bottom) vs time.
    Points where HV < 50 V are suppressed.
    """
    pairs = [
        ('imon2', 'hv1', prefixed(keyword, 'CurrentVsTime_imon2.png')),
        ('imon3', 'hv2', prefixed(keyword, 'CurrentVsTime_imon3.png')),
    ]
    for icol, hvcol, fname in pairs:
        d = df[df[hvcol] >= 50]

        fig, (ax_i, ax_h) = plt.subplots(2, 1, figsize=(12, 7), sharex=True)

        ax_i.plot(d['datetime'], d[icol] * 1e9,
                  color=COLOR[icol], lw=0.8, label=titles[icol])
        ax_i.set_ylim(CURRENT_YLIM)
        ax_i.set_ylabel('Current (nA)')
        ax_i.set_title(f"{titles[icol]} vs Time  (HV ≥ 50 V)")
        ax_i.legend(loc='upper right')
        ax_i.grid(True, alpha=0.3)

        ax_h.plot(d['datetime'], d[hvcol],
                  color=COLOR[hvcol], lw=0.8, label=titles[hvcol])
        ax_h.set_ylabel('HV (V)')
        ax_h.set_xlabel('Time')
        ax_h.set_title(f"{titles[hvcol]} vs Time")
        ax_h.legend(loc='upper right')
        ax_h.grid(True, alpha=0.3)

        fmt_time_axis(ax_h)
        fig.autofmt_xdate()
        fig.tight_layout()
        save_fig(fig, out_dir, fname)


# ─── Plot 3: Current vs HV ────────────────────────────────────────────────────
def plot_current_vs_hv(df: pd.DataFrame, titles: dict, keyword: str, out_dir: str):
    """
    Scatter plots of current vs HV.  Data within ±1 min of transition periods
    (3 consecutive readings with |ΔHV| > 5 V) are excluded.
    """
    pairs = [('imon2', 'hv1'), ('imon3', 'hv2')]
    fig, axes = plt.subplots(1, 2, figsize=(14, 6))

    for ax, (icol, hvcol) in zip(axes, pairs):
        t_mask = transition_mask(df[hvcol].values, df['datetime'].values)
        hv_mask = (df[hvcol] >= 50).values
        d = df[t_mask & hv_mask]

        ax.scatter(d[hvcol], d[icol] * 1e9,
                   color=COLOR[icol], s=4, alpha=0.4, label=titles[icol])
        ax.set_ylim(CURRENT_YLIM)
        ax.set_xlabel(f"{titles[hvcol]} (V)")
        ax.set_ylabel('Current (nA)')
        ax.set_title(f"{titles[icol]} vs {titles[hvcol]}")
        ax.legend()
        ax.grid(True, alpha=0.3)

    fig.tight_layout()
    save_fig(fig, out_dir, prefixed(keyword, 'CurrentVsHV.png'))


# ─── Plot 4: Current distributions per HV plateau ────────────────────────────
def plot_distributions(df: pd.DataFrame, titles: dict, keyword: str, out_dir: str):
    """
    For each stable HV plateau (±1 min margins removed), save a histogram of
    the corresponding current to a PDF.
    """
    pairs = [
        ('imon2', 'hv1', prefixed(keyword, 'Current_Distribution_uRwell_imon_2.pdf')),
        ('imon3', 'hv2', prefixed(keyword, 'Current_Distribution_uRwell_imon_3.pdf')),
    ]
    for icol, hvcol, pdf_name in pairs:
        hv_arr  = df[hvcol].values
        cur_arr = df[icol].values
        t_arr   = df['datetime'].values
        pdf_path = os.path.join(out_dir, pdf_name)
        os.makedirs(out_dir, exist_ok=True)

        with PdfPages(pdf_path) as pdf:
            n_pages = 0
            for hv_mean, mask in stable_periods(hv_arr, t_arr):
                if hv_mean < 50:
                    continue
                cur = cur_arr[mask] * 1e9    # convert A → nA
                t_start = pd.Timestamp(t_arr[mask][0]).strftime('%Y-%m-%d %H:%M:%S')
                t_end   = pd.Timestamp(t_arr[mask][-1]).strftime('%Y-%m-%d %H:%M:%S')
                fig, ax = plt.subplots(figsize=(8, 5))
                ax.hist(cur, bins=50, color=COLOR[icol], alpha=0.75,
                        edgecolor='black', linewidth=0.4)
                ax.set_xlabel('Current (nA)')
                ax.set_ylabel('Counts')
                ax.set_title(
                    f"{titles[icol]}  —  HV ≈ {hv_mean:.1f} V\n"
                    f"{t_start}  –  {t_end}\n"
                    f"N = {mask.sum()},  "
                    f"mean = {cur.mean():.4f} nA,  "
                    f"σ = {cur.std():.4f} nA"
                )
                ax.grid(True, alpha=0.3)
                pdf.savefig(fig, dpi=150)
                plt.close(fig)
                n_pages += 1

        print(f'  Saved: {pdf_path}  ({n_pages} pages)')


# ─── Plot 5: Interactive time-range plot ──────────────────────────────────────
def interactive_plot(df: pd.DataFrame, titles: dict, keyword: str, out_dir: str):
    """
    Prompt the user to choose variable(s) and a time interval, then save a
    plot.  Repeats until the user types 'q'.
    """
    var_opts = {
        '1': ('imon2', titles['imon2'], COLOR['imon2'], 'Current (nA)', 1e9),
        '2': ('imon3', titles['imon3'], COLOR['imon3'], 'Current (nA)', 1e9),
        '3': ('hv1',  titles['hv1'],  COLOR['hv1'],   'HV (V)',        1.0),
        '4': ('hv2',  titles['hv2'],  COLOR['hv2'],   'HV (V)',        1.0),
    }
    t_min = df['datetime'].min()
    t_max = df['datetime'].max()
    ref_date = str(df['datetime'].dt.date.iloc[0])

    print('\n=== Interactive time-range plot ===')
    print(f'Data span: {t_min}  →  {t_max}')
    print('Available variables:')
    for k, (col, ttl, *_) in var_opts.items():
        print(f'  {k}: {ttl}')

    def parse_time(prompt: str, fallback):
        s = input(prompt).strip()
        if not s:
            return fallback
        try:
            ts = f'{ref_date} {s}' if len(s) <= 8 else s
            return pd.to_datetime(ts)
        except Exception:
            print(f'  Could not parse "{s}", using {fallback}')
            return fallback

    while True:
        choice = input('\nVariables to plot (e.g. "1,2") — or q to quit: ').strip()
        if choice.lower() == 'q':
            break
        sel = [c.strip() for c in choice.split(',') if c.strip() in var_opts]
        if not sel:
            print('  Invalid choice, try again.')
            continue

        t0 = parse_time(f'  Start time [{t_min}]: ', t_min)
        t1 = parse_time(f'  End time   [{t_max}]: ', t_max)

        d = df[(df['datetime'] >= t0) & (df['datetime'] <= t1)]
        if d.empty:
            print('  No data in that range.')
            continue

        fig, ax = plt.subplots(figsize=(12, 5))
        for k in sel:
            col, ttl, clr, unit, scale = var_opts[k]
            ax.plot(d['datetime'], d[col] * scale,
                    color=clr, lw=0.8, label=f'{ttl} ({unit})')
        ax.set_xlabel('Time')
        ax.set_ylabel('Value')
        ax.set_title(
            f'Selected variable(s) vs Time\n'
            f'{t0.strftime("%Y-%m-%d %H:%M:%S")} – {t1.strftime("%Y-%m-%d %H:%M:%S")}'
        )
        ax.legend()
        ax.grid(True, alpha=0.3)
        fmt_time_axis(ax)
        fig.autofmt_xdate()
        fig.tight_layout()

        stamp = datetime.now().strftime('%H%M%S')
        save_fig(fig, out_dir, prefixed(keyword, f'interactive_plot_{stamp}.png'))


# ─── Main ─────────────────────────────────────────────────────────────────────
def main():
    parser = argparse.ArgumentParser(
        description='uRWell HV Scan Plotter',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        'input_file', nargs='?', default='Scan_TopLeftBottomRigh.dat',
        help='Input data file',
    )
    parser.add_argument(
        '--output-dir', '-o', default='Figs',
        help='Directory for output figures',
    )
    parser.add_argument(
        '--no-interactive', action='store_true',
        help='Skip the interactive time-range plot section',
    )
    args = parser.parse_args()

    default_file = args.input_file
    prompted = input(f'Input data file [{default_file}]: ').strip()
    input_file = prompted if prompted else default_file

    print(f'Loading {input_file} …')
    df = load_data(input_file)
    print(f'  {len(df)} rows  |  {df["datetime"].iloc[0]}  →  {df["datetime"].iloc[-1]}')

    keyword = ask_keyword()
    titles  = ask_titles()

    print('\n── [1/4] Current vs Time panels ──')
    plot_time_panels(df, titles, keyword, args.output_dir)

    print('\n── [2/4] Current vs HV ──')
    plot_current_vs_hv(df, titles, keyword, args.output_dir)

    print('\n── [3/4] Current distributions per HV plateau ──')
    plot_distributions(df, titles, keyword, args.output_dir)

    if not args.no_interactive:
        print('\n── [4/4] Interactive time-range plot ──')
        interactive_plot(df, titles, keyword, args.output_dir)

    print('\nAll done.')


if __name__ == '__main__':
    main()
