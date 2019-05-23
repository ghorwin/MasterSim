# -*- coding: utf-8 -*-

import os
import csv
import subprocess
import pandas as pd

import Delphin6OutputFile

# Implementation of class MasterSimTestGenerator 

class MasterSimTestGenerator:

	def __init__(self):
		self.fmuPath = ""
		self.simOptions = dict()
		MasterSimTestGenerator.OPT_KEYWORDS = [ 'StartTime', 'StopTime', 'StepSize', 'RelTol' ]
	
	def setup(self, fmuCaseBaseName):
		"""Reads auxiliary files for the given fmu test case.
		
		`Arguments`
		
		fmuCaseBaseName  
		  Full path to the basename of the fmu, for example ``/path/to/BouncingBall``
		
		Adding the extension ``.fmu`` yields the fmu path.
		
		The following files are read:
		
		- suffix ``_ref.csv``: file with reference results
		- suffix ``_ref.opt``: file with simulation parameters
		- suffix ``_in.csv``: file with provided input variables
		
		"""
		
		# check, if fmu file exists
		self.fmuPath = fmuCaseBaseName + ".fmu"
		if not os.path.exists(self.fmuPath):
			raise Exception("fmu-file '{}' not found".format(self.fmuPath))
		
		# read opt file
		optFile = fmuCaseBaseName + "_ref.opt"
		if not os.path.exists(optFile):
			raise Exception("opt-file '{}' expected".format(optFile))
		fobj = open(optFile, 'r')
		lines = fobj.readlines()
		fobj.close()
		del fobj
		for line in lines:
			# split line at ,
			tokens = line.split(',')
			if len(tokens) != 2:
				continue
			kw = tokens[0].strip()
			self.simOptions[kw] = float(tokens[1])

		for kw in MasterSimTestGenerator.OPT_KEYWORDS:
			if not kw in self.simOptions:
				raise Exception("Missing '{}' in opt file.".format(kw))
			
		# additional sanity checks
		if self.simOptions['StartTime'] >= self.simOptions['StopTime']:
			raise Exception("Invalid 'StartTime' or 'StopTime' parameter.")
		
		if self.simOptions['StepSize'] < 0:
			raise Exception("Invalid 'StepSize' parameter.")
		
		# check if 'in' file exists
		inFile = fmuCaseBaseName + "_in.csv"
		if os.path.exists(inFile):
			inReader = csv.reader(inFile, delimiter=',', quotechar='"')
			# extract the different column headers, which will become the input variables
			# provided to the fmu
			self.inputVars = next(inReader, None)
			
		# all input data parsed and stored, now also read results

		refFile = fmuCaseBaseName + "_ref.csv"
		if not os.path.exists(refFile):
			raise Exception("Reference result file '{}' expected".format(refFile))
		self.refData = pd.read_csv(refFile, delimiter=',', quotechar='"')
		timeColumnName = 'time'
		if not timeColumnName in self.refData:
			timeColumnName = 'Time'
			if not timeColumnName in self.refData:
				raise Exception("Missing 'time' o 'Time' column in reference result file '{}'.".format(refFile))
		
		# determine output frequency
		dtOut = None
		self.tp = self.refData[timeColumnName]
		for i in range(1,len(self.tp)):
			dt = self.tp[i] - self.tp[i-1]
			if dtOut == None:
				dtOut = dt
			else:
				if abs(dt - dtOut) > 1e-8:
					dtOut = min(dtOut, dt)
	
		self.dtOut = dtOut



	def generateMSim(self, targetDir):
		"""Generates the MasterSim project file to run the test case.
		This function creates a directory structure based on the fmu-name below the targetDir, whereby
		targetDir is a relative path to the current working directory. The entire directory structure
		is created, if not existing already.
		
		`Arguments:`
		
		- targetDir (relative) path to directory where MasterSim project file shall be created and executed
		"""
		
		if os.path.isfile(targetDir):
			raise Exception("Target directory '{}' exists already as file!".format(targetDir))
		
		if not os.path.exists(targetDir):
			os.makedirs(targetDir)
		
		TEMPLATE_MSIM_CS1_FILE = """
tStart               ${StartTime} s
tEnd                 ${StopTime} s
hMax                 30 min
hMin                 1e-6 s
hFallBackLimit       0.001 s
hStart               ${StepSize} s
hOutputMin           ${dtOutMin} s
binaryOutputFiles    no
adjustStepSize       no
absTol               1e-06
relTol               ${RelTol}
MasterMode           GAUSS_JACOBI
ErrorControlMode     NONE
maxIterations        1

${FMU-Definition}
"""
		
		TEMPLATE_MSIM_CS2_FILE = """
tStart               ${StartTime} s
tEnd                 ${StopTime} s
hMax                 30 min
hMin                 1e-6 s
hFallBackLimit       ${FallBackLimit} s
hStart               ${StepSize} s
hOutputMin           ${dtOutMin} s
binaryOutputFiles    no
adjustStepSize       yes
absTol               0
relTol               ${RelTol}
MasterMode           GAUSS_JACOBI
ErrorControlMode     NONE
maxIterations        1

${FMU-Definition}
"""
		msim_content = TEMPLATE_MSIM_CS1_FILE
		for kw in MasterSimTestGenerator.OPT_KEYWORDS:
			msim_content = msim_content.replace('${'+kw+'}', str(self.simOptions[kw]))

		msim_content = msim_content.replace('${dtOutMin}', str(self.dtOut))

		# generate FMU definition line
		# 'simulator 0 0 Prey #ffff8c00 "fmus/IBK/Prey.fmu"'
		
		# relative path to FMU
		absPathToTarget = os.path.abspath(targetDir)
		relPathToFmu = os.path.relpath(self.fmuPath, absPathToTarget)
		
		simLine = 'simulator 0 0 slave1 #ffff8c00 "{}"'.format(relPathToFmu)
		msim_content = msim_content.replace('${FMU-Definition}', simLine)

		self.msimFilename = targetDir + "/" + os.path.split(self.fmuPath)[1]
		self.msimFilename = self.msimFilename[:-4] + ".msim"
		fobj = open(self.msimFilename, 'w')
		fobj.write(msim_content)
		fobj.close()
		del fobj
		
	def run(self):
		"""Runs the MasterSimulation executable"""
		
		command = ['MasterSimulator', '--verbosity-level=4', '-x', self.msimFilename]
		print("Running 'MasterSimulator' for FMU '{}' ...".format(self.fmuPath))
		try:
			retcode = subprocess.call(command)
			if retcode != 0:
				print("Error during simulation, see logfile for details.")
				return False
			return True
		except OSError as e:
			print(e)
			print("Error running 'MasterSimulator', make sure it is in your PATH!")
			return False
		
		
	def checkResults(self):
		"""Reads computed results and compares them to provided reference results."""
		
		doubleOutputs = Delphin6OutputFile.Delphin6OutputFile()
		outFile = self.msimFilename[:-5] + "/results/real_---.d6o"
		if not doubleOutputs.read(outFile):
			print("Error reading results file '{}'".format(outFile))
			return False
		
		# process all variables in the reference result file
		
		tpOutputs = doubleOutputs.timePoints
		# now process all variables (except time) in reference data
		for var in self.refData:
			if var == 'time' or var == 'Time':
				continue
			# lookup corresponding column in output file
			try:
				colIdx = doubleOutputs.quantities.index(var)
			except:
				print("Quantity '{}' not generated as output. Skipped in test.".format(var))
				continue
			# get time point and value vector from 
			values = doubleOutputs.valueVectorAt(colIdx)
			# now 
			#for t in self.tp:
		
		return True