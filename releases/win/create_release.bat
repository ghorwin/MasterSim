@echo off

:: we expect "git", "svn" and "python" to be in the PATH variable, where Python 3 shall be used

if not defined GIT_WORKING_DIR (
	set GIT_WORKING_DIR=.
)

:: first the Proxy setup - pull the actual source code from GitHub

if exist %GIT_WORKING_DIR%\MasterSim.git (
	:: reset repo
	echo Git working directory exists, just pulling changes
	git pull --rebase %GIT_WORKING_DIR%\MasterSim.git
) ELSE (
	:: clone repo
	git clone https://github.com/ghorwin/MasterSim.git %GIT_WORKING_DIR%\MasterSim.git
)

:: now copy script directory and helper files so that the repository is again 
:: matching the required directory structure
mkdir %GIT_WORKING_DIR%\MasterSim.git\release
mkdir %GIT_WORKING_DIR%\MasterSim.git\release\scripts
mkdir %GIT_WORKING_DIR%\MasterSim.git\release\win

copy /Y ..\scripts\* %GIT_WORKING_DIR%\MasterSim.git\release\scripts\
copy /Y deploy64.iss.in %GIT_WORKING_DIR%\MasterSim.git\release\win\deploy64.iss.in

:: the deployment libs are sourced from a different SVN repo
if exist %GIT_WORKING_DIR%\MasterSim.git\release\win\VC19\ (
	svn up %GIT_WORKING_DIR%\MasterSim.git\release\win\VC19\
) ELSE (
	svn co svn+ssh://141.30.43.38/srv/svn/WinDeploymentLibs/VC19  %GIT_WORKING_DIR%\MasterSim.git\release\win\VC19
)
if ERRORLEVEL 1 GOTO fail


:: now do the regular release tool script magic
pushd %GIT_WORKING_DIR%\MasterSim.git\release\win
python ..\scripts\createRelease.py -p MasterSimulator --constants-path ..\..\MasterSim\src %*
popd
if ERRORLEVEL 1 GOTO fail

:: copy executable to current working directory
copy /Y %GIT_WORKING_DIR%\MasterSim.git\release\win\*.exe .
if ERRORLEVEL 1 GOTO fail

exit /b 0 

:fail
exit /b 1 
