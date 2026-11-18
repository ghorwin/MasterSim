@echo off

:: Build script for MasterSim with Visual Studio 2022 (x64) and Qt 6
::
:: Command line options:
::   deploy     - set DEPLOYMENT (and IBK_DEPLOYMENT) flag during build
::   no-gui     - do not build the Qt based libraries and user interface
::
:: Executables (and DLLs) are placed in <repo-root>\bin\release, libraries in the
:: 'lib' subdirectory of the build directory (see externals/IBK/build.cmake).

:: Modify environment variables only within this batch file - changes are not persistent after leaving batch
:: This allows calling this batch file several times within the same command window.
setlocal enabledelayedexpansion

:: vcvars64.bat sets VCPKG_ROOT to the vcpkg bundled with Visual Studio (manifest mode only),
:: hence remember the externally set vcpkg location (or the default) and restore it afterwards
if defined VCPKG_ROOT (
	set "MSIM_VCPKG_ROOT=%VCPKG_ROOT%"
) else (
	set "MSIM_VCPKG_ROOT=C:\vcpkg"
)

:: setup VC environment variables - check multiple possible locations
set VC_FOUND=0
for %%e in (BuildTools Community Professional Enterprise) do (
	if !VC_FOUND!==0 call :find_vc %%e
)
if %VC_FOUND%==0 (
	echo ERROR: Could not find Visual Studio 2022 installation
	echo Checked: BuildTools, Community, Professional, Enterprise
	exit /b 1
)

:: For different Qt installations, please set the environment variables QT_ROOT, VCPKG_ROOT and JOM_PATH
:: for the current Windows user. Also, make sure cmake is in the PATH variable.
::
:: For debugging crashes on Windows, change the CMAKE_BUILD_TYPE to "RelWithDebInfo".

:: These environment variables can also be set externally
if not defined QT_ROOT (
	set QT_ROOT=C:\Qt\6.9.3\msvc2022_64
)
set "VCPKG_ROOT=%MSIM_VCPKG_ROOT%"
if not defined JOM_PATH (
	set JOM_PATH=c:\Qt\Tools\QtCreator\bin\jom
)

:: zlib is provided via vcpkg, install it if missing
if not exist "%VCPKG_ROOT%\installed\x64-windows\lib\zlib.lib" (
	echo zlib not found, installing via vcpkg...
	call "%~dp0setup_zlib_vcpkg.bat"
	if ERRORLEVEL 1 (
		echo ERROR: Failed to setup zlib
		exit /b 1
	)
)

set CMAKE_PREFIX_PATH=%QT_ROOT%;%VCPKG_ROOT%\installed\x64-windows

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

:: add search path for jom.exe
set PATH=%PATH%;%JOM_PATH%
set BUILD_DIR=bb_VC2022_x64

:: create and change into build subdir
if not exist %BUILD_DIR% mkdir %BUILD_DIR%
pushd %BUILD_DIR%

:: configure makefiles and build
cmake -G "NMake Makefiles JOM" ..\.. %CMAKE_OPTIONS% -DCMAKE_BUILD_TYPE:String="Release" -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake"
if ERRORLEVEL 1 GOTO fail
jom
if ERRORLEVEL 1 GOTO fail

popd
exit /b 0


:fail
popd
echo ** Build Failed **
exit /b 1


:: Subroutine: loads vcvars64.bat of the given Visual Studio 2022 edition, if installed
:find_vc
set VC_PATH=C:\Program Files\Microsoft Visual Studio\2022\%1\VC\Auxiliary\Build\vcvars64.bat
if not exist "%VC_PATH%" set VC_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2022\%1\VC\Auxiliary\Build\vcvars64.bat
if exist "%VC_PATH%" (
	echo Loading VCVars64 2022 %1
	call "%VC_PATH%"
	set VC_FOUND=1
)
exit /b 0
