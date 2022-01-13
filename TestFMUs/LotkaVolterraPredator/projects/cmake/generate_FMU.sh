#!/bin/bash

# This script generates the FMU, it is expected to be run in
# the TestFMUs/LotkaVolterraPredator/projects/cmake directory.
#
# Syntax: generate_FMU.sh <path/to/projectRoot> <path/to/libMath003Part1.so>
# create subdir and copy all files together

# --- Predator ---

if [ -d Predator ]; then
  rm -rf Predator
fi &&

mkdir -p Predator/binaries/linux64 &&
cd Predator &&

cp $2/libLotkaVolterraPredator.so binaries/linux64/Predator.so &&
cp $1/data/modelDescription.xml . &&

if [ -f ../Predator.fmu ]; then
    rm ../Predator.fmu
fi &&

7za a ../Predator.zip . | cat > /dev/null &&
cd .. &&
mv Predator.zip Predator.fmu &&
echo "Created Predator.fmu" &&

cd ..
