#!/usr/bin/env python3
"""
jputSkims_multi.py

For one or more run numbers:
  1. Find matching skim files in DIR1.
  2. Run 'hipo-utils -test' on every file in parallel to detect corruption.
     A file is considered healthy only if the output contains "status check : OK".
  3. If ANY file in a run is corrupt, that run is aborted (no tar, no upload),
     but is recorded in the final summary.
  4. For healthy runs, create one tar.gz archive per run.
     Multiple runs' tar jobs are built in parallel.
  5. Finally, launch jput for every successfully-tarred run in parallel.

Usage:
    python3 jputSkims_multi.py <RUN1> [RUN2 ...]
    python3 jputSkims_multi.py 4527 4528 4529

Tuning:
    --check-workers N   threads for the hipo-utils test step (default: CPU count)
    --tar-workers   N   number of runs to tar concurrently       (default: 18)
    --jput-workers  N   number of jput processes to run at once   (default: 18)
"""

import argparse
import concurrent.futures as cf
import os
import subprocess
import sys
import tarfile
import threading
import time
from glob import glob

# ── Configure paths here ──────────────────────────────────────────────────────
DIR1 = "/volatile/clas12/rafopar/uRwell/Data/BigProto_Skims/"
DIR2 = "/mss/clas12/detectors/uRwell/Skim_PulseFit/EEL_2026/"
# ─────────────────────────────────────────────────────────────────────────────

HIPO_OK_MARKER = "status check : ok"   # compared against output.lower()


# ---------------------------------------------------------------------------
# File integrity check
# ---------------------------------------------------------------------------

def hipo_file_ok(filepath):
    """
    Run 'hipo-utils -test <filepath>'.
    Returns (ok: bool, reason: str).
    The file is considered healthy only if the lowercased output contains
    'status check : ok'.
    """
    try:
        result = subprocess.run(
            ["hipo-utils", "-test", filepath],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            timeout=600,
        )
    except FileNotFoundError:
        return False, "hipo-utils not found in PATH"
    except subprocess.TimeoutExpired:
        return False, "hipo-utils timeout (>10 min)"
    except OSError as exc:
        return False, f"hipo-utils launch failed: {exc}"

    output_lower = result.stdout.lower()

    if HIPO_OK_MARKER in output_lower:
        return True, ""
    # Pull a short snippet of the tail for diagnostic purposes
    tail = " | ".join(
        ln.strip() for ln in result.stdout.strip().splitlines()[-3:]
    )
    return False, f"no 'status check : OK' (tail: {tail or 'empty output'})"


# ---------------------------------------------------------------------------
# Per-run stages
# ---------------------------------------------------------------------------

def collect_files_for_run(run):
    """Return the sorted list of skim files belonging to this run."""
    pattern = os.path.join(DIR1, f"Skim_PulseFit_{run}_*.hipo")
    return sorted(glob(pattern))


def check_run_files(run, files, check_executor, progress):
    """
    Submit hipo-utils -test for every file in this run to the shared
    check_executor. Returns list of (filepath, ok, reason) tuples.
    """
    fut_to_file = {check_executor.submit(hipo_file_ok, f): f for f in files}
    results = []
    for fut in cf.as_completed(fut_to_file):
        f = fut_to_file[fut]
        try:
            ok, reason = fut.result()
        except Exception as e:
            ok, reason = False, f"unexpected error: {e}"
        results.append((f, ok, reason))
        with progress["lock"]:
            progress["done"] += 1
            status = "OK" if ok else "BAD"
            print(f"  [{progress['done']:>{progress['width']}}/{progress['total']}]  "
                  f"Run {run}  {os.path.basename(f):40s}  {status}")
    return results


def create_tar_for_run(run, files):
    """
    Create Skim_PulseFit_Run_<RUN>.tar.gz containing the given files.
    Runs in its own thread — multiple runs can tar concurrently.
    Returns (tar_path, success, error_msg).
    """
    tar_name = f"Skim_PulseFit_Run_{run}.tar.gz"
    tar_path = os.path.join(os.getcwd(), tar_name)

    try:
        with tarfile.open(tar_path, "w:gz") as tar:
            for f in files:
                tar.add(f, arcname=os.path.basename(f))
    except Exception as exc:
        return tar_path, False, str(exc)
    return tar_path, True, ""


def run_jput(tar_path):
    """
    Run 'jput -cache <tar_path> <DIR2>'. Returns (returncode, stdout+stderr).
    """
    try:
        result = subprocess.run(
            ["jput", "-cache", tar_path, DIR2],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )
        return result.returncode, result.stdout
    except FileNotFoundError:
        return -1, "jput not found in PATH"
    except OSError as exc:
        return -1, f"jput launch failed: {exc}"


# ---------------------------------------------------------------------------
# Small utilities
# ---------------------------------------------------------------------------

def fmt_size(n):
    for unit in ("B", "KB", "MB", "GB", "TB"):
        if n < 1024.0:
            return f"{n:.2f} {unit}"
        n /= 1024.0
    return f"{n:.2f} PB"


def print_table(title, headers, rows):
    col_widths = [len(h) for h in headers]
    for row in rows:
        for i, cell in enumerate(row):
            col_widths[i] = max(col_widths[i], len(str(cell)))
    sep = "+" + "+".join("-" * (w + 2) for w in col_widths) + "+"
    def fmt(cells):
        return "|" + "|".join(
            f" {str(c):<{col_widths[i]}} " for i, c in enumerate(cells)
        ) + "|"
    print(title)
    print(sep)
    print(fmt(headers))
    print(sep)
    for row in rows:
        print(fmt(row))
    print(sep)


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="Tar and jput CLAS12 skim files for multiple runs."
    )
    parser.add_argument("runs", nargs="+",
                        help="One or more run numbers (integers).")
    parser.add_argument("--check-workers", type=int, default=os.cpu_count() or 4,
                        help="Threads for hipo-utils -test (default: CPU count).")
    parser.add_argument("--tar-workers", type=int, default=18,
                        help="Number of runs tarred in parallel (default: 18).")
    parser.add_argument("--jput-workers", type=int, default=18,
                        help="Number of concurrent jput processes (default: 18).")
    args = parser.parse_args()

    # Validate run numbers
    try:
        runs = [int(r) for r in args.runs]
        if any(r < 0 for r in runs):
            raise ValueError
    except ValueError:
        print("Error: all RUN arguments must be non-negative integers.")
        sys.exit(1)
    runs = sorted(set(runs))   # dedupe & sort

    # Validate paths
    for d in (DIR1, DIR2):
        if not os.path.isdir(d):
            print(f"Error: '{d}' is not a valid directory.")
            sys.exit(1)

    print("=" * 70)
    print(f"Processing {len(runs)} run(s): {runs}")
    print(f"  check-workers = {args.check_workers}")
    print(f"  tar-workers   = {args.tar_workers}")
    print(f"  jput-workers  = {args.jput_workers}")
    print("=" * 70)

    # ------------------------------------------------------------------ #
    # 1. Collect files per run                                             #
    # ------------------------------------------------------------------ #
    run_files = {}          # {run: [filepaths]}
    runs_no_files = []      # runs with zero matching files

    for run in runs:
        files = collect_files_for_run(run)
        if files:
            run_files[run] = files
            print(f"Run {run:>6}: {len(files)} file(s)")
        else:
            runs_no_files.append(run)
            print(f"Run {run:>6}: NO files found")

    if not run_files:
        print("\nNothing to do — no runs have matching files.")
        sys.exit(1)

    # ------------------------------------------------------------------ #
    # 2. Parallel hipo-utils -test over ALL files from ALL runs            #
    # ------------------------------------------------------------------ #
    total_files = sum(len(v) for v in run_files.values())
    print()
    print("-" * 70)
    print(f"Stage 1: Running hipo-utils -test on {total_files} file(s)")
    print("-" * 70)

    progress = {
        "done":  0,
        "total": total_files,
        "width": len(str(total_files)),
        "lock":  threading.Lock(),
    }

    run_corrupt = {}   # {run: [(filepath, reason), ...]}

    t0 = time.time()
    with cf.ThreadPoolExecutor(max_workers=args.check_workers) as check_ex:
        # Kick off checks for every run; accumulate per-run results
        for run, files in run_files.items():
            run_corrupt[run] = []
            results = check_run_files(run, files, check_ex, progress)
            for filepath, ok, reason in results:
                if not ok:
                    run_corrupt[run].append((filepath, reason))

    t_check = time.time() - t0
    print(f"\nhipo-utils checks finished in {t_check:.1f} s")

    # Partition runs into healthy vs aborted
    healthy_runs = [r for r in run_files if not run_corrupt[r]]
    aborted_runs = [r for r in run_files if     run_corrupt[r]]

    # ------------------------------------------------------------------ #
    # 3. Parallel tar.gz creation for healthy runs                         #
    # ------------------------------------------------------------------ #
    tar_results = {}   # {run: (tar_path, success, err_msg)}

    if healthy_runs:
        print()
        print("-" * 70)
        print(f"Stage 2: Creating tar.gz for {len(healthy_runs)} healthy run(s)")
        print("-" * 70)

        t0 = time.time()
        with cf.ThreadPoolExecutor(max_workers=args.tar_workers) as tar_ex:
            fut_to_run = {
                tar_ex.submit(create_tar_for_run, run, run_files[run]): run
                for run in healthy_runs
            }
            for fut in cf.as_completed(fut_to_run):
                run = fut_to_run[fut]
                tar_path, ok, err = fut.result()
                tar_results[run] = (tar_path, ok, err)
                if ok:
                    size = os.path.getsize(tar_path)
                    print(f"  Run {run:>6}: tarred {os.path.basename(tar_path)}  "
                          f"({fmt_size(size)})")
                else:
                    print(f"  Run {run:>6}: TAR FAILED — {err}")
        t_tar = time.time() - t0
        print(f"\nTar stage finished in {t_tar:.1f} s")
    else:
        print("\nStage 2 skipped — no healthy runs to tar.")

    # ------------------------------------------------------------------ #
    # 4. Parallel jput for successfully tarred runs                        #
    # ------------------------------------------------------------------ #
    tarred_runs = [r for r, (_, ok, _) in tar_results.items() if ok]
    jput_results = {}  # {run: (returncode, output_tail)}

    if tarred_runs:
        print()
        print("-" * 70)
        print(f"Stage 3: Uploading {len(tarred_runs)} tar file(s) via jput")
        print("-" * 70)

        t0 = time.time()
        with cf.ThreadPoolExecutor(max_workers=args.jput_workers) as jput_ex:
            fut_to_run = {
                jput_ex.submit(run_jput, tar_results[run][0]): run
                for run in tarred_runs
            }
            print(f"  (Running up to {args.jput_workers} jput process(es) "
                  f"concurrently)\n")
            for fut in cf.as_completed(fut_to_run):
                run = fut_to_run[fut]
                rc, out = fut.result()
                jput_results[run] = (rc, out)
                tag = "OK" if rc == 0 else f"FAIL(rc={rc})"
                print(f"  Run {run:>6}: jput {tag}")
        t_jput = time.time() - t0
        print(f"\njput stage finished in {t_jput:.1f} s")
    else:
        print("\nStage 3 skipped — no tar files to upload.")

    # ------------------------------------------------------------------ #
    # 5. Final summary                                                     #
    # ------------------------------------------------------------------ #
    print()
    print("=" * 70)
    print("SUMMARY")
    print("=" * 70)

    n_uploaded    = sum(1 for r in jput_results if jput_results[r][0] == 0)
    n_jput_fail   = sum(1 for r in jput_results if jput_results[r][0] != 0)
    n_tar_fail    = sum(1 for r in tar_results if not tar_results[r][1])
    n_aborted     = len(aborted_runs)
    n_no_files    = len(runs_no_files)

    print(f"  Runs requested              : {len(runs)}")
    print(f"  Runs with no matching files : {n_no_files}")
    print(f"  Runs aborted (corrupt files): {n_aborted}")
    print(f"  Runs failed to tar          : {n_tar_fail}")
    print(f"  Runs successfully uploaded  : {n_uploaded}")
    print(f"  Runs with jput failure      : {n_jput_fail}")

    # Build an attention table
    rows = []
    for run in runs_no_files:
        rows.append((run, "no matching files in DIR1"))
    for run in aborted_runs:
        n_bad = len(run_corrupt[run])
        rows.append((run, f"ABORTED — {n_bad} corrupt file(s)"))
    for run, (_, ok, err) in tar_results.items():
        if not ok:
            rows.append((run, f"tar failed: {err}"))
    for run, (rc, _) in jput_results.items():
        if rc != 0:
            rows.append((run, f"jput exit code {rc}"))

    if rows:
        rows.sort(key=lambda x: x[0])
        print()
        print_table("Runs requiring attention:", ["Run", "Status"], rows)

    # Detail listing of the corrupt files per aborted run
    if aborted_runs:
        print()
        print("Corrupt file details (per aborted run):")
        for run in aborted_runs:
            print(f"  Run {run}:")
            for filepath, reason in run_corrupt[run]:
                print(f"    {os.path.basename(filepath)}  ->  {reason}")

    if not rows:
        print("\nAll requested runs completed successfully.")


if __name__ == "__main__":
    main()
