@echo off

:: setup VC environment variables
set VCVARSALL_PATH="c:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat"
call %VCVARSALL_PATH%

:: These environment variables can also be set externally
if not defined JOM_PATH (
	set JOM_PATH=c:\Qt\Qt5.10_VC14\Tools\QtCreator\bin
)

if not defined CMAKE_PREFIX_PATH (
	set CMAKE_PREFIX_PATH=c:\Qt\Qt5.10_VC14\5.10\msvc2015
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

exit /b 0

:fail
pause
exit /b 1


