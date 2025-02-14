@echo off

:: setup VC environment variables
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" (
	echo Loading VCVars64 Community
	call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
)

if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
	echo Loading VCVars64 Enterprise
	call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
)

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
	set CMAKE_PREFIX_PATH=C:\Qt\5.15.2\msvc2019_64
)

:: add search path for jom.exe
set PATH=%PATH%;%JOM_PATH%

:: create and change into build subdir
mkdir bb_VC_x64
pushd bb_VC_x64

:: configure makefiles and build
cmake -G "NMake Makefiles JOM" ../../.. -DCMAKE_BUILD_TYPE:String="Release"
jom
if ERRORLEVEL 1 GOTO fail

popd

:: copy executable to bin/release dir
xcopy /Y .\bb_VC_x64\MasterSimulator\mastersim.exe ..\..\bin\release_x64
xcopy /Y .\bb_VC_x64\MasterSimulatorUI\mastersim-gui.exe ..\..\bin\release_x64
xcopy /Y .\bb_VC_x64\TestFMUs\Math003Part1\Math003Part1.dll ..\..\bin\release_x64
xcopy /Y .\bb_VC_x64\TestFMUs\Math003Part2\Math003Part2.dll ..\..\bin\release_x64
xcopy /Y .\bb_VC_x64\TestFMUs\Math003Part3\Math003Part3.dll ..\..\bin\release_x64
xcopy /Y .\bb_VC_x64\TestFMUs\LotkaVolterraPredator\LotkaVolterraPredator.dll ..\..\bin\release_x64
xcopy /Y .\bb_VC_x64\TestFMUs\LotkaVolterraPrey\LotkaVolterraPrey.dll ..\..\bin\release_x64

exit /b 0


:fail
echo ** Build Failed **
exit /b 1

