import argparse
import subprocess
import sys

# This program calculates pedestals for a given run number.
#
# It runs three steps in sequence:
#   1) Decode the raw EVIO file with the uRwell decoder.
#   2) Run CheckDecoding on the decoded HIPO file.
#   3) Run DrawPedestals to produce the pedestal plots/tables.
#
# By default only 2000 events are decoded, which is enough for a pedestal
# analysis, but the number of events is an optional argument.
#
# Example:
#   python3 CalcPedestals.py 3261
#   python3 CalcPedestals.py 3261 -i /path/to/evio/dir -n 5000

# Default directory where the raw EVIO files are searched for.
DEFAULT_EVIODIR = "/volatile/clas12/rafopar/uRwell/Data/BigProto/"

# The EVIO file index used for the pedestal decoding (matches Data/decoded_<RUN>_<IDX>.hipo).lt 
FILE_INDEX = 0

# Path to the decoder executable.
DECODER = "../decoder/bin/uRwellDecoder.exe"


def run_step(description, cmd):
    """Run one shell command, streaming its output. Abort the chain on failure."""
    print("\n" + "=" * 70)
    print("* %s" % description)
    print("  command: %s" % cmd)
    print("=" * 70)

    proc = subprocess.Popen([cmd], shell=True)
    proc.wait()

    if proc.returncode != 0:
        print("\n*** ERROR: '%s' failed with exit code %d. Aborting." %
              (description, proc.returncode))
        sys.exit(proc.returncode)


if __name__ == "__main__":

    parser = argparse.ArgumentParser(
        description="Calculate pedestals for a given run: decode -> CheckDecoding -> DrawPedestals.")
    parser.add_argument("run", type=int, help="Run number, e.g. 3261")
    parser.add_argument("-i", "--indir", default=DEFAULT_EVIODIR,
                        help="Directory containing the raw EVIO file "
                             "(default: %s)" % DEFAULT_EVIODIR)
    parser.add_argument("-n", "--nevents", type=int, default=2000,
                        help="Number of events to decode (default: 2000)")

    args = parser.parse_args()

    run = args.run
    nevents = args.nevents
    evio_dir = args.indir.rstrip("/")

    evio_file = "%s/urwell_maroc_00%d.evio.00000" % (evio_dir, run)
    hipo_file = "Data/decoded_%d_%d.hipo" % (run, FILE_INDEX)

    print("Calculating pedestals for run %d" % run)
    print("  EVIO input : %s" % evio_file)
    print("  # events   : %d" % nevents)

    # -- Step 1: decode the raw EVIO file --
    cmd_decode = "%s -i %s -o %s -r %d -n %d" % (
        DECODER, evio_file, hipo_file, run, nevents)
    run_step("Step 1/3: Decoding", cmd_decode)

    # -- Step 2: CheckDecoding --
    cmd_check = "./CheckDecoding.exe %d %d" % (run, FILE_INDEX)
    run_step("Step 2/3: CheckDecoding", cmd_check)

    # -- Step 3: DrawPedestals --
    cmd_draw = "root -l -b -q 'DrawPedestals.cc(%d)'" % run
    run_step("Step 3/3: DrawPedestals", cmd_draw)

    print("\n* All done. Pedestals for run %d have been calculated." % run)
