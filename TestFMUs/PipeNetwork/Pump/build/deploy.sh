#!/bin/bash

# script is supposed to be executed in /build directory

# remove target directory if it exists
if [ -d Pump ]; then
  rm -rf Pump 
fi &&

# remove target FMU if it exists
if [ -f Pump.fmu ]; then
    rm Pump.fmu 
fi &&

# create subdir and change into it
mkdir -p Pump &&
cd Pump &&

# create binary dir for Linux
mkdir -p binaries/linux64 &&

# copy shared library, we expect it to be already renamed correctly
cp ../../bin/release/Pump.so binaries/linux64/Pump.so &&
cp ../../data/modelDescription.xml . &&

# create zip archive
7za a ../Pump.zip . | cat > /dev/null &&
cd .. && 
mv Pump.zip Pump.fmu &&
echo "Created Pump.fmu" &&

# change working directory back to original dir
cd -

