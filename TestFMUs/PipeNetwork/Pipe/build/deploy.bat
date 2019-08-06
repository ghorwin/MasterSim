@echo off
:: script is supposed to be executed in /build directory

if exist ..\bin\release_x64\Pipe.dll goto DLL_EXISTS
echo "ERROR: File Pipe.dll expected in directory ..\bin\release_x64\Pipe.dll, but does not exist.
exit /b 1
:DLL_EXISTS

:: remove target directory if it exists
if not exist Pipe goto DIRECTORY_CLEAN
echo Removing existing directory 'Pipe'
rd /S /Q "Pipe"
:DIRECTORY_CLEAN

:: remove target FMU if it exists
if not exist Pipe.fmu goto FMU_REMOVED
echo Removing existing FMU file 'Pipe.fmu'
del /F /S /Q "Pipe.fmu"
:FMU_REMOVED

::create subdir and change into it
mkdir Pipe

cd Pipe

:: create binary dir for Windows
mkdir binaries\win64

:: copy shared library, we expect it to be already renamed correctly
xcopy ..\..\bin\release_x64\Pipe.dll binaries\win64\
xcopy ..\..\data\modelDescription.xml .
echo Created FMU directory structure

::change working directory back to original dir
cd ..

::create zip archive
echo Creating archive 'Pipe.zip'
cd Pipe
7za a ../Pipe.zip .
cd ..

echo Renaming archive to 'Pipe.fmu'
rename Pipe.zip Pipe.fmu

:: all ok
exit /b 0
