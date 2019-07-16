#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from datetime import datetime

class CrossCheckResult:
	"""A class that encapsulates a single entry for a performed cross check result.
	"""
	
	def __init__(self, date=datetime.now(), fmuCase="", csversion="", platform="", tool="", result="", note=""):
		self.fmuCase = fmuCase
		self.date = date
		self.csversion = csversion
		self.platform = platform
		self.tool = tool
		self.result = result # should be one of 'pass', 'fail', 'reject'
		self.note = note # used for fail and reject, holds content of 'failed' or 'rejected' file
		
	def __repr__(self):
		"""Prints a unique representation of the data members, used to generate the hash for the class"""
		return 'CrossCheckEntry(datetime("{}"),"{}","{}","{}","{}")'.format(self.date.strftime("%d.%m.%Y %H:%M:%S"), self.fmuCase, self.csversion, self.platform, self.tool, self.result)
		
	def __str__(self):
		"""Prints a readible string representation of the data members, used for debugging purposes"""
		return repr(self)
	
	def __hash__(self):
		"""Generates a hash to uniquely identify this entry.
		   The hash is currently only based on the fmu case name, which encodes platform, os, version, but not date
		"""
		return hash(self.fmuCase)
	
	def read(self, line):
		"""Extracts the content of the class members from a tab-separated line."""
		tokens = line.strip().split('\t')
		assert(len(tokens) == 7)
		self.date = datetime.strptime(tokens[0], "%d.%m.%Y %H:%M:%S")
		self.fmuCase = tokens[1]
		self.csversion = tokens[2]
		self.platform = tokens[3]
		self.tool = tokens[4]
		self.result = tokens[5]
		self.note = tokens[6]
		
	def write(self):
		"""Writes data tab-separted into line."""
		return "{}\t{}\t{}\t{}\t{}\t{}\t{}".format(self.date.strftime("%d.%m.%Y %H:%M:%S"), self.fmuCase, self.csversion, self.platform, self.tool, self.result, self.note)
