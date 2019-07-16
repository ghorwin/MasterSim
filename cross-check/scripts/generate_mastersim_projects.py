#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Cross-check helper script, generates MasterSim project files and imports required files.
Copyright 2019, Andreas Nicolai <andreas.nicolai@gmx.net>

Run this script from within a directory that contains the content of the 'fmi-cross-check' github repository.
Script will create a subdirectory 'msim' within it will create for each test fmu another subdirectory.

Use the command line arguments to filter out unneeded test cases.

If a subdirectory already exists, it will be skipped and a warning will be printed.

> python generate_mastersim_projects.py [-t <tool>] [-v 1|2] [-p <platform>] <fmi-directory>

Arguments:

	-t <tool>       Which tool/vendor (basically the subdirectory name) to use; use all if omitted
    -v 1|2          Use either version 1 or 2 of the standard; use all if omitted
    -p <platform>   Platform (win64, darwin64, linux64); use all if omitted

Example:

> generate_mastersim_projects.py -v 1 -p linux64

Will process all test cases within directory:

    fmi-cross-check/fmus/1.0/cs/linux64/*

"""

import os
import platform
import argparse
import csv

import MasterSimTestGenerator as msimGenerator

FMI_CROSS_CHECK_DIRECTORY = "fmi-cross-check"
MSIM_DIRECTORY = "msim"

# *** main program ***


# command line arguments
parser = argparse.ArgumentParser(description="Runs cross-check tests for MasterSim")
parser.add_argument('-t', action="store", dest="tool", help="Tool/vendor directory to process")
parser.add_argument('-v', action="store", dest="fmiVersion", help="FMI version to use.")
parser.add_argument('-p', action="store", dest="platform", help="Platform (win64, darwin64, linux64).")

args = parser.parse_args()

fullPath = os.path.abspath(FMI_CROSS_CHECK_DIRECTORY)
fullPath = fullPath.replace('\\', '/') # windows fix
fullPathParts = fullPath.split('/')

# check if directory exists
if not os.path.exists(fullPath):
	print("Directory '{}' does not exist.".format(fullPath))
	exit(1)

# check if target directory exists, otherwise attempt to create it
msimFullPath = os.path.abspath(MSIM_DIRECTORY)
if os.path.exists(msimFullPath):
	if os.path.isfile(msimFullPath):
		print("Target directory '{}' exists as file.".format(msimFullPath))
		exit(1)
else:
	os.mkdir(msimFullPath)
	if not os.path.exists(msimFullPath):
		print("Cannot create target directory '{}'.".format(msimFullPath))
		exit(1)


# now parse file list and generate project files

fmuList = []

print("Collecting list of FMUs to import and test-run")
# filter out platform, if given
osType = args.platform
if osType == None:
	s = platform.system()
	if s == "Linux":
		osType = "linux64"
	elif s == "Windows":
		print("Selecting 'win64' platform")
		osType = "win64"
	else:
		osType = "darwin64"

for root, dirs, files in os.walk(fullPath, topdown=False):

	root = os.path.join(fullPath, root) # full path to current fmu file
	rootStr = root.replace('\\', '/') # windows fix
	pathParts = rootStr.split('/') # split into component
	pathParts = pathParts[len(fullPathParts):] # keep only path parts below toplevel dir
	
	# we only process directories that can actually contain models, that means more than 4 path parts
	if len(pathParts) < 5:
		continue

	relPath = '/'.join(pathParts[1:]) # compose relative path

	# filter out everything except the fmus sub-directory
	if pathParts[0] != "fmus":
		continue

	# filter out Model Exchange fmus
	if pathParts[2] == "me":
		continue

	# filter out fmi version if given
	if args.fmiVersion != None:
		found = False
		if args.fmiVersion == "1" and pathParts[1] == "1.0":
			found = True
		if args.fmiVersion == "2" and pathParts[1] == "2.0":
			found = True
		if not found:
			continue

	if pathParts[3] != osType:
		continue

	# filter out vendor/tool, if given
	if args.tool != None and pathParts[4] != args.tool:
		continue
	
	# now find .fmu files
	for name in files:
		e = os.path.splitext(name)
		if len(e) == 2 and e[1] == '.fmu':
			fmuPath = os.path.join(root, name)

			fmuCase = fmuPath[:-4] # strip the trailing .fmu
			
			# generate path to target directory, hereby substitute / with _ so that we only have one directory level
			relPath = os.path.split(os.path.relpath(fmuCase, fullPath))[0] # get relative path to directory with fmu file
			relPath = relPath.replace('\\', '_') # windows fix
			relPath = relPath.replace('/', '_')
			relPath = MSIM_DIRECTORY + '/' + relPath
			
			# create directory if not existing
			if not os.path.exists(relPath):
				os.mkdir(relPath)
						
			# setup test generator (parse input files)
			try:
				masterSim = msimGenerator.MasterSimTestGenerator()
				masterSim.setup(fmuCase)
			except Exception as e:
				print(e)
				# create a 'fail' file with error message
				with open(relPath + "/rejected", 'w') as file: 
					file.write(str(e) + "\n")
				continue

			# generate MasterSim file
			if not masterSim.generateMSim(relPath, pathParts[1]=="1.0"):
				continue # MasterSim project file exists already, skip FMU
			
			relPathFMUStr = os.path.join(relPath, name)
			print(relPathFMUStr + " processed...")

print("MasterSim project files regenerated.")
