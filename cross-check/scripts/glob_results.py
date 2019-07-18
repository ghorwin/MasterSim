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
- read 'rejected', 'failed', 'passed' files if existing in working directory
- create README.md file in cross-check directory with documentation of the validation process
- if 'failed' exists, copy content of 'failed' file to README.md, create 'failed' file; done
- if 'rejected' exists, copy content of 'rejected' file to README.md, create 'rejected' file; done
- perform fairly strict value comparison for all quantities in 'referenceValues.csv'
  - for each variable compared, add a line with the WRMS norm in README.md

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
import numpy as np
import math

from datetime import datetime

import MasterSimTestGenerator as msimGenerator
from CrossCheckResult import CrossCheckResult

MASTERSIM_VERSION = "0.7.0" # written in success files


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


class CSVFile:
	def read(self,csvFile):
		"""Reads CSV file either in tab-separated or , mode."""
		self.captions = []
		self.content = []
		with open(csvFile, 'r') as f:
			contentLines = f.readlines()
			if len(contentLines) < 2:
				raise("Missing data")
			captionLine = contentLines[0]
			contentLines = contentLines[1:]

			# determine separation character
			tokens = captionLine.strip().split('\t')
			if len(tokens) > 1:
				# extract units from tokens
				self.captions = []
				for t in tokens:
					p1 = t.find('[')
					p2 = t.find(']')
					if p1 != -1 and p2 != -1 and p2 > p1:
						t = t[:p1-1]
					t = t.strip('\r\n')
					# special handling of MasterSim reference test cases, remove 'slave1.' from string
					if t.find('slave1.') == 0:
						t = t[7:]
					self.captions.append(t)
				self.content = [l.strip().split('\t') for l in contentLines]
			else:
				self.captions = captionLine.strip('\r\n').split(",")
				self.captions = [c.strip('" ') for c in self.captions] # remove quotes around variable names
				self.content = [l.strip().split(',') for l in contentLines]

			valueVectors = []
			for i in range(len(self.captions)):
				valueVectors.append([])
				for j in range(len(self.content)):
					valueVectors[i].append(float(self.content[j][i]))

			# generate np arrays
			self.time = np.array(valueVectors[0])
			self.values = []
			for i in range(1,len(valueVectors)):
				self.values.append( np.array(valueVectors[i]) )

		return

	def write(self,csvFile, filterCaptions, synVars):
		"""
		filterCaptions - list of captions to be written to file, first caption is always
		   'time' or 'Time' and is always written
	   synVars - SynonymousVars object, holding variable associations
		"""
		# write csv file in comma separated format
		captionIndexes = [0] # time column always
		# process all requested variables
		for i in range(1, len(filterCaptions)):
			c = filterCaptions[i]
			# check if variable is directly provided in output files
			if c in self.captions:
				colIndex = self.captions.index(c)
				# store index to the respective column
				captionIndexes.append(colIndex)
			else:
				# must be variable synonym
				synCaption = synVars.resolveVarName(c)
				assert(synCaption != None)
				colIndex = self.captions.index(synCaption)
				# store index to the respective column
				captionIndexes.append(colIndex)
			
		with open(csvFile, 'w') as f:
			quotedCaptions = []
			# loop over all captions
			for c in range(len(filterCaptions)):
				i = captionIndexes[c] # index of column to be written
				quotedCaptions.append( '"' + filterCaptions[c] + '"')
			f.write(",".join(quotedCaptions) + "\n")
			for l in self.content:
				# reduce the number precision of the output file
				timeStamp = l[0]
				tval = float(timeStamp)
				l[0] = "{:.5g}".format(tval)
				selectedValues = []
				for i in captionIndexes:
					selectedValues.append(l[i])
				f.write(",".join(selectedValues) + "\n")
		return


class SynonymousVars:
	def __init__(self):
		self.variableSynonyms = dict()
		
	def read(self, synFile):
		with open(synFile, 'r') as f:
			lines = f.readlines()
			for l in lines:
				l = l.strip('\r\n')
				if len(l) == 0:
					continue # skip empty lines
				varNames = l.split('\t')
				outputVarName = varNames[1]
				if not outputVarName in self.variableSynonyms:
					self.variableSynonyms[outputVarName] = []
				self.variableSynonyms[outputVarName].append(varNames[2])
				
	def resolveVarName(self, varName):
		"""Looks up output variable name for a variable synonyme.
		Returns None if no such variable is defined sy synonymous variable.
		"""
		for m in self.variableSynonyms:
			for v in self.variableSynonyms[m]:
				if v == varName:
					return m
		return None


def writeResultFile(fmuCaseDir, fileType, notes):
	"""Generates a result file.

	Parameters:

	* fmuCase - path to FMU case in github repo
	* fileType - passed, failed, rejected
	* notes - content of README.md
	"""

	if not os.path.exists(fmuCaseDir):
		os.makedirs(fmuCaseDir)

	with open(fmuCaseDir + "/" + fileType, 'w') as f:
		if fileType != "passed":
			f.write(notes)
		else:
			f.write("MasterSim_" + MASTERSIM_VERSION + "\n")

	if fileType == "passed":
		with open(fmuCaseDir + "/README.md", 'w') as f:
			f.write(notes + "\n")



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
			modelName = os.path.split(fmuCase)[1]

			# compose path for results (to be pushed to github)
			resultsRoot = fullPath + "/results/" + "/".join(pathParts[1:4])
			resultsRoot = resultsRoot + "/MasterSim/" + MASTERSIM_VERSION + "/" + pathParts[4] + "/" + pathParts[5] + "/" + modelName

			# generate path to target directory, hereby substitute / with _ so that we only have one directory level
			relPath = os.path.split(os.path.relpath(fmuCase, fullPath))[0] # get relative path to directory with fmu file
			relPath = relPath.replace('\\', '_') # windows fix
			relPath = relPath.replace('/', '_')
			relPath = MSIM_DIRECTORY + '/' + relPath


			# initialize database entry, even for other platforms
			cres = CrossCheckResult(datetime.now(), relPath[len(MSIM_DIRECTORY)+1:], pathParts[1], pathParts[3], pathParts[4]+"-"+pathParts[5], "failed", "Not calculated, yet")
			if not hash(cres) in results:
				results[hash(cres)] = cres
			else:
				cres = results[hash(cres)]

			# create directory if not existing
			# directory is most likely missing, because this test case belongs to a different platform
			# in this case we do not want to touch possibly existing reference results
			if not os.path.exists(relPath):
				#print("skipped : {} - Working directory missing".format(relPath))
				continue
			
			# check if a 'passed' file exists in the directory
			passedFile = relPath + "/passed"
			if os.path.exists(passedFile):
				# update success flag in results db
				cres.result = "passed"
				results[hash(cres)] = cres
				print("already_passed : {}".format(relPath))
				continue

			# check if a 'rejected' file exists in the directory
			rejectedFile = relPath + "/rejected"
			if os.path.exists(rejectedFile):
				# open README.md file and also create a rejected file in target directory
				with open(rejectedFile, 'r') as f:
					lines = f.readlines()
				writeResultFile(resultsRoot, 'rejected', "\n".join(lines))
				print("rejected : {}".format(relPath))
				# store info
				# modification date
				modDate = os.path.getmtime(rejectedFile)
				date = datetime.fromtimestamp(modDate)
				lines = [l.strip() for l in lines]
				cres = CrossCheckResult(date, relPath[len(MSIM_DIRECTORY)+1:], pathParts[1], pathParts[3], pathParts[4]+"-"+pathParts[5], "rejected", ",".join(lines))
				results[hash(cres)] = cres
				continue

			# check if a 'failed' file exists in the directory
			failedFile = relPath + "/failed"
			if os.path.exists(failedFile):
				# open README.md file and also create a rejected file in target directory
				with open(failedFile, 'r') as f:
					lines = f.readlines()
				writeResultFile(resultsRoot, 'failed', "\n".join(lines))
				print("failed : {}".format(relPath))
				# store info
				# modification date
				modDate = os.path.getmtime(failedFile)
				date = datetime.fromtimestamp(modDate)
				lines = [l.strip() for l in lines]
				cres = CrossCheckResult(date, relPath[len(MSIM_DIRECTORY)+1:], pathParts[1], pathParts[3], pathParts[4]+"-"+pathParts[5], "failed", ",".join(lines))
				results[hash(cres)] = cres
				continue

			# - check if a msim project file exists in this working directory
			msimFilename = relPath + "/" + modelName + ".msim"
			if not os.path.exists(msimFilename):
				print("skipped : {} - MasterSim file missing.".format(relPath))
				continue

			refValuesFile = root + "/" + modelName + "_ref.csv"
			if not os.path.exists(refValuesFile):
				print("skipped : {} - Reference values file {} missing.".format(relPath, refValuesFile))
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

			synonymousVarsFile = msimFilename[:-5] + "/results/synonymous_variables.txt"
			synVars = SynonymousVars()
			if os.path.exists(synonymousVarsFile):
				synVars.read(synonymousVarsFile)
			

			print ("Processing {}...".format(relPath))

			# now we read our valuesFile and create the {modelname}_out.csv
			resultCSV = CSVFile()
			resultCSV.read(valuesFile)

			referenceCSV = CSVFile()
			referenceCSV.read(refValuesFile)

			# compare by variable
			# - what if time points differ? linear interpolation? If so, use linear interpolation
			#   on MasterSim results
			failure = False
			variableComparisonResults = [] # vector of tuple for each variable: first = success flag, second = wrms norm
			for i in range(1, len(referenceCSV.captions)):
				refIndex = i-1
				refCaption = referenceCSV.captions[i] # 'captions' includes time column!

				# look up corresponding index of calculated values
				if not refCaption in resultCSV.captions:
					# check if this is a variably synonyme
					synOutputVar = synVars.resolveVarName(refCaption)
					if synOutputVar == None:
						print("    '{}' not computed.".format(refCaption))
						failure = True
						continue
					# use synoymous variable output instead
					resultIndex = resultCSV.captions.index(synOutputVar) # 'captions' includes time columns
				else:
					resultIndex = resultCSV.captions.index(refCaption) # 'captions' includes time columns

				resultIndex = resultIndex - 1 # mind 0 based indexing of values vector

				# now compare values, first interpolate calculated data at time points of reference values
				valuesInterpolated = np.interp(referenceCSV.time, resultCSV.time, resultCSV.values[resultIndex])

				# valuesInterpolated has now the same size as referenceCSV.time

				# compute vector norm between values
				refVals = referenceCSV.values[refIndex]
				diff = valuesInterpolated - refVals

				maxVal = refVals.max()
				minVal = refVals.min()
				absMax = max(abs(maxVal), abs(minVal))
				ABSTOL = absMax * 1e-3 + 1e-20
				RELTOL = 1e-3

				# normalize differences
				weights = 1.0/(RELTOL*abs(refVals) + ABSTOL)
				diff = diff*weights

				diff = diff**2 # square sum
				wrms = math.sqrt(diff.sum()/len(valuesInterpolated))
				print ("  WRMS({}) = {}".format(refCaption, wrms))

				if wrms < 1000:
					evalstr = "passed"
				else:
					evalstr = "failed"
				variableComparisonResults.append( (refCaption, evalstr, wrms) )

			# if marked as failure (missing variable), the test is failed
			if failure:
				print("failed : {} - missing result variables".format(relPath))
				cres.result = "failed"
				results[hash(cres)] = cres
				continue

			# now check if any of the results has a 'failure' marking
			anyVarFailed = False
			notes = []
			for r in variableComparisonResults:
				if r[1] == 'failed':
					anyVarFailed = True
				notes.append("WRMS({}) = {}".format(r[0], r[2]))

			cres.note = ",".join(notes) # must be single line, no line breaks

			if anyVarFailed:
				print("failed : {} - Results mismatch".format(relPath))
				cres.result = "failed"
				results[hash(cres)] = cres
				continue

			# update database
			cres.result = "passed"
			cres.note = ";".join(notes)
			results[hash(cres)] = cres

			# write success file and README.txt file
			mdFile = """# Validation of '{}'

## Variables
Weighted-root-mean-square norm with RelTol = 1e-3 and AbsTol = 1e-3, where
AbsTol is based on max. magnitude of reference values.

```
{}
```

## MasterSim project file

Below is the project file that was used to successfully simulation the test case.
Mind: project file is copied from working directory, hence relative path to fmu file.

```
{}
```
"""
			# read project file
			with open(msimFilename, 'r') as f:
				masterSimProject = f.readlines()
				masterSimProject = [l.strip('\r\n') for l in masterSimProject]

			mdFile = mdFile.format(modelName, "\n".join(notes), "\n".join(masterSimProject))
			writeResultFile(resultsRoot, "passed", mdFile)
			
			resultCSV.write(resultsRoot + "/" + modelName + "_out.csv", referenceCSV.captions, synVars)
			if 0:
				# write computed results as CSV file
				resCSV = CSVFile()
				# use the same time stamps as the reference values file
				# when transferring captions, only transfer those that exist in resultsCSV and referenceCSV
				
				resCSV.captions.append(referenceCSV.captions[0]) # time caption
				for i in range(1, len(referenceCSV.captions)):
					refIndex = i-1
					refCaption = referenceCSV.captions[i] # 'captions' includes time column!
		
					# look up corresponding index of calculated values, skip unknown variables
					if not refCaption in resultCSV.captions:
						continue # this might cause the validation to fail, not sure though
					resultIndex = resultCSV.captions.index(refCaption) # 'captions' includes time columns
		
					resultIndex = resultIndex - 1 # mind 0 based indexing of values vector
		
					# now compare values, first interpolate calculated data at time points of reference values
					valuesInterpolated = np.interp(referenceCSV.time, resultCSV.time, resultCSV.values[resultIndex])
					



# now write new database

with open(DATABASE_FILE,'w') as dbfile:
	dbfile.write('Date\tFMUCase\tCS-Version\tPlatform\tTool\tResult\tNotes\n')
	# need a sorted list of fmuCases, otherwise we cannot use svn efficiently to monitor changes 
	fmuCaseList = []
	for d in results:
		fmuCaseList.append(results[d].fmuCase)
	fmuCaseList.sort()
	
	for c in fmuCaseList:
		dbfile.write(results[hash(c)].write() + '\n')
print("New db written with {} entries".format(len(results)) )
