#!/bin/bash

# create subdir and copy all files together

mkdir -p fmus
cd fmus

# --- Part 1 ---

mkdir -p Part1/binaries/linux64
cd Part1

cp ../../../bin/release/libMath003Part1.so binaries/linux64/Part1.so
cp ../../Math003Part1/data/modelDescription.xml .

if [ -f ../Part1.fmu ]; then
    rm ../Part1.fmu
fi

7za a ../Part1.zip . > nul && 
cd .. && 
mv Part1.zip Part1.fmu &&
echo "Created Part1.fmu"


# --- Part 2 ---


mkdir -p Part2/binaries/linux64
cd Part2

cp ../../../bin/release/libMath003Part2.so binaries/linux64/Part1.so
cp ../../Math003Part2/data/modelDescription.xml .

if [ -f ../Part2.fmu ]; then
    rm ../Part2.fmu
fi

7za a ../Part2.zip . > nul &&
cd .. &&
mv Part2.zip Part2.fmu &&
echo "Created Part2.fmu"


# --- Part 3 ---


mkdir -p Part3/binaries/linux64
cd Part3

cp ../../../bin/release/libMath003Part3.so binaries/linux64/Part1.so
cp ../../Math003Part3/data/modelDescription.xml .

if [ -f ../Part3.fmu ]; then
    rm ../Part3.fmu
fi

7za a ../Part3.zip . > nul &&
cd .. &&
mv Part3.zip Part3.fmu &&
echo "Created Part3.fmu"


# go back to original directory
cd ..

