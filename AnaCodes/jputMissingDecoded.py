#!/usr/bin/env python3
"""
Find files present in DIR1 but not in DIR2, and run jput for each missing file.

Usage:
    python jput_missing.py
"""

import os
import subprocess

# ── /volatile and /mss paths ──────────────────────────────────────────────────────
DIR1 = "/volatile/clas12/rafopar/uRwell/Data/BigProto_Decoded/"
DIR2 = "/mss/clas12/detectors/uRwell/decoded/2026_EEL/"
# ─────────────────────────────────────────────────────────────────────────────


def get_filenames(directory):
    """Return a dict of {filename: full_path} for all files in directory (non-recursive)."""
    files = {}
    for entry in os.scandir(directory):
        if entry.is_file():
            files[entry.name] = entry.path
    return files


def main():
    for d in (DIR1, DIR2):
        if not os.path.isdir(d):
            print(f"Error: '{d}' is not a valid directory.")
            return

    files_dir1 = get_filenames(DIR1)
    files_dir2 = get_filenames(DIR2)

    missing = {name: path for name, path in files_dir1.items() if name not in files_dir2}

    if not missing:
        print("No missing files found. DIR2 already contains all files from DIR1.")
        return

    print(f"Found {len(missing)} file(s) in DIR1 not present in DIR2:\n")

    missing_paths = [path for _, path in sorted(missing.items())]
    cmd = ["jput", "-cache"] + missing_paths + [DIR2]

    print(f"Running: {' '.join(cmd)}\n")
    result = subprocess.run(cmd)
    if result.returncode != 0:
        print(f"WARNING: jput exited with code {result.returncode}")


if __name__ == "__main__":
    main()
