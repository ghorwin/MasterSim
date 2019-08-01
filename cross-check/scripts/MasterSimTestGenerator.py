# -*- coding: utf-8 -*-

import os
import csv
import subprocess
import pandas as pd
from shutil import copyfile

# Implementation of class MasterSimTestGenerator 

class MasterSimTestGenerator:

	def __init__(self):
		self.fmuPath = ""
		self.simOptions = dict()
		self.variableInputFile = ""
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
			try:
				inReader = open(inFile, "r")
				# extract the different column headers, which will become the input variables
				# provided to the fmu
				line = inReader.readline()
				self.inputVars = line.split(',')
				line = [l.strip('"\r\n') for l in self.inputVars]
				self.inputVars = line[1:] # remove first column (time column)
				
				self.variableInputFile = inFile
			except:
				raise("Error reading input file '{}'".format(inFile))
			
		# all input data parsed and stored, now also read results

		refFile = fmuCaseBaseName + "_ref.csv"
		if not os.path.exists(refFile):
			raise Exception("Reference result file '{}' expected".format(refFile))
		self.refDataFile = refFile
		self.refData = pd.read_csv(refFile, delimiter=',', quotechar='"')
		timeColumnName = None
		for colName in self.refData:
			colNameStripped = colName.strip(' "')
			#print(colNameStripped)
			if colNameStripped == 'time' or colNameStripped == 'Time':
				timeColumnName = colName
		if timeColumnName == None:
			raise Exception("Missing 'time' or 'Time' column in reference result file '{}'.".format(refFile))
		
		# determine output frequency
		
		#dtOut = None
		#self.tp = self.refData[timeColumnName]
		#for i in range(1,len(self.tp)):
			#dt = self.tp[i] - self.tp[i-1]
			## handle case were missing number precision in ref file gives consecutive time points with same value
			#if dt == 0:
				#continue 
			#if dtOut == None:
				#dtOut = dt
			#else:
				#if abs(dt - dtOut) > 1e-8:
					#dtOut = min(dtOut, dt)

		simTime = self.simOptions['StopTime'] - self.simOptions['StartTime']
		self.dtOut = simTime / 100.0 # to get resonably fine output spacing



	def generateMSim(self, targetDir, cs1):
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

		# copy xxx_in.csv file to target directory
		if len(self.variableInputFile) > 0:
			targetInputFile = targetDir + "/" + os.path.basename(self.variableInputFile)
			copyfile(self.variableInputFile, targetInputFile)
			# file is now relative to project file and we only need to use the filename
			self.variableInputFile = os.path.basename(self.variableInputFile)


		self.msimFilename = targetDir + "/" + os.path.split(self.fmuPath)[1]
		self.msimFilename = self.msimFilename[:-4] + ".msim"

		# do not overwrite existing MasterSim file, in case it was manually modified
		if os.path.exists(self.msimFilename):
			print ("MASTERSIM file '{}' exists...".format(self.msimFilename))
			return False
		
		TEMPLATE_MSIM_CS1_FILE = """
tStart                   ${StartTime} s
tEnd                     ${StopTime} s
hMax                     30 min
hMin                     1e-6 s
hFallBackLimit           0.001 s
hStart                   ${StepSize} s
hOutputMin               ${dtOutMin} s
adjustStepSize           no
absTol                   1e-06
relTol                   ${RelTol}
MasterMode               GAUSS_JACOBI
ErrorControlMode         NONE
maxIterations            1
writeInternalVariables   yes

${FMU-Definition}

${Graph}
"""
		
		TEMPLATE_MSIM_CS2_FILE = """
tStart                   ${StartTime} s
tEnd                     ${StopTime} s
hMax                     30 min
hMin                     1e-6 s
hFallBackLimit           0.001 s
hStart                   ${StepSize} s
hOutputMin               ${dtOutMin} s
adjustStepSize           no
absTol                   1e-6
relTol                   ${RelTol}
MasterMode               GAUSS_JACOBI
ErrorControlMode         NONE
maxIterations            1
writeInternalVariables   yes

${FMU-Definition}

${Graph}
"""
		if cs1:
			msim_content = TEMPLATE_MSIM_CS1_FILE
		else:
			msim_content = TEMPLATE_MSIM_CS2_FILE
		for kw in MasterSimTestGenerator.OPT_KEYWORDS:
			msim_content = msim_content.replace('${'+kw+'}', "{:f}".format(self.simOptions[kw]))

		msim_content = msim_content.replace('${dtOutMin}', "{:f}".format(self.dtOut))

		# generate FMU definition line
		# 'simulator 0 0 Prey #ffff8c00 "fmus/IBK/Prey.fmu"'
		
		# relative path to FMU
		absPathToTarget = os.path.abspath(targetDir)
		relPathToFmu = os.path.relpath(self.fmuPath, absPathToTarget)
		
		simLine = 'simulator 0 0 slave1 #ffff8c00 "{}"'.format(relPathToFmu)
		
		graph = ""
		if len(self.variableInputFile) > 0:
			simLine = simLine + '\nsimulator 0 0 in_csv #ff0000ff "{}"'.format(self.variableInputFile)
			# self.inputVars -> parameters
			for iv in self.inputVars:
				graph = graph + "graph in_csv.{} slave1.{}\n".format(iv, iv) 
		msim_content = msim_content.replace('${Graph}', graph)
		
		msim_content = msim_content.replace('${FMU-Definition}', simLine)

		fobj = open(self.msimFilename, 'w')
		fobj.write(msim_content)
		fobj.close()
		del fobj

		if 0:
			# generate file with expected results in PostProc 2 format
			referenceValueFilename = targetDir + "/referenceValues.csv"
			refValueFile = open(referenceValueFilename, 'w')
			header = ["Time [s]"]
			
			for var in self.refData:
				if var == 'time' or var == 'Time':
					continue
				header.append(var)
			refValueFile.write('\t'.join(header) + '\n')
			# now all columns 
			
			for i in range(len(self.tp)):
				ref_t = self.tp[i]
				row = [str(ref_t)]
				for var in self.refData:
					if var == 'time' or var == 'Time':
						continue
					refVals = self.refData[var]
					ref_val = refVals[i]
					row.append(str(ref_val))
				refValueFile.write('\t'.join(row) + '\n')
			refValueFile.close()
			del refValueFile		
		else:
			# copy reference values file to target directory
			refFilename = os.path.split(self.refDataFile)[1]
			copyfile(self.refDataFile, targetDir + "/" + refFilename)
		
		return True # all ok, ready for simulation

		
	def run(self):
		"""Runs the MasterSimulation executable"""
		
		command = ['MasterSimulator', '--verbosity-level=1', '-x', self.msimFilename]
		print("Running 'MasterSimulator' for FMU '{}' ...".format(self.fmuPath))
		try:
			solverProcess = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
			retcode = solverProcess.wait()
			(outlog, errlog) = solverProcess.communicate()
			outlog = outlog.replace("[01;31m","")
			outlog = outlog.replace("[22;37m","")
			outlog = outlog.replace("[01;33m","")
			
			errlog = errlog.replace("[01;31m","")
			errlog = errlog.replace("[22;37m","")
			errlog = errlog.replace("[01;33m","")
			
			# dump output to logfile
			logf = open(self.msimFilename + ".log", 'w')
			logf.write(outlog)
			logf.close()
			del logf

			logf = open(self.msimFilename + ".errors", 'w')
			logf.write(errlog)
			logf.close()
			del logf
			
			if retcode != 0:
				print("Error during simulation, see '{}.logfile' for details.".format(os.path.basename(self.msimFilename)) )
				return False
			return True
		except OSError as e:
			print(e)
			print("Error running 'MasterSimulator', make sure it is in your PATH!")
			return False
		
	
	
	def checkResults(self):
		"""Reads computed results and compares them to provided reference results."""
		
		outFile = self.msimFilename[:-5] + "/results/values.csv"
		
		if not os.path.exists(outFile):
			print("Result data file '{}' missing!".format(outFile))
			return False
		

	
			
		msimResults = open(outFile, 'r')
		lines = msimResults.readlines()
		if len(lines) < 2:
			print("Missing data in result data file '{}'!".format(outFile))
			return False
		header = lines[0].strip().split('\t')
		captions = []
		data = []
		for h in header:
			# remove units []
			p1 = h.rfind('[')
			h = h[:p1-1].strip()
			# remove first part with slave name
			p1 = h.find('.')
			h = h[p1+1:]
			captions.append(h)
			data.append( [] )
		
		for i in range(1, len(lines)):
			line = lines[i]
			tokens = line.split('\t')
			for colIdx in range(len(tokens)):
				try:
					d = float(tokens[colIdx])
					data[colIdx].append(d)
				except:
					data[colIdx].append(-99999) # indicates missing/invalid value
			
		
		# process all variables in the reference result file
		
		tpOutputs = data[0]
		# now process all variables (except time) in reference data
		self.valueNorms = ""
		success = True
		for var in self.refData:
			if var == 'time' or var == 'Time':
				continue
			# lookup corresponding column in output file
			try:
				colIdx = captions.index(var)
			except:
				print("Quantity '{}' not generated as output. Marking test as failed.".format(var))
				return False
			
			# get time point and value vector from 
			values = data[colIdx]
			
			norm = 0
			refVals = self.refData[var]
			relTol = self.simOptions['RelTol']
			# now process all time points in the reference files
			for i in range(len(self.tp)):
				ref_t = self.tp[i]
				ref_val = refVals[i]
				valueAtT = findValue(ref_t, tpOutputs, values) # find the value in the list of values
				absValue = max(abs(ref_val), abs(valueAtT))
				diff = valueAtT - ref_val
				relDiff = diff/(absValue*relTol + 1e-6)
				norm = norm + relDiff**2
				#norm = norm + (valueAtT-ref_val)**2
			norm = norm/len(self.tp)
			
			print("|delta({})| = {}".format(var,norm))
			self.valueNorms = self.valueNorms + "|delta({})| = {}".format(var,norm)
			
			if norm > 1:
				success = False
		
		return success
	

def findValue(t, tp, vals):
	"""Returns first value from vector 'vals' whose time point is >= the given time point 't'."""

	for i in range(len(tp)):
		ti = tp[i]
		if t>= ti:
			return vals[i] 
