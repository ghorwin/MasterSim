#!/bin/bash

# create subdir and copy all files together

mkdir -p fmus
cd fmus

# --- Part 1 ---

if [ -d Part1 ]; then
  rm -rf Part1
fi

mkdir -p Part1/binaries/linux64
cd Part1

cp ../../../bin/release/libMath003Part1.so binaries/linux64/Part1.so
cp ../../Math003Part1/data/modelDescription.xml .

if [ -f ../Part1.fmu ]; then
    rm ../Part1.fmu
fi

7za a ../Part1.zip . | cat > /dev/null &&
cd .. && 
mv Part1.zip Part1.fmu &&
echo "Created Part1.fmu"


# --- Part 2 ---

if [ -d Part2 ]; then
  rm -rf Part2
fi

mkdir -p Part2/binaries/linux64
cd Part2

cp ../../../bin/release/libMath003Part2.so binaries/linux64/Part2.so
cp ../../Math003Part2/data/modelDescription.xml .

if [ -f ../Part2.fmu ]; then
    rm ../Part2.fmu
fi

7za a ../Part2.zip . | cat > /dev/null &&
cd .. &&
mv Part2.zip Part2.fmu &&
echo "Created Part2.fmu"


# --- Part 3 ---

if [ -d Part3 ]; then
  rm -rf Part3
fi

mkdir -p Part3/binaries/linux64
cd Part3

cp ../../../bin/release/libMath003Part3.so binaries/linux64/Part3.so
cp ../../Math003Part3/data/modelDescription.xml .

if [ -f ../Part3.fmu ]; then
    rm ../Part3.fmu
fi

7za a ../Part3.zip . | cat > /dev/null &&
cd .. &&
mv Part3.zip Part3.fmu &&
echo "Created Part3.fmu"


# --- Prey ---

if [ -d Prey ]; then
  rm -rf Prey
fi

mkdir -p Prey/binaries/linux64
cd Prey

cp ../../../bin/release/libLotkaVolterraPrey.so binaries/linux64/Prey.so
cp ../../LotkaVolterraPrey/data/modelDescription.xml .

if [ -f ../Prey.fmu ]; then
    rm ../Prey.fmu
fi

7za a ../Prey.zip . | cat > /dev/null &&
cd .. &&
mv Prey.zip Prey.fmu &&
echo "Created Prey.fmu"


# --- Predator ---

if [ -d Predator ]; then
  rm -rf Predator
fi

mkdir -p Predator/binaries/linux64
cd Predator

cp ../../../bin/release/libLotkaVolterraPredator.so binaries/linux64/Predator.so
cp ../../LotkaVolterraPredator/data/modelDescription.xml .

if [ -f ../Predator.fmu ]; then
    rm ../Predator.fmu
fi

7za a ../Predator.zip . | cat > /dev/null &&
cd .. &&
mv Predator.zip Predator.fmu &&
echo "Created Predator.fmu"


# go back to original directory
cd ..

# copy generates FMUs to test directory
cp fmus/Part1.fmu ../data/tests/linux64/Math_003_control_loop/fmus/IBK
cp fmus/Part2.fmu ../data/tests/linux64/Math_003_control_loop/fmus/IBK
cp fmus/Part3.fmu ../data/tests/linux64/Math_003_control_loop/fmus/IBK

