# -*- coding: utf-8 -*-
#
# Delphin6OutputFile.py
#
# Copyright (c) 2013-today Stefan Vogelsang and Andreas Nicolai, IBK, TU Dresden
#
# This file is part of MasterSim and free software. See LICENSE for details.

import os.path
import Delphin6GeoFileManager
import pandas as pd
import numpy as np
import datetime

class Delphin6OutputFile:
	"""Can read and interpret Delphin output files (ASCII Versions)."""
	def __init__(self):
		self.clear()

	def clear(self):
		"""Resets member variables to match an empty container."""
		self.headerData = dict()
		self.timePoints = []
		# holds time unit
		self.timeUnit = ""
		# holds all data as list of lists
		self.values = []
		# holds first column of data for convenient access to 2D data
		self.valueVector = []
		# for reference files, headers of individual columns
		self.quantities = []
		# coordinate vector, holds either x, y or z coordinates of the profile cut
		self.coordinates = []
		# step sizes/element thickness vector, holds either x, y or z widths of the elements in the profile cut
		self.steps = []
		# holds a string identifying the cut direction, either "X" or "Y" or "Z"
		self.direction = ""
		self.filename = ""
		# name for geofile
		self.geoName = ""

	def read(self, fname, geoFileManager=None):
		"""Reads DataIO container file.
		
		Function supports DataIO containers with major version 6 and 7.

		- fname -- Full path to DataIO file.
		- geoFileManager -- Instance to geometry file manager.

		Returns:
		   True if reading was successful, or False if reading failed due to an error. 
		   In case of errors, an error message is printed.
		"""
		self.clear()
		self.filename = fname
		# open output file and parse data
		try:
			outfile_obj = open(fname, 'r')
			lines = outfile_obj.readlines()
		except IOError:
			print "Can't read output or geo file."
			return False
		outfile_obj.close

		if (len(lines) == 0):
			print 'Empty output file.'
			return False

		# check header data
		self.d6oVersion = ""
		if lines[0].find("D6OARLZ! 006.") != -1:
			self.d6oVersion = "6"
		if lines[0].find("D6OARLZ! 007.") != -1:
			self.d6oVersion = "7"
		if self.d6oVersion == "":
			print 'Not a Delphin 6 Output file.'
			return False

		# start parsing data file header until indices vector found
		headerEnd = 0
		for i in range(len(lines)):
			line = lines[i].strip()
			# skip empty geoLines
			if len(line) == 0:
				continue
			# check for end-of-header
			tokens = line.split('=')
			keyword = tokens[0].strip(" #\t")

			# how do we find to parse actual data ?
			if len(tokens) == 1:
				# must be a comment line without keyword-value pair, just skip this
				continue
			if keyword.startswith("INDICES") :
				headerEnd = i
				break

			# parse header types
			value = tokens[1].strip()
			self.headerData[keyword] = value


		# extract number of indices
		if headerEnd == 0:
			print 'Missing data or incomplete header.'
			return False

		try:
			numberString = lines[headerEnd]
			numbers = numberString.split("=")
			numbers = numbers[1].split()
			numberCount = len(numbers)

			# check if we have a TYPE Keyword
			if not self.headerData.has_key("TYPE"):
				raise RuntimeError('Incomplete/invalid Delphin 6 Output file header, missing TYPE keyword.')
			if not self.headerData.has_key("SPACE_TYPE"):
				raise RuntimeError('Incomplete/invalid Delphin 6 Output file header, missing SPACE_TYPE keyword.')
			if not self.headerData.has_key("TIME_UNIT"):
				raise RuntimeError('Incomplete/invalid Delphin 6 Output file header, missing TIME_UNT keyword.')
			self.timeUnit = self.headerData["TIME_UNIT"]
			
			self.quantities = self.headerData["QUANTITY"].split('|')
			self.quantities = [q.split('.')[-1].strip() for q in self.quantities]

			# process selected elements and determine numbering direction and populate
			# the coordinates (cut) vector
			# we only do this for 3D output files
			if numberCount > 1:
				if self.headerData["SPACE_TYPE"] == "SINGLE":
					# check if we have a FLUX file, currently sides files with multiple values are not supported
					if self.headerData["TYPE"] == "FLUX":
						raise RuntimeError('Currently flux files with SPACE_TYPE = NONE are only supported for single sides.')

					# check if we have a TYPE Keyword FIELD file
					if self.headerData["TYPE"] == "FIELD":

						# we need a geometry file
						if not self.headerData.has_key("GEO_FILE"):
							raise RuntimeError('Missing GEO_FILE tag in header.')
						# get relative path to geo file
						self.geoName = self.headerData["GEO_FILE"]
						# create absolute path
						self.geoName = os.path.join( os.path.split(self.filename)[0], self.geoName)

						geoFile = geoFileManager.geoFile(self.geoName)
						if geoFile == None:
							raise RuntimeError('Error reading/accessing geometry file.')

						iIndexSet = set()
						jIndexSet = set()
						# collect all element indexes used in output data file
						for i in range(numberCount):
							elementIndex = int(numbers[i])
							i = geoFile.elements[elementIndex].i
							j = geoFile.elements[elementIndex].j
							iIndexSet.add(i)
							jIndexSet.add(j)
						# different code based on coordinate direction
						if len(jIndexSet) == 1:
							self.direction = "X"
							self.coordinates = geoFile.coordinatesInProfileCut(iIndexSet, 0)
							for i in iIndexSet:
								self.steps.append( geoFile.xSteps[ geoFile.elements[i].i ] )
						else:
							self.direction = "Y"
							self.coordinates = geoFile.coordinatesInProfileCut(iIndexSet, 1)
							for j in jIndexSet:
								self.steps.append( geoFile.ySteps[ geoFile.elements[j].j ] )

			headerEnd = headerEnd + 1

			self.timePoints = []
			self.values = []
			lines = lines[headerEnd:]
			for line in lines:
				# parse values
				vals = line.split()
				if len(vals) == 0:
					# skip empty lines
					pass
				else:
					self.timePoints.append(float(vals[0]))
					self.valueVector.append(float(vals[1]))
					values = []
					for i in range(1, len(vals)):
						values.append(float(vals[i]))
					self.values.append(values)

			return (len(self.values) > 0)

		# catch raised exceptions
		except RuntimeError as err:
			print err.message
			return False

	def valuesAt(self, timeCutValue):
		# check if timeCutValue is present in list of time points
		timeCutIndex = -1
		for i in range(len(self.timePoints)):
			if self.timePoints[i] == timeCutValue:
				timeCutIndex = i
				break
		if timeCutIndex == -1:
			raise RuntimeError("Time cut value {0} does not match any output time points.".format(timeCutValue) )
		valueVector = self.values[timeCutIndex]
		if len(valueVector) != len(self.coordinates):
			raise RuntimeError("{1} values found at time cut value {0}, but only {2} coordinates/elements "
			                   "specified in elements table.".format(timeCutValue, len(valueVector), len(self.coordinates) ) )
		return valueVector

	def valuesAtIndex(self, timeCutIndex):
		if timeCutIndex >= len(self.values):
			raise RuntimeError("Time cut value {} out of range (max index = {})."
			                   .format(timeCutIndex, len(self.value)) )
		valueVector = self.values[timeCutIndex]
		if len(valueVector) != len(self.coordinates):
			raise RuntimeError("{1} values found at time cut index {0}, but {2} coordinates/elements "
			                   "specified in elements table.".format(timeCutValue, len(valueVector), len(self.coordinates) ) )
		return valueVector

	def valueVectorAt(self, index):
		"""Extracts and returns a vector/column of data, most useful for reference data"""
		valueVec = []
		for i in range(len(self.values)):
			valueVec.append(self.values[i][index])
		return valueVec
	
	def asDataFrame(self):
		return pd.DataFrame(self.values, index=self.timePoints)

	def asDataFrameWithTimeIndex(self):
		unitConversionFactors = { 's' : 1, 'min' : 60, 'h' : 3600, 'd' : 24*3600, 'a' : 365*24*3600 }
		factor = unitConversionFactors[self.timeUnit]
		dateIndex = [datetime.datetime.fromtimestamp(i*factor) - datetime.datetime.fromtimestamp(0) + datetime.datetime(year=2000, month=1, day=1)  for i in self.timePoints ]
		return pd.DataFrame(self.values, index=dateIndex)

