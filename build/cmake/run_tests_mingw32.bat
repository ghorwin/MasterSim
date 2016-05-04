@echo off

set PATH=c:\Qt\4.8.6_VC10\bin;%PATH%

echo *** init_test_win32_vc10.cfg ***
pushd ..\..\data\tests
..\..\bin\release_mingw32\MasterSimulator.exe init_test_win32_vc10.cfg -t --verbosity-level=2
popd

echo *** Math_003_control_loop_win32.cfg ***
pushd ..\..\data\tests
..\..\bin\release_mingw32\MasterSimulator.exe Math_003_control_loop_win32.cfg -t --verbosity-level=2 --working-dir="Math_003_control_loop"
popd

:: python ..\..\scripts\tests\run_tests.py -p ..\..\data\tests -s ..\..\bin\release\MasterSimulator.exe -e sim

pause
