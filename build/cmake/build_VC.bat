@echo off

:: setup VC environment variables
set VCVARSALL_PATH="c:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat"
call %VCVARSALL_PATH%

:: For different Qt installations, please set the environment variables JOM_PATH and CMAKE_PREFIX_PATH
:: for the current Windows user. Also, make sure cmake is in the PATH variable.
:: Mind: the dlls in the release/win/VC14_xxx subdirectories must match the Qt version for building.
::       You must copy the Qt dlls used for building into these directories.
::
:: For debugging crashes on Windows, change the CMAKE_BUILD_TYPE to "RelWithDebInfo".

:: These environment variables can also be set externally
if not defined JOM_PATH (
	set JOM_PATH=c:\Qt\Tools\QtCreator\bin
)

if not defined CMAKE_PREFIX_PATH (
	set CMAKE_PREFIX_PATH=c:\Qt\5.11.3\msvc2015
)

:: add search path for jom.exe
set PATH=%PATH%;%JOM_PATH%

:: create and change into build subdir
mkdir bb_VC
pushd bb_VC

:: configure makefiles and build
cmake -G "NMake Makefiles JOM" .. -DCMAKE_BUILD_TYPE:String="Release"
jom
if ERRORLEVEL 1 GOTO fail

popd


:: copy executable to bin/release dir
xcopy /Y .\bb_VC\MasterSimulator\MasterSimulator.exe ..\..\bin\release
xcopy /Y .\bb_VC\MasterSimulatorUI\MasterSimulatorUI.exe ..\..\bin\release
xcopy /Y .\bb_VC\Math003Part1\Math003Part1.dll ..\..\bin\release
xcopy /Y .\bb_VC\Math003Part2\Math003Part2.dll ..\..\bin\release
xcopy /Y .\bb_VC\Math003Part3\Math003Part3.dll ..\..\bin\release

exit /b 0

:fail
pause
exit /b 1


