@echo off
set QTDIR=C:\Qt\5.15.2\msvc2019
set PATH=c:\Qt\5.15.2\msvc2019\bin;%PATH%

:: setup VC environment variables
set VCVARSALL_PATH="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars32.bat"
call %VCVARSALL_PATH%

windeployqt.exe MasterSimulatorUI.exe
