@echo off

:: Modify environment variables only within this batch file - changes are not persistent after leaving batch
:: This allows calling this batch file several times within the same command window.
setlocal

:: setup VC environment variables - check multiple possible locations
set VC_FOUND=0

:: Check BuildTools (x86 path)
set VC_PATH="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if exist %VC_PATH% (
	echo Loading VCVars64 2022 BuildTools
	call %VC_PATH%
	set VC_FOUND=1
)

:: Check Community
if %VC_FOUND%==0 (
	set VC_PATH="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
	if exist %VC_PATH% (
		echo Loading VCVars64 2022 Community
		call %VC_PATH%
		set VC_FOUND=1
	)
)

:: Check Professional
if %VC_FOUND%==0 (
	set VC_PATH="C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
	if exist %VC_PATH% (
		echo Loading VCVars64 2022 Professional
		call %VC_PATH%
		set VC_FOUND=1
	)
)

:: Check Enterprise
if %VC_FOUND%==0 (
	set VC_PATH="C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
	if exist %VC_PATH% (
		echo Loading VCVars64 2022 Enterprise
		call %VC_PATH%
		set VC_FOUND=1
	)
)

if %VC_FOUND%==0 (
	echo ERROR: Could not find Visual Studio 2022 installation
	echo Checked: BuildTools, Community, Professional, Enterprise
	exit /b 1
)

:: Select Qt version
set QT_ROOT=C:\Qt\6.9.3\msvc2022_64

:: Wichtig für find_package(Qt6 ...)
set CMAKE_PREFIX_PATH=%QT_ROOT%;%VCPKG_ROOT%\installed\x64-windows
set Qt6_DIR=%QT_ROOT%\lib\cmake\Qt6

:: Check if zlib is installed via vcpkg, if not install it
set VCPKG_ROOT=C:\vcpkg
if not exist "%VCPKG_ROOT%\installed\x64-windows\lib\zlib.lib" (
    echo zlib not found, installing via vcpkg...
    call "%~dp0setup_zlib_vcpkg.bat"
    if ERRORLEVEL 1 (
        echo ERROR: Failed to setup zlib
        exit /b 1
    )
)

:: These environment variables can also be set externally
if not defined JOM_PATH (
	set JOM_PATH=c:\Qt\Tools\QtCreator\bin\jom
)

:: Parse arguments for deploy flag
for %%a in (%*) do (
	if /i "%%a"=="deploy" (
		set CMAKE_OPTIONS=%CMAKE_OPTIONS% -DIBK_DEPLOYMENT:BOOL=ON
		echo Deployment build mode enabled
	)
)

:: add search path for jom.exe
set PATH=%PATH%;%JOM_PATH%

set BUILD_DIR=bb_VC_2022_Qt6_x64
:: create and change into build subdir
mkdir %BUILD_DIR%
pushd %BUILD_DIR%

:: configure makefiles and build
cmake -G "NMake Makefiles JOM" ../../.. %CMAKE_OPTIONS% -DCMAKE_BUILD_TYPE:String="Release" -DUSE_OMP:BOOL=ON -DVICUS_QT_VERSION:STRING=6 -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake
jom 
if ERRORLEVEL 1 GOTO fail

popd

:: copy executable to bin/release dir
xcopy /Y .\%BUILD_DIR%\View3D\View3D.exe ..\..\bin\release_x64
xcopy /Y .\%BUILD_DIR%\NandradSolver\NandradSolver.exe ..\..\bin\release_x64
xcopy /Y .\%BUILD_DIR%\SIM-VICUS\SIM-VICUS.exe ..\..\bin\release_x64
xcopy /Y .\%BUILD_DIR%\NandradSolverFMI\NandradSolverFMI.dll ..\..\bin\release_x64

exit /b 0

:fail
echo ** Build Failed **
exit /b 1

