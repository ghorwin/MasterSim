# -*- coding: utf-8 -*-

import numpy as np

class CSVFile:
	def __init__(self):
		self.captions = []
		self.content = []
		
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
					p1 = t.rfind('[')
					p2 = t.rfind(']')
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
				l[0] = "{:.6g}".format(tval)
				selectedValues = []
				for i in captionIndexes:
					selectedValues.append(l[i])
				f.write(",".join(selectedValues) + "\n")
		return
