# -*- coding: utf-8 -*-
#
# Delphin6GeoFileManager.py
#
# Copyright (c) 2013-today Andreas Nicolai, IBK, TU Dresden
#
# This file is part of MasterSim and free software. See LICENSE for details.

import os.path
import Delphin6GeoFile

class Delphin6GeoFileManager:
	"""Stores and provides geometry files for access by data files.

	Members:
	   geoFiles -- dictionary with geometry files, keys are full file paths to
	               geometry file, values are Delphin6GeoFile objects
	"""
	def __init__(self):
		self.clear()

	def clear(self):
		"""Clears cached data."""
		self.geoFiles = dict()

	def geoFile(self,geoFilePath):
		"""Returns the Delphin6GeoFile instance matching the file path.
		If geometry file has not been read, yet, the manager attempts to read
		the geometry file and returns an instance if successful.

		Arguments:
		   geoFilePath - Full file path to geometry file

		Returns:
		   Instance of Delphin6GeoFile object corresponding to geoFilePath, or
		   None if reading of file failed (error messages are written out).
		"""
		if not self.geoFiles.has_key(geoFilePath):
			geoFile = Delphin6GeoFile.Delphin6GeoFile()
			if geoFile.read(geoFilePath):
				self.geoFiles[geoFilePath] = geoFile
			else:
				print 'Error reading geometry file.'
				return None
		# return cached geometry file instance
		return self.geoFiles[geoFilePath]

