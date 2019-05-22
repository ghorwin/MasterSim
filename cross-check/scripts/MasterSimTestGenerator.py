# -*- coding: utf-8 -*-

import os
import csv


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
			
		# all input data parsed and stored


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
hOutputMin           0.001 s
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
hOutputMin           0.001 s
binaryOutputFiles    no
adjustStepSize       yes
absTol               0
relTol               ${RelTol}
MasterMode           GAUSS_JACOBI
ErrorControlMode     NONE
maxIterations        1

${FMU-Definition}
"""
		msim_content = TEMPLATE_MSIM_FILE
		for kw in MasterSimTestGenerator.OPT_KEYWORDS:
			msim_content = msim_content.replace('${'+kw+'}', str(self.simOptions[kw]))

		# generate FMU definition line
		# 'simulator 0 0 Prey #ffff8c00 "fmus/IBK/Prey.fmu"'
		
		# relative path to FMU
		absPathToTarget = os.path.abspath(targetDir)
		relPathToFmu = os.path.relpath(self.fmuPath, absPathToTarget)
		
		simLine = 'simulator 0 0 slave1 #ffff8c00 "{}"'.format(relPathToFmu)
		msim_content = msim_content.replace('${FMU-Definition}', simLine)

		msim_fname = targetDir + "/" + os.path.split(self.fmuPath)[1]
		msim_fname = msim_fname[:-4] + ".msim"
		fobj = open(msim_fname, 'w')
		fobj.write(msim_content)
		fobj.close()
		del fobj
		