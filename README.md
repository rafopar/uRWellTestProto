This git repository contains analysis codes/tools for the data collected using the uRwell test prototype.

Check the link https://clasweb.jlab.org/wiki/index.php/Test_Proto_Documents for more details on the uRwell Test Prototype and also 
for instructions on Software installation and running.

# Installation

### Prerequisites
You need to have the LZ4 installed in your environment 

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

### Format of the Skimmed file
When the fitting of the pulse is done, parameters of the fit function(A, MPV and sigma) are stored.
In addition chi2, ndf and also ADC data from all 15 time samples is stored too. In case one want to Re-Fit pulses. The data is stored in the bank named `uRwell::Pulse`

The picture below shows one particular event from the **uRwell::Pulse** bank.
![[Doc/uRwell_PulseBank.png]]

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

[comment]: <> ( ## Clone the package )

[comment]: <> (Package has a dependency on hipo library, which is a submodule. )
[comment]: <> (Use the command: )
[comment]: <> ( )
[comment]: <> (``` )
[comment]: <> (git clone --recurse-submodules git@github.com:rafopar/uRWellTestProto.git )
[comment]: <> (``` )
[comment]: <> ( )
[comment]: <> (to clone the distribution. )
