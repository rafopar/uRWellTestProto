This git repository contains analysis codes/tools for the data collected using the uRwell test prototype.

Check the link https://clasweb.jlab.org/wiki/index.php/Test_Proto_Documents for more details on the uRwell Test Prototype and also 
for instructions on Software installation and running.

# Installation

### Prerequisites
You need to have the LZ4 installed in your environment, and the XYHodoTools package

#### LZ4
You can get from https://lz4.org/

#### XYHodoTools
You can download the XYHodoTools from this repository https://code.jlab.org/hallb/XYHodo

### Installing $\mu$RwellTools 
1) clone the repository
   `git clone git@github.com:rafopar/uRWellTestProto.git`
2) If you are working on JLab ifarms or any other computer where /group/clas12 is mounted, then automatically it will detect LZ4, otherwise you need to point to in the in the **cmake_modules/FindLZ4.cmake** file.
3) go to the **uRWellTestProto** directory and run
	1) `cmake -S . -Bbuild -DCMAKE_INSTALL_PREFIX=/pathe/where/you/want/package/ToBeInstalled/`
	2) `cmake --build build`
	3) `cmake --install build`




# Basic Analysis steps

### Decoding
The 1st step is to decode evio files. One should use the coatjava to do the decoding. For this particular detector in order to speed up the decoding a special branch of coatjave is used (otherwise the decoding is about x10 slower). 

`git clone git@github.com:JeffersonLab/clas12-offline-software.git`
`git checkout iss1008-urWellDecoder`

Note: this branch is in the old repository of coatjava (called clas12-offline-software).

In order to do the decoding you should also use an sqlite file `/group/clas12/users/rafopar/uRWellImportant/clas12.sqlite`
Your environmental variable **CCDB_CONNECTION** should point to `sqlite:////group/clas12/users/rafopar/uRWellImportant/clas12.sqlite`
Note: "///" before /group... is not a mistake.

The Decoding command is:
`/path/to/decoder -i inpFile.evio -o OutputHipoFile.hipo -c 1`

### Skimming
In the decoded file you should expect to see following two banks:
`XYHODO::tdc` and `URWELL::adc`

The `XYHODO::tdc` represents hits from the XYHodoscope that pass the threshold, however
the `URWELL::adc` in the uRwell readout there is no Zero suppression, in other words data from all 1408 channels of the uRwell are readout for every event. This makes the analysis extremely slow. In order to do a meaningful analysis in a reasonable time, we first Skim, and keep only hits that are above some very loose threshold.
In the data stream the APV25 chip provides a waveform of ADCs. Each time sample corresponds to the measured charge in 25 ns time period. Number of time samples is multiple of 3, and can be maximum 15. Currently with cosmic runs, we take 15 time samples of data.
During the skimming process, all pulses that are above the hit threshold are fit with a A Landau(x, MPV, sigma) function, where A is the amplitude of the function, MPV is the Most Probable Value of the Landau function, and the sigma is the width of the Landau function.

The skimming executable is `Skim_PulseFit.exe`.

`Usage: ./Skim_PulseFit.exe <RUN> <File_Number>`
where the <RUN> is the Run number and <File_number> is the index of the file in the given run.
The program will look for an input hipo file  Data/decoded_\<RUN\>_\<File_Number\>.hipo and put the output 
in the **Skims** directory: *Skims/Skim\_PulseFit\_\<RUN>\_<File\_Number>.hipo*

**IMPORTANT** Make sure you have a directory "Skims" where you run the **Skim_PulseFit.exe** executable.

### Format of the Skimmed file
When the fitting of the pulse is done, parameters of the fit function(A, MPV and sigma) are stored.
In addition chi2, ndf and also ADC data from all 15 time samples is stored too. In case one want to Re-Fit pulses. The data is stored in the bank named `uRwell::Pulse`

The picture below shows one particular event from the **uRwell::Pulse** bank.
![An example of the uRwell::Pulse bank ](Doc/uRwell_PulseBank.png)

- sec:  Sector, usually takes values 6 for the uRwell, if 8 then it is a different detector, for example GEM
- layer: takes 2 values: 1 = U layer, 2  V layer
- strip: the strip number (ranges from 1 to 704)
- stripLocal: this also shows the strip number but in the given Readout chip, ranges from 1 to 128
- adc: This is the total ADC of the signal divided by the number of time samples
- adcRel: is the adc divided by the sigma (noise) for the given channel, in other words it shows how many sigma the hit is away from the 0. 
- ts: the time sample of the highest ADC in the pulse waveform. (Ranges from 0 to n_ts - 1)
- slot: the slot (or readout board) number
- ped_rms: the RMS (sigma) of the given strip. This doesn't change from event to event.
- pulse_A0: Amplitude of the function, i.e. the value the Landau function is scaled with.
- pulse_MPV: the MPV of the Landau
- pulse_Sigma: the Sigma of the pulse
- pulse_Chi2: Chi2 of the fit
- pulse_NDF: NDF of the fit
- pulse_ADCN: R ranges from 0 to 15. is the ADC of the pulse in the N-th time sample.


# Automatized running

While it is not advised to run multiple parallel jobs on ifarm, however if you have a computer with multiple cores, the **AnaCodes/RunAnaChain.py** 
python script does the whole chain of analysis Tasks for the given run.

`Usage: python3 RunAnaChain.py <RUN> [Task]`
The **Task** is an optional parameter, which tells there python program from which task to start.
1. Gets the list of all files from the /cache disk
2. with a bunch of 18 parallel jobs runs decoding,
	1. waits if no more than 8 jobs are left, then runs 2nd 18 parallel decodings and so on till all decodings are finished. This step is relatively slow, might take close to 30 min per file.
3. In a similar manner runs the Skimming on all files, with 18 parallel tasks, and jumps to the next 18 runs only when no more than 8 processes are running. This step is also relatively slow, usually takes slightly more than 30 minutes per file.
4. When all Skims are done. The same way it runs the analysis executable **AnaPulseFits.exe**  for all skimmed files, the same way (bunch of 18 parallel processes).  This step is very fast. Processing of a Single file will take under 20 seconds
5. At the end it unifies output root files of the **AnaPulseFits.exe**  command (using hadd), then removes all those root files, and keeps only the unifided one.

Possible values of the **Task** are: 
- "TASK_DECODE" : Starts from the decoding
- "TASK_SkimPulseFit" : Starts from Skim step
- "TASK_AnaPulseFit" : Starts from the Analysis
- "TASK_Hadd" : Runs only the last step: combines all Analysis output root files together then removes input root files.
- If no Task is specified, it will start from the decoding.

# Pedestal Runs
During the analysis/Skiming, we nee to know the noise level for each strip, so that we can make a decision whether the given hit is a signal or just a noise. For this we take so called Pedestal runs, with a Randome trigger, and lower HVs. With those condition we know there is no real signal, and what we measure is just a noise. No much statistics is needed for the pedestal runs. Typically 2000 events are enough, which can be collected under 1 minute.

The pedestal run should be decoded the same way as the production run, however to calculate pedestals following steps should be done:
   1. ./CheckDecoding.exe <RUN> <FILE>
   2.  root -l 'DrawPedestals.cc(<RUN>)'

The DrawPedestals.cc will calculate pedestals and noise for each channel, will produce diagnostic plots in the sub directory **Figs**, and write pedestal and sigmas for in channel in the PedFiles/Peds_<RUN> and PedFiles/GEM_Peds_<RUN>

Later for the given production run (let say 3333), if you want to use pedestals from the given pedestal run e.g. 1234, in the PedFiles directory link Peds_3333 to point to Peds_1234, and GEM_Peds_3333 to point to Peds_1234.


[comment]: <> ( ## Clone the package )

[comment]: <> (Package has a dependency on hipo library, which is a submodule. )
[comment]: <> (Use the command: )
[comment]: <> ( )
[comment]: <> (``` )
[comment]: <> (git clone --recurse-submodules git@github.com:rafopar/uRWellTestProto.git )
[comment]: <> (``` )
[comment]: <> ( )
[comment]: <> (to clone the distribution. )
