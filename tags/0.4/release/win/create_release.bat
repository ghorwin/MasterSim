@echo off
set PATH=c:\Python27;%PATH%
python ..\scripts\createRelease.py -p MasterSimulator --constants-path ..\..\MasterSim\src %*

