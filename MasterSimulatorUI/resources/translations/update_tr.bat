@echo off
set PATH=c:\Qt\5.11.3\msvc2015_64\bin;%PATH%
lupdate ..\..\MasterSimulatorUI.pro

pause

linguist MasterSimulatorUI_de.ts

