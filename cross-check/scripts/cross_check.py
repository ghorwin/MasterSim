#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
Cross-check script, processes FMU import test cases.
Copyright 2019, Andreas Nicolai <andreas.nicolai@gmx.net>

> python cross_check.py [-d <directory>] [-t <tool>] [-v 1|2] [-p <platform>] <fmi-directory>

Arguments:

	-t <tool>       Which tool/vendor (basically the subdirectory name) to use; use all if omitted
    -v 1|2          Use either version 1 or 2 of the standard; use all if omitted
    -p <platform>   Platform (win32, win64, darwin64, linux32, linux64); use 64-bit version of current OS if omitted

    <fmi-directory> Root directory of fmi-cross-check repository.

Run this script from within a temporary directory where generated files can be placed.

Example:

> python cross_check.py -d FMI-Tests -t cs -v 1 -p linux64 ../fmi-cross-check

Will process all test cases within directory:

    ../fmi-cross-check/fmus/1.0/cs/linux64/Test-FMUs

"""


import numpy as np
import matplotlib.pyplot as pl
import os
import platform
import argparse
import csv

import CC_functions as cc
import MasterSimTestGenerator as msimGenerator


# *** main program ***


# command line arguments
parser = argparse.ArgumentParser(description="Runs cross-check tests for MasterSim")
parser.add_argument('-t', action="store", dest="tool", help="Tool/vendor directory to process")
parser.add_argument('-v', action="store", dest="fmiVersion", help="FMI version to use.")
parser.add_argument('-p', action="store", dest="platform", help="Platform (win32, win64, darwin32, darwin64, linux32, linux64).")
parser.add_argument(action="store", dest="fmiDirectory", help="Root directory with fmi-cross-check files.")

args = parser.parse_args()



# now process all subdirectories of root dir accordingly and filter out not selected directories
# create a list of all models to run
fullPath = os.path.abspath(args.fmiDirectory)
fullPathStr = fullPath.replace('\\' ,'/') # windows fix
fullPathParts = fullPathStr.split('/')

fmuList = []

print("Collecting list of FMUs to import and test-run")
for root, dirs, files in os.walk(fullPath, topdown=False):
	# split root directory into components

	root = os.path.join(fullPath, root)
	rootStr = root.replace('\\', '/') # windows fix
	pathParts = rootStr.split('/')
	pathParts = pathParts[len(fullPathParts):]

	# we only process directories that can actually contain models
	if len(pathParts) < 5:
		continue

	relPath = '/'.join(pathParts[1:])

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

	# filter out platform, if given
	osType = args.platform
	if osType == None:
		s = platform.system()
		if s == "Linux":
			osType = "linux64"
		elif s == "Windows":
			osType == "win64"
		else:
			osType == "darwin64"

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
			fmuList.append(fmuPath[:-4]) # strip the trailing .fmu
			print("  " + os.path.join(relPath, name))

print("{} FMUs to test".format(len(fmuList)))


# read list of test cases (may be a subset of total FMUs)
RESULT_CSV = "fmuTestResults.csv"
fmuEvalResult = dict() # key is fmuCase; value holds tuples: (runSuccessful, resultsCorrect)

if os.path.exists(RESULT_CSV):
	fobj = open(RESULT_CSV, 'r')
	lines = fobj.readlines()
	fobj.close()
	del fobj
	for i in range(1,len(lines)):
		tokens = lines[i].split('\t')
		if len(tokens) != 3:
			continue
		simOk = tokens[1].strip() == '1'
		resultsOk = tokens[2].strip() == '1'
		fmuEvalResult[tokens[0]] = (simOk, resultsOk)


# for each fmu, create an instance of our MasterSimImportTester class, parse the input/reference/options files 
# and then run the test
for fmuCase in fmuList:
	# generate path to MasterSim working directory = unique ID for test case
	relPath = os.path.split(os.path.relpath(fmuCase, fullPathStr))[0] # get relative path to directory with fmu file
	relPath = relPath.replace('\\', '_') # windows fix
	relPath = relPath.replace('/', '_')
	
	# check if case has already been completed successfully
	if relPath in fmuEvalResult:
		res = fmuEvalResult[relPath]
		if res[1] == True:
			print('Case {} already completed, skipped'.format(fmuCase))
			continue

	# setup test generator (parse input files)
	try:
		masterSim = msimGenerator.MasterSimTestGenerator()
		masterSim.setup(fmuCase)
	except Exception as e:
		print e
		fmuEvalResult[relPath] = (False, False)
		continue

	# generate MasterSim file
	masterSim.generateMSim(relPath)

	# run MasterSim, expects MasterSimulator executable to be in the path
	res = masterSim.run()
	if not res:
		fmuEvalResult[relPath] = (False, False)
		continue

	# read results
	res = masterSim.checkResults()
	if not res:
		fmuEvalResult[relPath] = (True, False)
		continue

	# mark fmuCase as completed
	fmuEvalResult[relPath] = (True, True)


with open(RESULT_CSV, 'w') as csvfile:
	csvfile.write("FMU\tRuns\tResults correct\n")
	for t in fmuEvalResult:
		res = fmuEvalResult[t]
		csvfile.write("{}\t{}\t{}\n".format(t, int(res[0]), int(res[1])) )


print("Result summary file {} written".format(RESULT_CSV))
