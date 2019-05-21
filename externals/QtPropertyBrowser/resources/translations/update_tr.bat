@echo off
set PATH=c:\Qt\4.7.4_VC10\bin;%PATH%
lupdate -noobsolete ..\..\projects\Qt\QtPropertyBrowser.pro

pause

linguist QtPropertyBrowser_de.ts
