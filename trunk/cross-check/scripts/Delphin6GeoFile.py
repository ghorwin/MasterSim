# -*- coding: utf-8 -*-
#
# Delphin6GeoFile.py
#
# Copyright (c) 2013-today Andreas Nicolai, IBK, TU Dresden
#
# This file is part of MasterSim and free software. See LICENSE for details.

import os.path
import numpy as np

class Element:
	"""A class to store element index information. """
	def __init__(self):
		self.i = -1
		self.j = -1
		self.k = -1
		self.x = 0
		self.y = 0
		self.z = 0

	def __init__(self, i, j, k, x, y, z):
		self.i = i
		self.j = j
		self.k = k
		self.x = x
		self.y = y
		self.z = z

class Delphin6GeoFile:
	"""
	Can read and interpret Delphin 6 geometry files (ASCII Versions).
	"""

	def __init__(self):
		self.clear()

	def clear(self):
		self.headerData = dict()
		# step sizes (element thickness) in x direction, size = x_dimension
		self.xSteps = []
		# step sizes (element thickness) in y direction, size = y_dimension
		self.ySteps = []
		# step sizes (element thickness) in z direction, size = z_dimension
		self.zSteps = []
		# all elements (i,j,k indexes and x,y,z coordinates), size = nElements
		self.elements = []
		# filename of Geofile
		self.filename = ""

	def readMaterialsTable(self, lines, lastLine):
		for i in range(lastLine+1, len(lines)):
			line = lines[i].strip()
			if len(line) == 0:
				continue
			if line.startswith("TABLE") or line.startswith("INDICES") :
				return i
			# parse materials line... currently ignored
		raise RuntimeError("Incomplete Delphin 6 output file, missing data after MATERIALS table.")

	def readGridTable(self, lines, lastLine):
		xSteps = lines[lastLine+1].split()
		self.xSteps = np.array(map(float, xSteps))
		ySteps = lines[lastLine+2].split()
		self.ySteps = np.array(map(float, ySteps))
		zSteps = lines[lastLine+3].split()
		self.zSteps = np.array(map(float, zSteps))
		# we decide on x or y direction based on number of x values
		if len(self.zSteps) != 1:
			raise RuntimeError("Invalid Delphin 6 output file, must have only one z-coordinate!")

		for i in range(lastLine+4, len(lines)):
			line = lines[i].strip()
			if len(line) == 0:
				continue
			if line.startswith("TABLE") or line.startswith("INDICES") :
				return i
		raise RuntimeError("Incomplete Delphin 6 output file, missing data after GRID table.")

	def readElementsTable(self, lines, lastLine):
		for i in range(lastLine+1, len(lines)):
			line = lines[i].strip()
			if len(line) == 0:
				continue
			if line.startswith("TABLE") or line.startswith("INDICES") :
				return i
			# parse elements line... extract coordinates
			elementData = line.split()
			if len(elementData) == 6:
				self.elements.append( Element( int(elementData[3]), int(elementData[4]), -1, float(elementData[1]), float(elementData[2]), 0) )
			else:
				self.elements.append( Element( int(elementData[4]), int(elementData[5]), int(elementData[6]), float(elementData[1]), float(elementData[2]), float(elementData[3])) )
		raise RuntimeError("Incomplete Delphin 6 output file, missing data after ELEMENTS table.")

	def readSidesTable(self, lines, lastLine):
		for i in range(lastLine+1, len(lines)):
			line = lines[i].strip()
			if len(line) == 0:
				continue
			# parse sides line... currently ignored
			# sidesData = line.split()
		# no data after sides table, so we read until the end
		return len(lines)

	def read(self, geoName):
		"""Reads a geometry file.

		*geoName* -- Geometry file name.

		Returns:
		   True on success, False on error (error messages are written).
		"""

		try:
			geofile_obj = open(geoName, 'r')
			#print "Geometry file: '{}'".format(os.path.split(geoName)[1])
			geoLines = geofile_obj.readlines()
		except IOError:
			print "Can't read/open geometry file."
			return False
		geofile_obj.close
		if (len(geoLines) == 0):
			print 'Empty geometry file.'
			return False

		# check header data
		self.d6gVersion = ""
		if geoLines[0].find('D6GARLZ! 006.') == -1:
			self.d6gVersion = "6"
		if geoLines[0].find('D6GARLZ! 007.') == -1:
			self.d6gVersion = "7"
		if self.d6gVersion == "":
			print 'Not a Delphin 6 geometry file (neither version 6 nor 7).'
			return False

		# process all lines, fork off read functions when a TABLE is found
		maxI = len(geoLines)
		i = 0
		while i < maxI:
			line = geoLines[i].strip()
			# skip empty geoLines
			if len(line) == 0:
				i = i + 1
				continue

			if line.startswith("TABLE"):
				# parse all tables
				tableNameTokens = line.split()
				if len(tableNameTokens) < 2:
					raise RuntimeError('Incomplete/invalid Delphin 6 Output file, error in table definition, line:\n{}'.format(keyword))
				tableName = tableNameTokens[1].strip()
				if tableName == "MATERIALS":
					i = self.readMaterialsTable(geoLines, i)
				elif tableName == "GRID":
					i = self.readGridTable(geoLines, i)
				elif tableName == "ELEMENT_GEOMETRY":
					i = self.readElementsTable(geoLines, i)
				elif tableName == "SIDES_GEOMETRY":
					i = self.readSidesTable(geoLines, i)
			else:
				i = i + 1

		# Done reading

		# TODO : add check for completeness/consistency of data
		return True

	def coordinatesInProfileCut(self, elementIndexes, direction):
		"""This function obtains an array of continuous x, y or z-coordinates based on the element indexes
		in the passed array.

		This function expects the selected elements to be part of a profile cut in either x, y, or z direction.

		Arguments:
		   elementIndexes Array of element indexes.
		   direction Either 0 for x direction, 1 for y direction or 2 for z direction.
		"""
		coordinates = []
		for i in elementIndexes:
			if direction == 0:
				coordinates.append(self.elements[i].x)
			elif direction == 1:
				coordinates.append(self.elements[i].y)
			else:
				coordinates.append(self.elements[i].z)
		return coordinates
