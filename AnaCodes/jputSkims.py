#!/usr/bin/env python3
"""
For a given run number, find matching skim files in DIR1, check their sizes,
create a tar.gz archive, and upload it with jput.

Usage:
    python jput_skims.py <RUN>

Example:
    python jput_skims.py 4527
"""

import os
import sys
import glob
import tarfile
import subprocess

# ── Configure paths here ──────────────────────────────────────────────────────
DIR1 = "/volatile/clas12/rafopar/uRwell/Data/BigProto_Skims/"
DIR2 = "/mss/clas12/detectors/uRwell/Skim_PulseFit/EEL_2026/"
# ─────────────────────────────────────────────────────────────────────────────

MIN_SIZE_BYTES = 1 * 1024 * 1024  # 1 MB


def main():
    if len(sys.argv) != 2 or not sys.argv[1].isdigit():
        print(f"Usage: python {sys.argv[0]} <RUN>")
        print("  RUN must be an integer (e.g. 4527)")
        sys.exit(1)

    run = sys.argv[1]

    for d in (DIR1, DIR2):
        if not os.path.isdir(d):
            print(f"Error: '{d}' is not a valid directory.")
            sys.exit(1)

    # Find all matching skim files for this run
    pattern = os.path.join(DIR1, f"Skim_PulseFit_{run}_*.hipo")
    skim_files = sorted(glob.glob(pattern))

    if not skim_files:
        print(f"No files found matching: {pattern}")
        sys.exit(1)

    print(f"Found {len(skim_files)} skim file(s) for run {run}.\n")

    # Check for files under 1 MB — if any exist, print and exit
    too_small = [f for f in skim_files if os.path.getsize(f) < MIN_SIZE_BYTES]
    if too_small:
        print(f"ERROR: {len(too_small)} file(s) are smaller than 1 MB. Aborting.")
        print("Please check these files before proceeding:\n")
        for f in too_small:
            size_kb = os.path.getsize(f) / 1024
            print(f"  {f}  ({size_kb:.1f} KB)")
        sys.exit(1)

    # Create tar.gz archive
    tar_name = f"Skim_PulseFit_Run_{run}.tar.gz"
    tar_path = os.path.join("./", tar_name)
    print(f"Creating archive: {tar_path}")

    try:
        with tarfile.open(tar_path, "w:gz") as tar:
            for f in skim_files:
                tar.add(f, arcname=os.path.basename(f))
                print(f"  Added: {os.path.basename(f)}")
    except Exception as e:
        print(f"ERROR: Failed to create tar.gz archive: {e}")
        sys.exit(1)

    print(f"\nArchive created successfully: {tar_path}")

    # Run jput for the tar.gz file
    cmd = ["jput", "-cache", tar_path, DIR2]
    print(f"\nRunning: {' '.join(cmd)}\n")
    result = subprocess.run(cmd)
    if result.returncode != 0:
        print(f"WARNING: jput exited with code {result.returncode}")


if __name__ == "__main__":
    main()