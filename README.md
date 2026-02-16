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

The 1st step is to decode evio files. One should use the coatjava to do the decoding. For this particular detector in order to speed up the decoding a special branch of coatjave is used (otherwise the decoding is about x10 slower). 

`git clone git@github.com:JeffersonLab/clas12-offline-software.git`
`git checkout iss1008-urWellDecoder`

Note: this branch is in the old repository of coatjava (called clas12-offline-software).

In order to do the decoding you should also use an sqlite file `/group/clas12/users/rafopar/uRWellImportant/clas12.sqlite`
Your environmental variable **CCDB_CONNECTION** should point to `sqlite:////group/clas12/users/rafopar/uRWellImportant/clas12.sqlite`
Note: "///" before /group... is not a mistake.

[comment]: <> ( ## Clone the package )

[comment]: <> (Package has a dependency on hipo library, which is a submodule. )
[comment]: <> (Use the command: )
[comment]: <> ( )
[comment]: <> (``` )
[comment]: <> (git clone --recurse-submodules git@github.com:rafopar/uRWellTestProto.git )
[comment]: <> (``` )
[comment]: <> ( )
[comment]: <> (to clone the distribution. )
