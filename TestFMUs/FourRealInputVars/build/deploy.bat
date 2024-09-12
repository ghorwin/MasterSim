@echo off
:: script is supposed to be executed in /build directory

if exist ..\bin\release_x64\FourRealInputVars.dll goto DLL_EXISTS
echo "ERROR: File FourRealInputVars.dll expected in directory ..\bin\release_x64\FourRealInputVars.dll, but does not exist.
exit /b 1
:DLL_EXISTS

:: remove target directory if it exists
if not exist FourRealInputVars goto DIRECTORY_CLEAN
echo Removing existing directory 'FourRealInputVars'
rd /S /Q "FourRealInputVars"
:DIRECTORY_CLEAN

:: remove target FMU if it exists
if not exist FourRealInputVars.fmu goto FMU_REMOVED
echo Removing existing FMU file 'FourRealInputVars.fmu'
del /F /S /Q "FourRealInputVars.fmu"
:FMU_REMOVED

::create subdir and change into it
mkdir FourRealInputVars

cd FourRealInputVars

:: create binary dir for Windows
mkdir binaries\win64

:: copy shared library, we expect it to be already renamed correctly
xcopy ..\..\bin\release_x64\FourRealInputVars.dll binaries\win64\
xcopy ..\..\data\modelDescription.xml .
echo Created FMU directory structure

::change working directory back to original dir
cd ..

::create zip archive
echo Creating archive 'FourRealInputVars.zip'
cd FourRealInputVars
7za a ../FourRealInputVars.zip .
cd ..

echo Renaming archive to 'FourRealInputVars.fmu'
rename FourRealInputVars.zip FourRealInputVars.fmu

:: all ok
exit /b 0
