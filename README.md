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
	1) cmake -S . -Bbuild -DCMAKE_INSTALL_PREFIX=/pathe/where/you/want/package/ToBeInstalled/
	2) cmake --build build
	3) cmake --install build


[comment]: <> ( ## Clone the package )

[comment]: <> (Package has a dependency on hipo library, which is a submodule. )
[comment]: <> (Use the command: )
[comment]: <> ( )
[comment]: <> (``` )
[comment]: <> (git clone --recurse-submodules git@github.com:rafopar/uRWellTestProto.git )
[comment]: <> (``` )
[comment]: <> ( )
[comment]: <> (to clone the distribution. )
