#!/usr/bin/python

# Script for configuring a build and building the application, then configuring deployment and
# creating installer.
#
# Script is expected to be called with the platform-specific release subdirectory (win, linux, or mac)
#
# Note: The working directory must be relative to ../../externals/IBK, otherwise --IBK-path argument 
#       has to be provided

import os
import platform
import argparse
import subprocess
import fileinput
import datetime
import shutil
import sys

from build import *
from deployment import *
from ProductConfig import ProductConfig

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


def configCommandLineArguments():
	"""
	This method sets the available input parameters and parses them.

	Returns a configured argparse.ArgumentParser object.
	"""

	parser = argparse.ArgumentParser("parameter list")

	parser.add_argument('-p','--product', dest='product', required=True, type=str, 
	                    help='ID name of the product to build/deploy (must match product ID in products.ini if this '
	                    'file exists, otherwise product string will be used to name installer file).')
	parser.add_argument('--build-script', dest="build_script", required=False, type=str, 
	                    help='Alternative script file name (if missing,  build.sh or build_VC.bat are assumed, '
	                    'respective to the current platform).')
	parser.add_argument('--x64', dest='x64', required=False, action='store_true', default=False,
	                    help='Only meaningful for Windows builds. If set, the automatically generated build script name is changed from "build_VC.bat" to "build_VC_x64.bat", the '
						'deployment script name is changed from "deploy.iss" to "deploy64.iss", and "64" is appended to the win-output file name.')
	parser.add_argument('--constants-path', dest='constantsPath', required=True, type=str, 
	                    help='Path to directory containing the xxxConstants.cpp file with version and '
	                    'long version number.')
	parser.add_argument('--silent', dest='silent', required=False, action='store_true', default=False,
	                    help='To skip output of release checklist to user and wait for keypress after individual steps.')
	parser.add_argument('--nightly', dest='nightly', required=False, action='store_true', default=False,
	                    help='Adds date stamp to output file.')
	parser.add_argument('--IBK-path', dest='IBKPath', required=False, type=str, default='../../externals/IBK', 
	                    help='Path to IBK lib containing the IBK_BuildFlags.h file (in case IBK lib is not '
	                    'in ../../externals/IBK). Default: ../../externals/IBK')
	parser.add_argument('--skip-build', dest='skip_build', required=False, action='store_true', default=False,
	                    help='Development flag, disables build step.')
	parser.add_argument('--skip-deployment', dest='skip_deployment', required=False, action='store_true', default=False,
	                    help='Deployment flag, disables deployment step.')

	return parser.parse_args()


def printReleaseChecklist():
	fn = '../release_checklist.txt'
	if os.path.exists(fn):
		with open(fn) as f:
			for line in f:
				print line.rstrip("\r\n")	
	else:
		print "Release checklist file  '{}' file not found.".format(os.path.abspath(fn))
	print "Press any key to continue!"
	raw_input()





#
# *** Main Application ***
#

# Reading user arguments
args = configCommandLineArguments()

# in interactive mode, show release checklist
if not args.silent:
	printReleaseChecklist()

print "Creating release for product '{}'".format(args.product)

# create and initialize product config
productConf = ProductConfig()

try:
	# try reading products ini
	productConf.parseProductsIni(args.product, '../products.ini')

	# Uncomment deployment flag
	buildFlagsFile = args.IBKPath + '/src/IBK_BuildFlags.h'
	print "Adjusting deployment flag in file '{}'".format(buildFlagsFile)
	adjustDeploymentFlag( buildFlagsFile )

	# Build Application
	if not args.skip_build:
		if not args.silent:
			raw_input("Configuration done, press any key to start build!");
		buildApplication(args.build_script, args.x64)
	
	if not args.skip_deployment:
		# Extract Version and Long Version of current Application
		productConf.version, productConf.longVersion = extractVersionNumber( args.constantsPath )
		print 'Versions:\n  Version      = {}\n  Long Version = {}'.format(productConf.version, productConf.longVersion)

		# Adapting deploy scripts or iss installer scripts of current Application.
		print "Configuring deployment files"
		deployScript = configDeployment( productConf, args.nightly, args.x64 )
		# Script is done, calling shell/batch script shall now package the application.
		if not args.silent:
			raw_input("Build done, press any key to start packaging!");

		deploy(deployScript)


except Exception as e:
	print(e)
	print '*** Error creating release, aborting. ***'
	sys.exit(1)
	
sys.exit(0)
