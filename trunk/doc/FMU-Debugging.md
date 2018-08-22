# FMU Debugging with Qt Creator on Linux


## Common setup

- generate FMUs
- configure MasterSim project file
- run MasterSim -> should extract FMUs to directory

## Replace binaries with symlinks to debug-builds from Qt Creator

- remove so files from extracted FMU binaries
- create a symlink with same name to shared library build by Qt Creator
  (in bin/debug_x64 dir)

## Start Debugging

- Start Qt Creator 
- Configure debugging using the --skip-unzip option and 
  start debugging - setting breakpoints in code (FMUs and Master)
  should work from the start




