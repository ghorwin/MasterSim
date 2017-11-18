@echo off
set MASTERSIM=..\..\..\..\bin\release\MasterSimulator.exe

%MASTERSIM% Math019_SimX_GaussSeidelIterativeErrorTest.msim -x
%MASTERSIM% Math019_SimX_NewtonErrorTest.msim -x
pause
