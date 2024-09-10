import subprocess
import shlex
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
    
        

if __name__ == "__main__":
    
    if len(sys.argv) != 2 :
        print( "Wrong syntax, exiting" )
        print( "The command should look like Decode_Run.py $Run" )
        exit(1)

    processes = set()
    run = int(sys.argv[1])

    print( "The run is %d" %(run))
    
    #os.environ["CCDB_CONNECTION"] = "sqlite:////work/clas12/rafopar/uRWELL/Readout/TT_Tests/clas12.sqlite"
    os.environ["CCDB_CONNECTION"] = "sqlite:////group/clas12/users/rafopar/uRWellImportant/clas12.sqlite"
    
    #evio_DIR = "/cache/clas12/detectors/uRwell/2022_2023_EEL_Data/"
    #evio_DIR = "/cache/clas12/detectors/uRwell/2024_EEL_Data/"
    evio_DIR = "/cache/clas12/detectors/uRwell/2024_EEL_Hodo_And_uRwell/"
    
    DECODER = "/home/rafopar/work/git/clas12-offline-software/coatjava/bin/decoder"
    
    files = glob.glob("%s/urwell_maroc_00%d.evio*" %(evio_DIR, run));
    #files = glob.glob("%s/urwell_00%d.evio*" %(evio_DIR, run));
    
    # will keep track of processes for each file, and rnu next step when then current step is finished
    proc_decode = {} 
    
    file_counter = 0
    
    for curFile in files:
        
        #split the file name by ".evio." separator to get the file index
        splited_fname = curFile.split(".evio.")    
        file_ind = int( splited_fname[1] )
        
        cmd = "%s -i %s -o Data/decoded_%d_%d.hipo -c 1" %(DECODER, curFile, run, file_ind )
        
        ModifiedRun = "%d%d"%(run, file_ind)
        
        #cmd = "ln -s Skim_ZeroSuppr_%d_%d.hipo Skim_ZeroSuppr_%s_All.hipo" %(run, file_ind, ModifiedRun)
        #cmd = "./AnaClustering.exe -r %s -t 4 -m 1" %( ModifiedRun)
        
        print( "The command is %s"%(cmd) )
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
    print("*     Starting the Skim_ZeroSuppression")

    proc_Skim = {}
    file_counter = 0
    for curFile in files:
        
        #split the file name by ".evio." separator to get the file index
        splited_fname = curFile.split(".evio.")    
        file_ind = int( splited_fname[1] )
        
        cmd = "./SkimZeroSuppression.exe %d %d" %( run, file_ind )
        
        proc_Skim[file_ind] = subprocess.Popen([cmd], shell = True)
        
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

        print("* Checking if SkimZeroSuppression of all files is finished...")
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
            
        

    threshold = 4.
    clSize = 1

    proc_Ana = {}
    
    file_counter = 0
    for curFile in files:

        #split the file name by ".evio." separator to get the file index
        splited_fname = curFile.split(".evio.")    
        file_ind = int( splited_fname[1] )
        
        cmd = "./AnaClustering.exe -r %d -f %d -t %1.1f -m %d" %( run, file_ind, threshold, clSize )
        
        proc_Ana[file_ind] = subprocess.Popen([cmd], shell = True)
        
        file_counter = file_counter + 1

        
        
    time.sleep(5)
    
    stillRunning = True
    
    while stillRunning:
        nProcAna = check_NumberofProcesses(proc_Ana)

        if( nProcAna >0 ):
            print("* There are still %d Anaclustering processes are running"%(nProcAna))
            time.sleep(2)
        else:
           stillRunning = False 
    

    print(" \n\n\n\n * Adding all root files together")

    cmd_hadd = "hadd -f AnaClustering_%d_Thr_%1.1f_MinHits_%d.root AnaClustering_%d_Thr_%1.1f_MinHits_%d_File_*.root"%( run, threshold, clSize, run, threshold, clSize )
    proc_Hadd = subprocess.Popen([cmd_hadd], shell = True)

    WaitWhileRunning(proc_Hadd, "Hadd")
    
#    stillRunning = True
#    while stillRunning:

#        print(proc_Hadd.poll())

#        if proc_Hadd.poll() is None: 
#            print("* Hadd is still running")
#            time.sleep(2)
#        else:
#           stillRunning = False 

    print("\n\n\n * Cleanning all individual root files")
    cmd_rmRoot = "rm -f AnaClustering_%d_Thr_%1.1f_MinHits_%d_File_*.root"%( run, threshold, clSize )
    proc_rm = subprocess.Popen([cmd_rmRoot], shell = True)

    WaitWhileRunning(proc_rm, "Cleaning root files")

    print("\n\n\n * Starting Drawing histograms")

    cmd_Draw = "./DrawPlotsWithClustering.exe -r %d -t %1.1f -m %d"%(run, threshold, clSize)
    proc_Draw = subprocess.Popen([cmd_Draw], shell = True)

    WaitWhileRunning(proc_rm, "Drawing plots")
    print("\n\n All done")
