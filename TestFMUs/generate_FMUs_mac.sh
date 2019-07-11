#!/bin/bash

# create subdir and copy all files together

mkdir -p fmus
cd fmus

# --- Part 1 ---

if [ -d Part1 ]; then
  rm -rf Part1
fi

mkdir -p Part1/binaries/darwin64
cd Part1

cp ../../../bin/release/libMath003Part1.dylib binaries/darwin64/Part1.dylib
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

mkdir -p Part2/binaries/darwin64
cd Part2

cp ../../../bin/release/libMath003Part2.dylib binaries/darwin64/Part2.dylib
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

mkdir -p Part3/binaries/darwin64
cd Part3

cp ../../../bin/release/libMath003Part3.dylib binaries/darwin64/Part3.dylib
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

mkdir -p Prey/binaries/darwin64
cd Prey

cp ../../../bin/release/libLotkaVolterraPrey.dylib binaries/darwin64/Prey.dylib
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

mkdir -p Predator/binaries/darwin64
cd Predator

cp ../../../bin/release/libLotkaVolterraPredator.dylib binaries/darwin64/Predator.dylib
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
cp fmus/Part1.fmu ../data/tests/darwin64/Math_003_control_loop/fmus/IBK
cp fmus/Part2.fmu ../data/tests/darwin64/Math_003_control_loop/fmus/IBK
cp fmus/Part3.fmu ../data/tests/darwin64/Math_003_control_loop/fmus/IBK
cp fmus/Prey.fmu ../data/tests/darwin64/Lotka_Volterra_System/fmus/IBK
cp fmus/Predator.fmu ../data/tests/darwin64/Lotka_Volterra_System/fmus/IBK

