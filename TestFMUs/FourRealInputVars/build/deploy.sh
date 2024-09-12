#!/bin/bash

# script is supposed to be executed in /build directory

# remove target directory if it exists
if [ -d FourRealInputVars ]; then
  rm -rf FourRealInputVars 
fi &&

# remove target FMU if it exists
if [ -f FourRealInputVars.fmu ]; then
    rm FourRealInputVars.fmu 
fi &&

# create subdir and change into it
mkdir -p FourRealInputVars &&
cd FourRealInputVars &&

# create binary dir for Linux
mkdir -p binaries/linux64 &&

# copy shared library, we expect it to be already renamed correctly
cp ../../bin/release/FourRealInputVars.so binaries/linux64/FourRealInputVars.so &&
cp ../../data/modelDescription.xml . &&

# create zip archive
7za a ../FourRealInputVars.zip . | cat > /dev/null &&
cd .. && 
mv FourRealInputVars.zip FourRealInputVars.fmu &&
echo "Created FourRealInputVars.fmu" &&

# change working directory back to original dir
cd -

