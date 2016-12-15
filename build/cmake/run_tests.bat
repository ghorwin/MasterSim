@echo off

::set PATH=c:\Qt\4.8.6_VC10\bin;%PATH%

python ..\..\scripts\tests\run_tests.py -p ..\..\data\tests -s ..\..\bin\release\MasterSimulator.exe -e msim

pause
