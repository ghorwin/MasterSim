@echo off

echo *** Running win64 tests ***
python ..\..\scripts\TestSuite\run_tests.py -p ..\..\data\tests\win64 -s ..\..\bin\release_x64\MasterSimulator.exe -e msim

pause
