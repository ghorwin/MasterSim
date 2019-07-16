#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Cross-check helper script, processes simulation results and generates/updates the files needed for publishing the cross-check results.

Steps:

- process entire directory structure in fmi-cross-check
- for each directory (cs only), generate local path as does the 'generate_mastersim_projects.py' script

For example: 
  from   ./fmi-cross-check/fmus/1.0/cs/linux64/JModelica.org/1.15/ControlledTemperature
  to     ./msim/fmus_1.0_cs_linux64_JModelica.org_1.15_ControlledTemperature
  (working directory)
  
- check if a msim project file exists in this working directory
- check if a result directory exists
- check if a values.csv file exists
- read 'values.csv' file
- read 'referenceValues.csv' file
- read 'rejected', 'fail', 'success' files if existing in working directory
- create README.md file in cross-check directory with documentation of the validation process
- if 'fail' exists, copy content of 'fail' file to README.md, create 'failed' file; done
- if 'rejected' exists, copy content of 'rejected' file to README.md, create 'rejected' file; done
- perform fairly strict value comparison for all quantities in 'referenceValues.csv'
  - for each variable compared, add a line with the absolute and relative norm in README.md

- populate summary db table with following data for each case:
  - case path
  - cs version
  - tool
  - platform
  - state (failed, rejected, success)
  - comment
  - details (content of readme file)
  
  summary db file should be read/appended (to support cross-platform testing)
"""


import os
import platform
import argparse
import csv
import subprocess, os   # for subprocess and os.path
import platform         # for checking for Windows OS

from datetime import datetime

import MasterSimTestGenerator as msimGenerator
from CrossCheckResult import CrossCheckResult

def runMasterSim(projectFile, workingDirectory):
	"""Creates a new process to run MasterSim command line solver.
	Returns when the simulation is complete.

	**Return codes**
	When a return code less than 0 is returned, an error message is printed onto the console.

	When the working directory is invalid, return code is -2 returned.
	When the processes could not be spawned, possibly because of an invalid executable path, the
	code is -1 returned.
	"""

	if not workingDirectory == None:
		if not os.path.exists(workingDirectory):
			print("Invalid working directory '{}'".format(workingDirectory))
			return -2
	try:
		if platform.system() == "Windows":
			MASTERSIM_SOLVER = "../bin/release_x64/MasterSimulator"
		else:
			MASTERSIM_SOLVER = "../bin/release/MasterSimulator"
		cmdline = [MASTERSIM_SOLVER, projectFile, '--verbosity-level=0']
		if platform.system() == "Windows":
			cmdline.append['-x']
			solver_process = subprocess.Popen(cmdline, creationflags=subprocess.CREATE_NEW_CONSOLE,
		                                      cwd=workingDirectory)
		else:
			solver_process = subprocess.Popen(cmdline, cwd=workingDirectory)
		# run simulation, a successful simulation should return 0
		return solver_process.wait()
	except Exception as ex:
		print ("Error running MasterSim executable '{}', {}".format(MASTERSIM_SOLVER, ex))
		return -1


def writeResultFile(fmuCaseDir, fileType, notes):
	pass


# ---- main ----------------------------------------------------------------------------------------------------

FMI_CROSS_CHECK_DIRECTORY = "fmi-cross-check"
MSIM_DIRECTORY = "msim"
DATABASE_FILE = "results/cross-check-results.csv"

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
		print("Working directory '{}' exists as file.".format(msimFullPath))
		exit(1)
else:
	print("Missing working directory '{}'.".format(msimFullPath))
	exit(1)


print("Parsing fmi-cross-check directory structure")

results = dict()
# read existing database file 
if os.path.exists(DATABASE_FILE):
	with open(DATABASE_FILE,'r') as dbfile:
		lines = dbfile.readlines()

		# remove header line
		if len(lines) > 0 and lines[0].find("FMUCase") != -1:
			lines = lines[1:]

		for l in lines:
			cres = CrossCheckResult()
			cres.read(l)
			results[hash(cres)] = cres
	print("Existing db read with {} entries".format(len(results)) )


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
			# directory is most likely missing, because this test case belongs to a different platform
			# in this case we do not want to touch possibly existing reference results
			if not os.path.exists(relPath):
				print("skipped : {} - Working directory missing".format(relPath))
				continue
			
			
			# check if a 'rejected' file exists in the directory
			rejectedFile = relPath + "/rejected"
			if os.path.exists(rejectedFile):
				# open README.md file and also create a rejected file in target directory
				with open(rejectedFile, 'r') as f:
					lines = f.readlines()
				writeResultFile(root, 'rejected', lines)
				print("rejected : {}".format(relPath))
				# store info
				# modification date
				modDate = os.path.getmtime(rejectedFile)
				date = datetime.fromtimestamp(modDate)
				lines = [l.strip() for l in lines]
				cres = CrossCheckResult(date, relPath[len(MSIM_DIRECTORY)+1:], pathParts[3], pathParts[1], pathParts[4]+"-"+pathParts[5], "rejected", ",".join(lines)) 
				results[hash(cres)] = cres
				continue
			
			# check if a 'failed' file exists in the directory
			failedFile = relPath + "/failed"
			if os.path.exists(failedFile):
				# open README.md file and also create a rejected file in target directory
				with open(failedFile, 'r') as f:
					lines = f.readlines()
				writeResultFile(root, 'failed', lines)
				print("failed : {}".format(relPath))
				# store info
				# modification date
				modDate = os.path.getmtime(failedFile)
				date = datetime.fromtimestamp(modDate)
				lines = [l.strip() for l in lines]
				cres = CrossCheckResult(date, relPath[len(MSIM_DIRECTORY)+1:], pathParts[3], pathParts[1], pathParts[4]+"-"+pathParts[5], "failed", ",".join(lines)) 
				results[hash(cres)] = cres
				continue
			
			# - check if a msim project file exists in this working directory
			msimFilename = relPath + "/" + os.path.split(fmuCase)[1] + ".msim"
			if not os.path.exists(msimFilename):
				print("skipped : {} - MasterSim file missing.".format(relPath))
				continue
			
			valuesFile = msimFilename[:-5] + "/results/values.csv"
			if not os.path.exists(valuesFile):
				print("running : {}...".format(relPath))
				# attempt to run simulator
				res = runMasterSim(msimFilename, None)
				if res != 0 or not os.path.exists(valuesFile):
					# error running?
					print("failed : {} - Error running MasterSim".format(relPath))
					continue

				
			# now we read our valuesFile and create the {modelname}_out.csv
			
			
			# extend our database with tests
			#cres = CrossCheckResult(fmuCase)
			#results[hash(cres)] = cres


# now write new database

with open(DATABASE_FILE,'w') as dbfile:
	dbfile.write('Date\tFMUCase\tCS-Version\tPlatform\tTool\tResult\tNotes\n')
	for d in results:
		dbfile.write(results[d].write() + '\n')
print("New db written with {} entries".format(len(results)) )
