#!/bin/bash

# script is supposed to be executed in /build directory

# remove target directory if it exists
if [ -d Pipe ]; then
  rm -rf Pipe 
fi &&

# remove target FMU if it exists
if [ -f Pipe.fmu ]; then
    rm Pipe.fmu 
fi &&

# create subdir and change into it
mkdir -p Pipe &&
cd Pipe &&

# create binary dir for Linux
mkdir -p binaries/linux64 &&

# copy shared library, we expect it to be already renamed correctly
cp ../../bin/release/Pipe.so binaries/linux64/Pipe.so &&
cp ../../data/modelDescription.xml . &&

# create zip archive
7za a ../Pipe.zip . | cat > /dev/null &&
cd .. && 
mv Pipe.zip Pipe.fmu &&
echo "Created Pipe.fmu" &&

# change working directory back to original dir
cd -

