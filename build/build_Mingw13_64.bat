@echo off

:: Build script for MasterSim with MinGW 13 (x64) and Qt 6
::
:: Command line options:
::   deploy     - set DEPLOYMENT (and IBK_DEPLOYMENT) flag during build
::   no-gui     - do not build the Qt based libraries and user interface
::
:: Executables (and DLLs) are placed in <repo-root>\bin\release, libraries in the
:: 'lib' subdirectory of the build directory (see externals/IBK/build.cmake).

setlocal enabledelayedexpansion

:: add path to mingw32 compiler
PATH=%PATH%;C:\Qt\Tools\mingw1310_64\bin

:: For different Qt installations, please set the environment variables CMAKE_PREFIX_PATH and VCPKG_ROOT
:: for the current Windows user. Also, make sure cmake is in the PATH variable.
:: zlib is provided via vcpkg (static MinGW triplet), it is installed automatically if missing.
::
:: For debugging crashes on Windows, change the CMAKE_BUILD_TYPE to "RelWithDebInfo".

:: These environment variables can also be set externally
if not defined CMAKE_PREFIX_PATH (
	set CMAKE_PREFIX_PATH=c:\Qt\6.9.3\mingw_64
)
if not defined VCPKG_ROOT (
	set "VCPKG_ROOT=C:\vcpkg"
)

:: zlib is provided via vcpkg, built with the MinGW compiler in the PATH;
:: the host triplet is set as well, so that vcpkg does not require Visual Studio
set VCPKG_TRIPLET=x64-mingw-static
set VCPKG_DEFAULT_HOST_TRIPLET=x64-mingw-static
if not exist "%VCPKG_ROOT%\installed\%VCPKG_TRIPLET%\include\zlib.h" (
	echo zlib not found, installing via vcpkg...
	call "%~dp0setup_zlib_vcpkg.bat"
	if ERRORLEVEL 1 (
		echo ERROR: Failed to setup zlib
		exit /b 1
	)
)

:: Parse arguments
set DEPLOYMENT=OFF
set DISABLE_QT=OFF
for %%a in (%*) do (
	if /i "%%a"=="deploy" (
		set DEPLOYMENT=ON
		echo Deployment build mode enabled
	)
	if /i "%%a"=="no-gui" (
		set DISABLE_QT=ON
		echo Building without GUI
	)
)
set CMAKE_OPTIONS=-DDEPLOYMENT:BOOL=%DEPLOYMENT% -DDISABLE_QT:BOOL=%DISABLE_QT%

set BUILD_DIR=bb_mingw13_64

:: create and change into build subdir
if not exist %BUILD_DIR% mkdir %BUILD_DIR%
pushd %BUILD_DIR%

:: configure makefiles and build
cmake -G "MinGW Makefiles" ..\.. %CMAKE_OPTIONS% -DCMAKE_BUILD_TYPE:String="Release" -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=%VCPKG_TRIPLET%
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
