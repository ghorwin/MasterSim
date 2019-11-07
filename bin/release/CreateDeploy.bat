
set QTDIR=C:\Qt\5.11.3\msvc2015
set PATH=C:\Qt\5.11.3\msvc2015\bin;C:\Qt\Tools\QtCreator\bin;%PATH%

:: setup VC environment variables
set VCVARSALL_PATH="c:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat"
call %VCVARSALL_PATH% x86

windeployqt.exe MasterSimulatorUI.exe
