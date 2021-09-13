# FMU Debugging with Qt Creator on Linux

This is a very short summary of the process to debug _MasterSim_ alone
or _MasterSim_ together with slaves.

## Common setup

- generate FMUs
- configure _MasterSim_ project file
- run MasterSim -> should extract FMUs to directory

## Replace binaries with symlinks to debug-builds from Qt Creator

- remove so files from extracted FMU binaries
- create a symlink with same name to shared library build by Qt Creator
  (in bin/debug_x64 dir)

    ln -s ../../../../../../../../../bin/debug_x64/libLotkaVolterraPrey.so.2.0.0 Prey.so

## Build MasterSim for FMU Debugging

Open `externals/IBK/projects/Qt/CONFIG.pri` and enable option `FMU_Debugging`.
Clear directory `externals/lib_x64` and `lib_x64` and rebuild all.

## Start Debugging

- Open _MasterSim_ session
- Configure debugging using the --skip-unzip option and 
  start debugging - setting breakpoints in code (FMUs and Master)
  should work from the start
- If debugged FMU library uses other dynamically loaded libraries, configure
  the environment variable LD_LIBRARY_PATH to include that library path
  

