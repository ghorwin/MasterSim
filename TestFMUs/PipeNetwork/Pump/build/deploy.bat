@echo off
:: script is supposed to be executed in /build directory

if exist ..\bin\release_x64\Pump.dll goto DLL_EXISTS
echo "ERROR: File Pump.dll expected in directory ..\bin\release_x64\Pump.dll, but does not exist.
exit /b 1
:DLL_EXISTS

:: remove target directory if it exists
if not exist Pump goto DIRECTORY_CLEAN
echo Removing existing directory 'Pump'
rd /S /Q "Pump"
:DIRECTORY_CLEAN

:: remove target FMU if it exists
if not exist Pump.fmu goto FMU_REMOVED
echo Removing existing FMU file 'Pump.fmu'
del /F /S /Q "Pump.fmu"
:FMU_REMOVED

::create subdir and change into it
mkdir Pump

cd Pump

:: create binary dir for Windows
mkdir binaries\win64

:: copy shared library, we expect it to be already renamed correctly
xcopy ..\..\bin\release_x64\Pump.dll binaries\win64\
xcopy ..\..\data\modelDescription.xml .
echo Created FMU directory structure

::change working directory back to original dir
cd ..

::create zip archive
echo Creating archive 'Pump.zip'
cd Pump
7za a ../Pump.zip .
cd ..

echo Renaming archive to 'Pump.fmu'
rename Pump.zip Pump.fmu

:: all ok
exit /b 0
