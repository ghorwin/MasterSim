#!/usr/bin/python

# IBK Deployment Script System
# Copyright Andreas Nicolai <andreas.nicolai -at- tu-dresden.de>
# Released under BSD License.
#
#
# Script for configuring a build and building the application.
#
# Script is expected to be called with working directory 
# relative to ../../externals/IBK, otherwise --IBK-path argument has to be provided

import os
import platform
import argparse
import subprocess
import fileinput
import datetime
import shutil
import sys


def getCurrentOS():
	"""
	This function returns an abbreviation of the current Operating System.
	
	Return values:
	
	linux
	mac
	win
	"""

	currentOS = platform.system()

	if currentOS   == "Linux" :
		myOSName   = "linux"
	
	elif currentOS == "Windows" :
		myOSName   = "win"
	
	elif currentOS == "Darwin" :
		myOSName   = "mac"
	
	else:
		raise RuntimeError('The current Operating System is unknown.')
	return myOSName


def adjustDeploymentFlag( pathToFlag ):
	"""
	This method changes the IBK deployment flag (usually in externals/IBK/src/IBK_BuildFlags.h
	or any other file named by 'pathToFlag')
	"""

	# The search expression is required in IBK_BuildFlags.h in apps and has to be set before release.
	searchExp = "//#define IBK_DEPLOYMENT"
	replaceExp = "#define IBK_DEPLOYMENT"
	
	setDeployFlag = False
	deployFlagAlreadySet = False
	
	if not os.path.exists(pathToFlag):
		raise RuntimeError("File '{}' not found.".format(pathToFlag))

	# find the line "//#define IBK_DEPLOYMENT" and replace it with "#define IBK_DEPLOYMENT"
	fobj = open(pathToFlag, 'r')
	lines = fobj.readlines()
	fobj.close()
	del fobj

	# do not modify original file (don't update timestamp to avoid unnecessary rebuilds)
	tmpFn = pathToFlag + ".tmp"
	fobj = open(tmpFn, 'w')
	for line in lines:
		# Line must start with searchExp -> otherwise expression is changed in wrong location
		if line.find(searchExp) == 0 :
			line = line.replace( searchExp, replaceExp )
			setDeployFlag = True
		elif line.find(replaceExp) == 0:
			deployFlagAlreadySet = True
		fobj.write(line.rstrip("\r\n") + "\n")
	fobj.close()
	del fobj

	if not setDeployFlag and not deployFlagAlreadySet:
		raise RuntimeError("Deployment flag was not found in " + pathToFlag)

	if setDeployFlag:
		# if the flag was set, copy file over (this will then cause a rebuild)
		shutil.copyfile(tmpFn, pathToFlag)

	os.remove(tmpFn)


def buildApplication(build_script):
	"""
	This method starts the build.sh or build.bat script to build 
	the application in dependency of the current OS.
	
	The build script is expected to be inside this directory:
	
    ::
	
	  ../../build/cmake/
	"""
	
	workingDir = "../../build/cmake/"
	command    = ""
	currentOS = getCurrentOS()
	

	if currentOS   == "linux" or currentOS == "mac":

		if build_script == None:
			build_script = 'build.sh'
		# ../../build/cmake/build.sh

		command = './'+build_script+' 4 skip-test'

	elif currentOS == "win" :
		if build_script == None:
			build_script = 'build_VC.bat'

		# ../../build/cmake/build.bat

		#command = 'start /wait build_VC.bat'
		command = build_script

	# Open sub-process and start build script.    
	p = subprocess.Popen(command, shell=True, cwd=workingDir)

	# p.communicate() waits for termination of subprocess
	(output, err) = p.communicate()        

	if p.returncode != 0:
		print "--------------------------------------------------------------------"
		raise RuntimeError('Error during build process.')    

