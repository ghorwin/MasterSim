@echo off

::set PATH=c:\Qt\4.8.6_VC10\bin;%PATH%


echo *** Running win32 tests ***
python ..\..\scripts\TestSuite\run_tests.py -p ..\..\data\tests\win32 -s ..\..\bin\release\MasterSimulator.exe -e msim

pause
