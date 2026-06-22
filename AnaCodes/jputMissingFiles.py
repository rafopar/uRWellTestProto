#!/usr/bin/env python3
"""
Find files present in DIR1 but not in DIR2, and run a single jput command
with all missing files listed together. Files smaller than 1 MB are skipped.

Usage:
    python jput_missing.py <mode>

Arguments:
    mode    Either "Decodes" or "Skims"

Examples:
    python jput_missing.py Decodes
    python jput_missing.py Skims
"""

import os
import sys
import subprocess

# ── Configure paths here ──────────────────────────────────────────────────────
PATHS = {
    "Decodes": {
        "DIR1": "/volatile/clas12/rafopar/uRwell/Data/BigProto_Decoded/",
        "DIR2": "/mss/clas12/detectors/uRwell/decoded/2026_EEL/",
    },
    "Skims": {
        "DIR1": "/volatile/clas12/rafopar/uRwell/Data/BigProto_Skims/",
        "DIR2": "/mss/clas12/detectors/uRwell/Skim_PulseFit/EEL_2026/",
    },
}
# ─────────────────────────────────────────────────────────────────────────────

MIN_SIZE_BYTES = 1 * 1024 * 1024  # 1 MB


def get_filenames(directory):
    """Return a dict of {filename: full_path} for all files in directory (non-recursive)."""
    files = {}
    for entry in os.scandir(directory):
        if entry.is_file():
            files[entry.name] = entry.path
    return files


def main():
    if len(sys.argv) != 2 or sys.argv[1] not in PATHS:
        print(f"Usage: python {sys.argv[0]} <mode>")
        print(f"  mode must be one of: {', '.join(PATHS.keys())}")
        sys.exit(1)

    mode = sys.argv[1]
    DIR1 = PATHS[mode]["DIR1"]
    DIR2 = PATHS[mode]["DIR2"]

    print(f"Mode    : {mode}")
    print(f"DIR1    : {DIR1}")
    print(f"DIR2    : {DIR2}\n")

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

    # Filter out files smaller than 1 MB
    too_small = {name: path for name, path in missing.items() if os.path.getsize(path) < MIN_SIZE_BYTES}
    missing   = {name: path for name, path in missing.items() if name not in too_small}

    if too_small:
        print(f"Skipped {len(too_small)} file(s) smaller than 1 MB:")
        for name in sorted(too_small):
            size_kb = os.path.getsize(too_small[name]) / 1024
            print(f"  {too_small[name]}  ({size_kb:.1f} KB)")
        print()

    if not missing:
        print("No qualifying files to transfer (all missing files were under 1 MB).")
        return

    print(f"Found {len(missing)} file(s) in DIR1 not present in DIR2 and >= 1 MB.\n")

    missing_paths = [path for _, path in sorted(missing.items())]
    cmd = ["jput", "-cache"] + missing_paths + [DIR2]

    print(f"Running: {' '.join(cmd)}\n")
    result = subprocess.run(cmd)
    if result.returncode != 0:
        print(f"WARNING: jput exited with code {result.returncode}")


if __name__ == "__main__":
    main()