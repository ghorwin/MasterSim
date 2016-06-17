@echo off

:: setup VC environment variables
set VCVARSALL_PATH="c:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"
call %VCVARSALL_PATH%

:: These environment variables can also be set externally
if not defined JOM_PATH (
	set JOM_PATH=c:\Qt\qtcreator-3.2.2\bin
)
if not defined QMAKE_PATH (
	set QMAKE_PATH=c:\Qt\4.8.6_VC10\bin\qmake.exe
)

:: add search path for jom.exe
set PATH=%PATH%;%JOM_PATH%

:: create and change into build subdir
mkdir bb_VC
pushd bb_VC

:: configure makefiles and build
cmake -G "NMake Makefiles JOM" .. -DCMAKE_BUILD_TYPE:String="Release" -DQT_QMAKE_EXECUTABLE=%QMAKE_PATH%
jom

popd

:: copy executable to bin/release dir
xcopy /Y .\bb_VC\MasterSimulator\MasterSimulator.exe ..\..\bin\release
xcopy /Y .\bb_VC\MasterSimulatorUI\MasterSimulatorUI.exe ..\..\bin\release

pause
