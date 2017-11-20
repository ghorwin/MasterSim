@echo off
set MASTERSIM=..\..\..\..\bin\release\MasterSimulator.exe

%MASTERSIM% Math003_SimX_GaussJacobi_1iters_fixedStep.msim -x
%MASTERSIM% Math003_SimX_GaussJacobi_1iters_variableStep_Richardson.msim -x
%MASTERSIM% Math003_SimX_GaussSeidel_1iters_fixedStep.msim -x
%MASTERSIM% Math003_SimX_GaussSeidel_1iters_variableStep_Richardson.msim -x
%MASTERSIM% Math003_SimX_GaussSeidel_2iters_variableStep_Richardson.msim -x
%MASTERSIM% Math003_SimX_GaussSeidel_3iters_variableStep_Richardson.msim -x
pause
