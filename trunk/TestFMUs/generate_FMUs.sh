#!/bin/bash

# create subdir and copy all files together

mkdir fmus
cd fmus

# --- Part 1 ---

mkdir Part1
cd Part1

mkdir binaries
mkdir binaries/linux64
cp ../../../bin/release/libMath003Part1.so binaries/linux64
cp ../../Math003Part1/data/modelDescription.xml .

rm ../Part1.fmu

7za a ../Part1.zip .
cd ..
mv Part1.zip Part1.fmu


# --- Part 2 ---


mkdir Part2
cd Part2

mkdir binaries
mkdir binaries/linux64
cp ../../../bin/release/libMath003Part2.so binaries/linux64
cp ../../Math003Part2/data/modelDescription.xml .

rm ../Part2.fmu

7za a ../Part2.zip .
cd ..
mv Part2.zip Part2.fmu


# --- Part 3 ---


mkdir Part3
cd Part3

mkdir binaries
mkdir binaries/linux64
cp ../../../bin/release/libMath003Part3.so binaries/linux64
cp ../../Math003Part3/data/modelDescription.xml .

rm ../Part3.fmu

7za a ../Part3.zip .
cd ..
mv Part3.zip Part3.fmu


# go back to original directory
cd ..

