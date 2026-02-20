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

def Run_AnaPulseFit(file_list):

    proc_Ana = {}

    file_counter = 0
    for curFile in file_list:

        splited_fname = curFile.split(".evio.")
        file_ind = int( splited_fname[1] )

        cmd = "./AnaPulseFits.exe -r %d -f %d " %( run, file_ind )

        proc_Ana[file_ind] = subprocess.Popen([cmd], shell = True)

        file_counter = file_counter + 1

        if file_counter % 18 == 0:

            time.sleep(2)
            stillRunning = True

            while stillRunning:

                nProc = check_NumberofProcesses(proc_Ana)

                if nProc > 8:
                    print("* Still %d ./AnaPulseFits.exe are are running for this batch" %(nProc))
                    print("* Sleeping...")
                    time.sleep(10)
                else:
                    print( "Going to start next batch of Skim" )
                    stillRunning = False


    time.sleep(5)

    stillRunning = True

    while stillRunning:
        nProcAna = check_NumberofProcesses(proc_Ana)

        if( nProcAna >0 ):
            print("* There are still %d Anaclustering processes are running"%(nProcAna))
            time.sleep(2)
        else:
            stillRunning = False


    print("All AnalPulseFits are done")

    return 0

def Run_Hadd(run):

    print(" \n\n\n\n * Adding all root files together")

    cmd_hadd = "hadd -f -j 18 AnaPulseFits_%d.root AnaPulseFits_%d_File_*.root"%( run, run )
    proc_Hadd = subprocess.Popen([cmd_hadd], shell = True)

    WaitWhileRunning(proc_Hadd, "Hadd")

    print("\n\n\n * Cleanning all individual root files")
    cmd_rmRoot = "rm -f AnaPulseFits_%d_File_*.root"%( run )
    proc_rm = subprocess.Popen([cmd_rmRoot], shell = True)

    WaitWhileRunning(proc_rm, "Cleaning root files")


    return 0

TASKS = [
    ("TASK_DECODE", Run_Decoding),
    ("TASK_SkimPulseFit", Run_Skim_PulseFitting),
    ("TASK_AnaPulseFit", Run_AnaPulseFit),
    ("TASK_Hadd", Run_Hadd),
]


if __name__ == "__main__":

    if len(sys.argv) < 2:
        print("Usage: python3 RunAnaChain.py <RUN> [START_TASK]")
        sys.exit(1)

    run = int(sys.argv[1])
    start_task = sys.argv[2] if len(sys.argv) > 2 else "TASK_DECODE"

    print( "The run is %d" %(run))
    os.environ["CCDB_CONNECTION"] = "sqlite:////group/clas12/users/rafopar/uRWellImportant/clas12.sqlite"
    evio_DIR = "/volatile/clas12/rafopar/uRwell/Data/BigProto/"
    DECODER = "/home/rafopar/work/git/clas12-offline-software/coatjava/bin/decoder"

    # Getting list of files. and Sort them by time. Sorting by time is important
    # to avid partially populated batched
    files = sorted(glob.glob("%s/urwell_maroc_00%d.evio*" %(evio_DIR, run)), key=os.path.getmtime)


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
        argument = files # default argument
        if name == "TASK_Hadd":
            argument = run
        func(argument)

    print("All requested tasks finished.")
