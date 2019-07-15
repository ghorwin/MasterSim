@echo off

:: create subdir and copy all files together

echo Creating directory 'fmus'
mkdir fmus
cd fmus

:: --- Part1 ---

echo Removing old 'Part1' directory
rmdir /Q /S Part1

echo Creating directory structure
mkdir Part1\binaries\win64
cd Part1

echo Copying Part1.dll and modelDescription.xml
copy ..\..\..\bin\release_x64\Math003Part1.dll binaries\win64\Part1.dll
copy ..\..\Math003Part1\data\modelDescription.xml .

echo Removing old Part1.fmu
del ..\Part1.fmu

echo Creating archive Part1.zip
7za a ..\Part1.zip .
cd .. 
echo Renaming to Part1.fmu
rename Part1.zip Part1.fmu
echo *** Done: Created Part1.fmu


:: --- Part2 ---

echo Removing old 'Part2' directory
rmdir /Q /S Part2

echo Creating directory structure
mkdir Part2\binaries\win64
cd Part2

echo Copying Part2.dll and modelDescription.xml
copy ..\..\..\bin\release_x64\Math003Part2.dll binaries\win64\Part2.dll
copy ..\..\Math003Part2\data\modelDescription.xml .

echo Removing old Part2.fmu
del ..\Part2.fmu

echo Creating archive Part2.zip
7za a ..\Part2.zip .
cd .. 
echo Renaming to Part2.fmu
rename Part2.zip Part2.fmu
echo *** Done: Created Part2.fmu


:: --- Part3 ---

echo Removing old 'Part3' directory
rmdir /Q /S Part3

echo Creating directory structure
mkdir Part3\binaries\win64
cd Part3

echo Copying Part3.dll and modelDescription.xml
copy ..\..\..\bin\release_x64\Math003Part3.dll binaries\win64\Part3.dll
copy ..\..\Math003Part3\data\modelDescription.xml .

echo Removing old Part3.fmu
del ..\Part3.fmu

echo Creating archive Part3.zip
7za a ..\Part3.zip .
cd .. 
echo Renaming to Part3.fmu
rename Part3.zip Part3.fmu
echo *** Done: Created Part3.fmu


:: --- Prey ---

echo Removing old 'Prey' directory
rmdir /Q /S Prey

echo Creating directory structure
mkdir Prey\binaries\win64
cd Prey

echo Copying Prey.dll and modelDescription.xml
copy ..\..\..\bin\release_x64\LotkaVolterraPrey.dll binaries\win64\Prey.dll
copy ..\..\LotkaVolterraPrey\data\modelDescription.xml .

echo Removing old Prey.fmu
del ..\Prey.fmu

echo Creating archive Prey.zip
7za a ..\Prey.zip .
cd .. 
echo Renaming to Prey.fmu
rename Prey.zip Prey.fmu
echo *** Done: Created Prey.fmu


:: --- Predator ---

echo Removing old 'Predator' directory
rmdir /Q /S Predator

echo Creating directory structure
mkdir Predator\binaries\win64
cd Predator

echo Copying Predator.dll and modelDescription.xml
copy ..\..\..\bin\release_x64\LotkaVolterraPredator.dll binaries\win64\Predator.dll
copy ..\..\LotkaVolterraPredator\data\modelDescription.xml .

echo Removing old Predator.fmu
del ..\Predator.fmu

echo Creating archive Predator.zip
7za a ..\Predator.zip .
cd .. 
echo Renaming to Predator.fmu
rename Predator.zip Predator.fmu
echo *** Done: Created Predator.fmu


:: go back to original directory
cd ..

:: copy generates FMUs to test directory
echo Copying fmus to test directory
copy fmus\Part1.fmu ..\data\tests\win64\Math_003_control_loop\fmus\IBK
copy fmus\Part2.fmu ..\data\tests\win64\Math_003_control_loop\fmus\IBK
copy fmus\Part3.fmu ..\data\tests\win64\Math_003_control_loop\fmus\IBK
copy fmus\Prey.fmu ..\data\tests\win64\FileReaderSlave\fmus\IBK

