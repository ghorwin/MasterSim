#!/bin/bash

# script is supposed to be executed in /build directory

# remove target directory if it exists
if [ -d Valve ]; then
  rm -rf Valve 
fi &&

# remove target FMU if it exists
if [ -f Valve.fmu ]; then
    rm Valve.fmu 
fi &&

# create subdir and change into it
mkdir -p Valve &&
cd Valve &&

# create binary dir for Linux
mkdir -p binaries/linux64 &&

# copy shared library, we expect it to be already renamed correctly
cp ../../bin/release/Valve.so binaries/linux64/Valve.so &&
cp ../../data/modelDescription.xml . &&

# create zip archive
7za a ../Valve.zip . | cat > /dev/null &&
cd .. && 
mv Valve.zip Valve.fmu &&
echo "Created Valve.fmu" &&

# change working directory back to original dir
cd -

