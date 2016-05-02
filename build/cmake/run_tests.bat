@echo off

set PATH=c:\Qt\4.8.6_VC10\bin;%PATH%

pushd ..\..\data\tests
..\..\bin\release\MasterSimulator.exe init_test_win32_vc10.cfg -t --verbosity-level=2
popd


:: python ..\..\scripts\tests\run_tests.py -p ..\..\data\tests -s ..\..\bin\release\MasterSimulator.exe -e sim

pause
