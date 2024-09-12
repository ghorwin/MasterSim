#!/bin/bash

# This script is executed as part of the build to create the FMUs 
# needed for installation on Unix/Linux systems.
# 
# Syntax: generate_FMUs.sh <path/to/cmake_binary dir>
# Working directory should be data/TestFMUs
#
# Example so-location: <path/to/cmake_binary dir>/Math003/libMath003Part1.so

if [ "$#" -ne 1 ]; then
    echo "Syntax: generate_FMUs.sh <path/to/cmake_binary dir>"
    exit 1
fi

mkdir -p fmus
cd fmus &&

# --- Part 1 ---

if [ -d Part1 ]; then
  rm -rf Part1
fi &&

mkdir -p Part1/binaries/linux64 &&
cd Part1 &&

cp $1/Math003Part1/libMath003Part1.so binaries/linux64/Part1.so &&
cp ../../Math003Part1/data/modelDescription.xml . &&

if [ -f ../Part1.fmu ]; then
    rm ../Part1.fmu
fi &&

7za a ../Part1.zip . | cat > /dev/null &&
cd .. && 
mv Part1.zip Part1.fmu &&
echo "Created Part1.fmu" &&


# --- Part 2 ---

if [ -d Part2 ]; then
  rm -rf Part2
fi &&

mkdir -p Part2/binaries/linux64 &&
cd Part2 &&

cp $1/Math003Part2/libMath003Part2.so binaries/linux64/Part2.so &&
cp ../../Math003Part2/data/modelDescription.xml . &&

if [ -f ../Part2.fmu ]; then
    rm ../Part2.fmu
fi &&

7za a ../Part2.zip . | cat > /dev/null &&
cd .. &&
mv Part2.zip Part2.fmu &&
echo "Created Part2.fmu" &&


# --- Part 3 ---

if [ -d Part3 ]; then
  rm -rf Part3
fi &&

mkdir -p Part3/binaries/linux64 &&
cd Part3 &&

cp $1/Math003Part3/libMath003Part3.so binaries/linux64/Part3.so &&
cp ../../Math003Part3/data/modelDescription.xml . &&

if [ -f ../Part3.fmu ]; then
    rm ../Part3.fmu
fi &&

7za a ../Part3.zip . | cat > /dev/null &&
cd .. &&
mv Part3.zip Part3.fmu &&
echo "Created Part3.fmu" &&


# --- Prey ---

if [ -d Prey ]; then
  rm -rf Prey
fi &&

mkdir -p Prey/binaries/linux64 &&
cd Prey &&

cp $1/LotkaVolterraPrey/libLotkaVolterraPrey.so binaries/linux64/Prey.so &&
cp ../../LotkaVolterraPrey/data/modelDescription.xml . &&

if [ -f ../Prey.fmu ]; then
    rm ../Prey.fmu
fi &&

7za a ../Prey.zip . | cat > /dev/null &&
cd .. &&
mv Prey.zip Prey.fmu &&
echo "Created Prey.fmu" &&


# --- Predator ---

if [ -d Predator ]; then
  rm -rf Predator
fi &&

mkdir -p Predator/binaries/linux64 &&
cd Predator &&

cp $1/LotkaVolterraPredator/libLotkaVolterraPredator.so binaries/linux64/Predator.so &&
cp ../../LotkaVolterraPredator/data/modelDescription.xml . &&

if [ -f ../Predator.fmu ]; then
    rm ../Predator.fmu
fi &&

7za a ../Predator.zip . | cat > /dev/null &&
cd .. &&
mv Predator.zip Predator.fmu &&
echo "Created Predator.fmu" &&


# --- FourRealInputVars ---

if [ -d FourRealInputVars ]; then
  rm -rf FourRealInputVars
fi &&

mkdir -p FourRealInputVars/binaries/linux64 &&
cd FourRealInputVars &&

cp $1/FourRealInputVars/libFourRealInputVars.so binaries/linux64/FourRealInputVars.so &&
cp ../../FourRealInputVars/data/modelDescription.xml . &&

if [ -f ../FourRealInputVars.fmu ]; then
    rm ../FourRealInputVars.fmu
fi &&

7za a ../FourRealInputVars.zip . | cat > /dev/null &&
cd .. &&
mv FourRealInputVars.zip FourRealInputVars.fmu &&
echo "Created FourRealInputVars.fmu" &&



# go back to original directory
cd .. &&

# copy generated FMUs to test directory, if it exists (in Debian deployment mode this
# directory is omitted)
if [ -d ../data/tests/linux64 ]; then
	echo "Updating test FMUs in data/tests/linux64" &&
	cp fmus/Part1.fmu ../data/tests/linux64/Math_003_control_loop/fmus/IBK &&
	cp fmus/Part2.fmu ../data/tests/linux64/Math_003_control_loop/fmus/IBK &&
	cp fmus/Part3.fmu ../data/tests/linux64/Math_003_control_loop/fmus/IBK &&
	cp fmus/Prey.fmu ../data/tests/linux64/Lotka_Volterra_System/fmus/IBK &&
	cp fmus/Predator.fmu ../data/tests/linux64/Lotka_Volterra_System/fmus/IBK &&
	cp fmus/FourRealInputVars.fmu ../data/tests/linux64/FileReaderSlave/fmus/IBK
fi
