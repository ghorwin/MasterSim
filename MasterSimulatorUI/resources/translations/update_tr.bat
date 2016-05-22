@echo off
set PATH=c:\Qt\4.8.6_VC10\bin;%PATH%
lupdate -noobsolete ..\..\projects\Qt\MasterSimulatorUI.pro

pause

linguist MasterSimulatorUI_de.ts

