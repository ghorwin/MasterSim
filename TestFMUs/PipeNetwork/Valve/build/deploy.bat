@echo off
:: script is supposed to be executed in /build directory

if exist ..\bin\release_x64\Valve.dll goto DLL_EXISTS
echo "ERROR: File Valve.dll expected in directory ..\bin\release_x64\Valve.dll, but does not exist.
exit /b 1
:DLL_EXISTS

:: remove target directory if it exists
if not exist Valve goto DIRECTORY_CLEAN
echo Removing existing directory 'Valve'
rd /S /Q "Valve"
:DIRECTORY_CLEAN

:: remove target FMU if it exists
if not exist Valve.fmu goto FMU_REMOVED
echo Removing existing FMU file 'Valve.fmu'
del /F /S /Q "Valve.fmu"
:FMU_REMOVED

::create subdir and change into it
mkdir Valve

cd Valve

:: create binary dir for Windows
mkdir binaries\win64

:: copy shared library, we expect it to be already renamed correctly
xcopy ..\..\bin\release_x64\Valve.dll binaries\win64\
xcopy ..\..\data\modelDescription.xml .
echo Created FMU directory structure

::change working directory back to original dir
cd ..

::create zip archive
echo Creating archive 'Valve.zip'
cd Valve
7za a ../Valve.zip .
cd ..

echo Renaming archive to 'Valve.fmu'
rename Valve.zip Valve.fmu

:: all ok
exit /b 0
