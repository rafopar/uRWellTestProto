import subprocess
import os
import time
import glob
import sys

def check_NumberofProcesses(proc_dict):
    nRunningProcess = 0
    for file_ind in proc_dict:
        if proc_dict[file_ind].poll() is None:
            nRunningProcess = nRunningProcess + 1
    return nRunningProcess;


def WaitWhileRunning(proc, procDescription):
    StillRunning = True
    while StillRunning:
        if proc.poll() is None:
            print("* Waiting 5 seconds, while %s is running" % (procDescription))
            time.sleep(5);
        else:
            return 1

def Run_Decoding(files):

    # will keep track of processes for each file, and rnu next step when then current step is finished
    proc_decode = {}

    file_counter = 0

    for curFile in files:

        splited_fname = curFile.split(".evio.")
        file_ind = int( splited_fname[1] )
        cmd = "%s -i %s -o Data/decoded_%d_%d.hipo -c 1" %(DECODER, curFile, run, file_ind )
        print("Decode command is %s"%(cmd))
        proc_decode[file_ind] = subprocess.Popen([cmd], shell = True)

        file_counter = file_counter + 1

        if file_counter % 18 == 0:

            time.sleep(2)

            stillRunning = True

            while stillRunning:

                nProc = check_NumberofProcesses(proc_decode)

                if nProc > 8:
                    print("* Still %d decodings are are running for this batch" %(nProc))
                    print("* Sleeping...")
                    time.sleep(10)
                else:
                    print( "Going to start next batch of decoding" )
                    stillRunning = False


    time.sleep(5)

    stillRunning = True

    while stillRunning:
        nRunningProcess = 0

        print("* Checking if decoding of all files is finished...")
        for file_ind in proc_decode:

            print( "Process Status for %d process is %s" %( proc_decode[file_ind].pid, proc_decode[file_ind].poll() ) )

            if proc_decode[file_ind].poll() is None:
                nRunningProcess = nRunningProcess + 1
                print("The decoding for the file %d is still running" %(file_ind))

        if nRunningProcess == 0:
            stillRunning = False;
            break
        else:
            print("There are still %d decodings running." %(nRunningProcess) )
            wait_sec = 30;
            print("Waiting for %d seconds" %(wait_sec))
            time.sleep(wait_sec)


    print("*     Decoding of all files is finished")
    print("\n\n\n")

    return 0
def Run_Skim_PulseFitting(files):

    proc_Skim = {}

    file_counter = 0

    for curFile in files:

        splited_fname = curFile.split(".evio.")
        file_ind = int( splited_fname[1] )

        print("file ind  = %d" %(file_ind))

        cmd_Skim_PulseFit = "./Skim_PulseFit.exe %d %d" %(run, file_ind)

        proc_Skim[file_ind] = subprocess.Popen([cmd_Skim_PulseFit], shell = True)

        file_counter = file_counter + 1

        if file_counter % 18 == 0:

            time.sleep(2)
            stillRunning = True

            while stillRunning:

                nProc = check_NumberofProcesses(proc_Skim)

                if nProc > 8:
                    print("* Still %d Skim are are running for this batch" %(nProc))
                    print("* Sleeping...")
                    time.sleep(10)
                else:
                    print( "Going to start next batch of Skim" )
                    stillRunning = False

    time.sleep(5)

    stillRunning = True

    while stillRunning:
        nRunningProcess = 0

        print("* Checking if Skim_PulseFit of all files is finished...")
        for file_ind in proc_Skim:

            print( "Process Status for %d process is %s" %( proc_Skim[file_ind].pid, proc_Skim[file_ind].poll() ) )

            if proc_Skim[file_ind].poll() is None:
                nRunningProcess = nRunningProcess + 1
                print("The Skimming for the file %d is still running" %(file_ind))

        if nRunningProcess == 0:
            stillRunning = False;
            break
        else:
            print("There are still %d Skims running." %(nRunningProcess) )
            wait_sec = 30;
            print("Waiting for %d seconds" %(wait_sec))
            time.sleep(wait_sec)

    print("Skimming is done")


    return 0

def Run_Ana(file_list, exe, descr):
    # Generic per-file analysis runner. The only things that change between the
    # different analyses are the executable (exe) and the printed description
    # (descr). The output root file names are decided inside the C++ program,
    # so they only matter at the Hadd step (see Run_Hadd).

    proc_Ana = {}

    file_counter = 0
    for curFile in file_list:

        splited_fname = curFile.split(".evio.")
        file_ind = int( splited_fname[1] )

        cmd = "%s -r %d -f %d " %( exe, run, file_ind )

        proc_Ana[file_ind] = subprocess.Popen([cmd], shell = True)

        file_counter = file_counter + 1

        if file_counter % 18 == 0:

            time.sleep(2)
            stillRunning = True

            while stillRunning:

                nProc = check_NumberofProcesses(proc_Ana)

                if nProc > 8:
                    print("* Still %d %s are are running for this batch" %(nProc, exe))
                    print("* Sleeping...")
                    time.sleep(10)
                else:
                    print( "Going to start next batch of %s" %(descr) )
                    stillRunning = False


    time.sleep(5)

    stillRunning = True

    while stillRunning:
        nProcAna = check_NumberofProcesses(proc_Ana)

        if( nProcAna >0 ):
            print("* There are still %d %s processes are running"%(nProcAna, descr))
            time.sleep(2)
        else:
            stillRunning = False


    print("All %s are done" %(descr))

    return 0

def Run_AnaPulseFit(file_list):
    return Run_Ana(file_list, "./AnaPulseFits.exe", "AnaPulseFits")

def Run_AnaDoubleHodo(file_list):
    return Run_Ana(file_list, "./AnaSecondProtoDoubleHodo.exe", "AnaSecondProtoDoubleHodo")

def Run_Hadd(run, prefix="AnaPulseFits"):

    print(" \n\n\n\n * Adding all %s root files together" %(prefix))

    cmd_hadd = "hadd -f402 -j 18 %s_%d.root %s_%d_File_*.root"%( prefix, run, prefix, run )
    proc_Hadd = subprocess.Popen([cmd_hadd], shell = True)

    WaitWhileRunning(proc_Hadd, "Hadd")

    print("\n\n\n * Cleanning all individual root files")
    cmd_rmRoot = "rm -f %s_%d_File_*.root"%( prefix, run )
    proc_rm = subprocess.Popen([cmd_rmRoot], shell = True)

    WaitWhileRunning(proc_rm, "Cleaning root files")


    return 0

# Available analyses that can be run after TASK_SkimPulseFit. Each entry maps a
# short selector (used on the command line via --ana=...) to:
#   (task name, analysis function, output root-file prefix used by Run_Hadd)
ANA_OPTIONS = {
    "pulsefit":   ("TASK_AnaPulseFit",  Run_AnaPulseFit,  "AnaPulseFits"),
    "doublehodo": ("TASK_AnaDoubleHodo", Run_AnaDoubleHodo, "AnaSecondHodoDoubleHodo"),
}


def build_tasks(selected_anas):
    # Decoding and skimming are always done first. For every selected analysis
    # we insert its Ana task followed by the matching Hadd task (each Hadd
    # closes over its own output prefix so it adds the correct root files).
    tasks = [
        ("TASK_DECODE", Run_Decoding),
        ("TASK_SkimPulseFit", Run_Skim_PulseFitting),
    ]

    for key in selected_anas:
        task_name, ana_func, prefix = ANA_OPTIONS[key]
        tasks.append((task_name, ana_func))
        tasks.append(("TASK_Hadd_%s" % (key), lambda r, p=prefix: Run_Hadd(r, p)))

    return tasks


if __name__ == "__main__":

    if len(sys.argv) < 2:
        print("Usage: python3 RunSecondProtoAnaChain.py <RUN> [START_TASK] [--ana=pulsefit|doublehodo|both]")
        print("  --ana selects which analysis to run after TASK_SkimPulseFit (default: both)")
        sys.exit(1)

    run = int(sys.argv[1])

    # Defaults
    start_task = "TASK_DECODE"
    ana_selection = "both"

    # Remaining args: an optional START_TASK (positional) and/or an --ana=... flag
    for arg in sys.argv[2:]:
        if arg.startswith("--ana="):
            ana_selection = arg.split("=", 1)[1]
        else:
            start_task = arg

    if ana_selection == "both":
        selected_anas = list(ANA_OPTIONS.keys())
    elif ana_selection in ANA_OPTIONS:
        selected_anas = [ana_selection]
    else:
        print("Unknown --ana value: %s" % (ana_selection))
        print("Available: pulsefit, doublehodo, both")
        sys.exit(1)

    print( "The run is %d" %(run))
    print( "Selected analyses: %s" %(", ".join(selected_anas)))
    os.environ["CCDB_CONNECTION"] = "sqlite:////group/clas12/users/rafopar/uRWellImportant/clas12SecondProto.sqlite"
    evio_DIR = "/volatile/clas12/rafopar/uRwell/Data/BigProto/"
    DECODER = "/home/rafopar/work/git/clas12-offline-software/coatjava/bin/decoder"

    # Getting list of files. and Sort them by time. Sorting by time is important
    # to avid partially populated batched
    files = sorted(glob.glob("%s/urwell_maroc_00%d.evio*" %(evio_DIR, run)), key=os.path.getmtime)

    # Build the task chain based on the requested analyses
    TASKS = build_tasks(selected_anas)

    # Find where to start
    task_names = [name for name, _ in TASKS]

    if start_task not in task_names:
        print(f"Unknown task: {start_task}")
        print("Available tasks:", ", ".join(task_names))
        sys.exit(1)

    start_index = task_names.index(start_task)

    # Execute from chosen task onward
    for name, func in TASKS[start_index:]:
        print(f"Starting {name}")
        if name.startswith("TASK_Hadd"):
            func(run)
        else:
            func(files)

    print("All requested tasks finished.")
