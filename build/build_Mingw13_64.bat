@echo off

:: Build script for MasterSim with MinGW 13 (x64) and Qt 6
::
:: Executables (and DLLs) are placed in <repo-root>\bin\release, libraries in the
:: 'lib' subdirectory of the build directory (see externals/IBK/build.cmake).

setlocal

:: add path to mingw32 compiler
PATH=%PATH%;C:\Qt\Tools\mingw1310_64\bin

:: For different Qt installations, please set the environment variable CMAKE_PREFIX_PATH
:: for the current Windows user. Also, make sure cmake is in the PATH variable.
:: zlib is taken from the MinGW tool chain.
::
:: For debugging crashes on Windows, change the CMAKE_BUILD_TYPE to "RelWithDebInfo".

:: These environment variables can also be set externally
if not defined CMAKE_PREFIX_PATH (
	set CMAKE_PREFIX_PATH=c:\Qt\6.9.3\mingw_64
)

set BUILD_DIR=bb_mingw13_64

:: create and change into build subdir
if not exist %BUILD_DIR% mkdir %BUILD_DIR%
pushd %BUILD_DIR%

:: configure makefiles and build
cmake -G "MinGW Makefiles" ..\.. -DCMAKE_BUILD_TYPE:String="Release"
if ERRORLEVEL 1 GOTO fail
mingw32-make -j12
if ERRORLEVEL 1 GOTO fail

popd
exit /b 0


:fail
popd
echo ** Build Failed **
pause
exit /b 1
