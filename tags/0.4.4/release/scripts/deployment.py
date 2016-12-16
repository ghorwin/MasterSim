#!/usr/bin/python

# This file holds functions that configure deployment/installer scripts.
# Script is expected to be called with working directory equal to the respective
# platform release scripts, i.e. ./release/win or ./release/linux.

import os
import platform
import argparse
import subprocess
import fileinput
import datetime
import shutil
import sys

from ProductConfig import ProductConfig
from build import getCurrentOS

# adjust this path if inno setup is installed in a different place
INNO_SETUP_PATH   = r"c:\Program Files (x86)\Inno Setup 5\ISCC.exe"


def extractVersionNumber( pathToConstants ):
	"""
	The version number is a constant in IBK solvers and apps and will be taken to identify the current status.
	After extracting the current number it can be used as suffix in folder names, etc.
	To read the version number the IBK specific notation has to be realized in the *Constants.cpp of solver or app.

	Returns a tuple of (version, longVersion)
	"""
	versionNumber        = ""
	searchExpVersion     = "const char * const VERSION"
	searchExpLongVersion = "const char * const LONG_VERSION"

	version              = None
	longVersion         = None

	for root, dirs, files in os.walk( pathToConstants, topdown=True, onerror=None, followlinks=False ):

		for currentFile in files:

			if "Constants.cpp" in currentFile:
				print "Extracting constants from file: {}".format(currentFile)
				# open file to get LONG_VERSION and VERSION
				pathToFile = os.path.join( root, currentFile )

				with open(pathToFile, mode='r', buffering=1) as f:

					for line in f.readlines():

						if searchExpVersion in line:
							versionNumber = line.split("=")[1]
							version = versionNumber.strip(" \";\n")

						if searchExpLongVersion in line:
							versionNumber = line.split("=")[1]
							longVersion = versionNumber.strip(" \";\n")

				f.close()

				if longVersion == None or version == None:
					raise RuntimeError("Missing or invalid version/long version number in file '{}'"
									   .format(pathToFile))

				return version, longVersion

	if longVersion == None or version == None:
		raise RuntimeError("Missing xxxConstants.cpp file in directory '{}'.".format(pathToConstants))

	return version, longVersion


def replacePlaceholders(fn, keyValues):
	"""
	Opens file 'fn' and searches for placeholders in format ${xxx} and replaces placeholders with
	values from map.

	Arguments
	---------

	- *fn*         file path
	- *keyValues*  dictionary of placeholder-value pairs, keys do not include ${}
	"""

	# read content of file
	if not os.path.exists(fn):
		raise RuntimeError("File '{}' not found.".format(fn))
	fobj = open(fn)
	lines = fobj.readlines()
	fobj.close()
	del fobj

	# write substituted content of file into temporary file (avoid corrupting original file)
	tmpFn = fn+".tmp"
	fobj = open(tmpFn, 'w')
	try:
		substitution_count = 0;
		for line in lines:

			pos = line.find("${")
			while pos != -1:
				pos2 = line.find("}", pos)
				if pos2 != -1 and pos2 > pos:
					placeholder = line[pos+2:pos2]

					found = False
					for key,value in keyValues.iteritems():
						if placeholder == key:
							line = line[0:pos] + value + line[pos2+1:]
							found = True
							break;
					if not found:
						raise RuntimeError("Unknown placeholder '{}'".format(placeholder))
				pos = line.find("${")

			fobj.write(line.rstrip("\n\r") + "\n")
	except Exception as e:
		fobj.close()
		del fobj
		os.remove(tmpFn) # remove tmp file again
		print e.message
		raise RuntimeError("Error processing file '{}' and writing '{}'".format(fn, tmpFn))

	fobj.close()
	del fobj
	shutil.copyfile(tmpFn, fn) # copy tmp file over to original file
	os.remove(tmpFn) # remove tmp file again



def configDeployment( product, nightlyBuild, x64):
	"""
	This method replaces variable values in deploy.sh or deploy.iss files.
	The files are expected relative to this script ../<OS>/deploy.sh or ../<currentOS>/deploy.iss
	"""

	suffix = ""
	if nightlyBuild:
		suffix = datetime.datetime.now().strftime("_%Y-%m-%d")
		product.parameters["OutputFileSuffix"] = suffix
	else:
		product.parameters["OutputFileSuffix"] = ""

	currentDate = datetime.datetime.now()
	currentYear = str(currentDate.year) 

	product.parameters["Version"] = product.version
	product.parameters["LongVersion"] = product.longVersion
	product.parameters["ProductID"] = product.productID
	product.parameters["CurrentYear"] = currentYear

	currentOS = getCurrentOS()
	if currentOS   == "linux":
		# ../linux/deploy.sh

		pathToDeploy = os.path.join( os.getcwd(), '../linux', 'deploy.sh' )

		# create copy of input/template file
		shutil.copyfile(pathToDeploy+".in", pathToDeploy) 

		product.parameters["OutputDirectoryName"] = product.productID + "-" + product.version
		product.parameters["OutputFileBasename"] = product.productID + "_" + product.longVersion + "_linux" + suffix
		product.parameters["CreateArchive"] = """# finally create 7z
7z a -t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on $OUTPUT_FILE_BASENAME.7z $OUTPUT_DIRECTORY_NAME &&

echo "Created '$OUTPUT_FILE_BASENAME.7z'"
"""

	elif currentOS == "mac" :
		# ../mac/deploy.sh

		pathToDeploy = os.path.join( os.getcwd(), '../mac', 'deploy.sh' )

		# create copy of input/template file
		shutil.copyfile(pathToDeploy+".in", pathToDeploy)

		product.parameters["OutputDirectoryName"] = product.productID + "-" + product.version
		product.parameters["OutputFileBasename"] = product.productID + "_" + product.longVersion + "_macosx" + suffix

		# ../mac/plist.info
		fullPath = os.path.join( os.getcwd(), '../mac', 'Info.plist' )
		if not os.path.exists(fullPath+".in"):
			raise RuntimeError("The file 'Info.plist.in' could not be found.".format(fullPath))
		shutil.copyfile(fullPath+".in", fullPath)
		replacePlaceholders(fullPath, product.parameters)

	elif currentOS == "win" :
		# ../win/deploy.iss

		if x64:
			pathToDeploy = os.path.join( os.getcwd(), '..\\win', 'deploy64.iss' )
			product.parameters["OutputFileBasename"] = product.productID + "_" + product.longVersion + "_win64" + suffix
		else:
			pathToDeploy = os.path.join( os.getcwd(), '..\\win', 'deploy.iss' )
			product.parameters["OutputFileBasename"] = product.productID + "_" + product.longVersion + "_win" + suffix

	# create copy of input/template file
	shutil.copyfile(pathToDeploy+".in", pathToDeploy) 

	# set executable bits
	if currentOS   == "linux" or currentOS == "mac":
		os.chmod(pathToDeploy, 0755)

	# Add predefined path placeholders	
	replacePlaceholders(pathToDeploy, product.parameters)

	return pathToDeploy


def deploy(deploy_script):
	"""
	This method starts the deployment script or iss compiler.
	The current working directory is expected to be the directory
	with the deployment script.
	"""

	workingDir = os.getcwd()
	command    = ""
	currentOS = getCurrentOS()


	if currentOS   == "linux" or currentOS == "mac":
		command = deploy_script

	elif currentOS == "win" :
		command = '\"' + INNO_SETUP_PATH + '" ' + deploy_script

	# Open sub-process and start build script.    
	p = subprocess.Popen(command, shell=True, cwd=workingDir)

	# p.communicate() waits for termination of subprocess
	(output, err) = p.communicate()        

	if p.returncode != 0:
		print "--------------------------------------------------------------------"
		raise RuntimeError('Error during deployment.')    

