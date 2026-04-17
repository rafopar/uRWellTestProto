#!/usr/bin/env python3
"""
Find decoded files present in DIR1 but not in DIR2, and run a single jput
command with all qualifying files. Files smaller than 1 MB abort the transfer.

Usage:
    python jput_decodes.py
"""

import os
import sys
import subprocess

# ── Configure paths here ──────────────────────────────────────────────────────
DIR1 = "/volatile/clas12/rafopar/uRwell/Data/BigProto_Decoded/"
DIR2 = "/mss/clas12/detectors/uRwell/decoded/2026_EEL/"
# ─────────────────────────────────────────────────────────────────────────────

MIN_SIZE_BYTES = 1 * 1024 * 1024  # 1 MB


def get_filenames(directory):
    files = {}
    for entry in os.scandir(directory):
        if entry.is_file():
            files[entry.name] = entry.path
    return files


def main():
    for d in (DIR1, DIR2):
        if not os.path.isdir(d):
            print(f"Error: '{d}' is not a valid directory.")
            sys.exit(1)

    files_dir1 = get_filenames(DIR1)
    files_dir2 = get_filenames(DIR2)

    missing = {name: path for name, path in files_dir1.items() if name not in files_dir2}

    if not missing:
        print("No missing files found. DIR2 already contains all files from DIR1.")
        return

    # Check for files under 1 MB — if any exist, print and exit
    too_small = {name: path for name, path in missing.items() if os.path.getsize(path) < MIN_SIZE_BYTES}
    if too_small:
        print(f"ERROR: {len(too_small)} file(s) are smaller than 1 MB. Aborting transfer.")
        print("Please check these files before proceeding:\n")
        for name in sorted(too_small):
            size_kb = os.path.getsize(too_small[name]) / 1024
            print(f"  {too_small[name]}  ({size_kb:.1f} KB)")
        sys.exit(1)

    print(f"Found {len(missing)} file(s) in DIR1 not present in DIR2.\n")

    missing_paths = [path for _, path in sorted(missing.items())]
    cmd = ["jput", "-cache"] + missing_paths + [DIR2]

    print(f"Running: {' '.join(cmd)}\n")
    result = subprocess.run(cmd)
    if result.returncode != 0:
        print(f"WARNING: jput exited with code {result.returncode}")


if __name__ == "__main__":
    main()