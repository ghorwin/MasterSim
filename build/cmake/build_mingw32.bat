@echo off

:: These environment variables can also be set externally
if not defined JOM_PATH (
	set JOM_PATH=c:\Qt\qtcreator-3.2.2\bin
)
if not defined QMAKE_PATH (
	set QMAKE_PATH=c:\Qt\4.8.6\bin\qmake.exe
)

:: add search path for jom.exe
set PATH=%PATH%;c:\mingw32\bin;%JOM_PATH%

:: create and change into build subdir
mkdir bb_mingw32
pushd bb_mingw32

:: configure makefiles and build
cmake -G "MinGW Makefiles" .. -DCMAKE_BUILD_TYPE:String="RelWithDebInfo" -DQT_QMAKE_EXECUTABLE=%QMAKE_PATH%
jom

popd

:: copy executable to bin/release dir
mkdir ..\..\bin\release_mingw32
xcopy /Y .\bb_mingw32\MasterSimulator\MasterSimulator.exe ..\..\bin\release_mingw32
xcopy /Y .\bb_mingw32\MasterSimulatorUI\MasterSimulatorUI.exe ..\..\bin\release_mingw32

