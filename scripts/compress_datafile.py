#!/usr/bin/python

# Script to compress data files holding time-value series data
# by removing all data samples that can be re-constructed through
# linear interpolation between the preceeding and following sample.
#
# Author: Andreas Nicolai <andreas.nicolai -at- tu-dresden.de>
# License: GPL, see LICENSE file
#
# Syntax: compress_datafile <datafile> --skip-rows 12 [--output-file <target datafile path>]
#
# Compressed file is stored under name 

import os
import optparse

RELTOL = 1e-4
ABSTOL = 1e-6

def parseLine(lineStr):
	value_strings = lineStr.split()
	values = []
	for i in range(len(value_strings)):
		values.append( float(value_strings[i]) )
	return values

def removeRedundantValues(valueLines, stringLines, offset):
	# process all intervals and remove all rows that can be reconstructed through interpolation of 
	# its predecessor or follower
	# valueLines and stringLines is altered
	intervalFound = True
	while intervalFound:
		intervalFound = False
		# first process all intervals
		j = 0
		print ("{} lines to process".format(len(valueLines)))
		skipIndexes = []
		while j+2 < len(valueLines):
			refVals = valueLines[j]
			skipCandidate = valueLines[j+1]
			otherVals = valueLines[j+2]
			# difference
			step = otherVals[0] - refVals[0]
			alpha = (otherVals[0] - skipCandidate[0])/step # weight factor for refVals
			beta = 1 - alpha # weight factor for otherVals
			if alpha < 0 or alpha > 1:
				print("ERROR: time steps not consecutively increasing.")
				exit (1)
			allGood = True
			for k in range(1, len(refVals)):
				interpValue = alpha*refVals[k] + beta*otherVals[k]
				diff = abs(interpValue-skipCandidate[k])
				normDiff = diff/(abs(skipCandidate[k])*RELTOL + ABSTOL)
				if normDiff > 1:
					allGood = False # difference too large, need to keep the line
					break
			if allGood:
				# we can remove the line
				skipIndexes.append(j+1)
				intervalFound = True
			j = j + 2 # keep the line
		if len(skipIndexes) != 0:
			newValueLines = []
			newLines = stringLines[:offset]
			j = 0
			for i in range(len(skipIndexes)):
				nextSkipIndex = skipIndexes[i]
				for k in range(j, nextSkipIndex):
					newValueLines.append(valueLines[k])
					newLines.append(stringLines[offset+k])
				j = nextSkipIndex + 1
			# append remaing lines
			for k in range(j, len(valueLines)):
				newValueLines.append(valueLines[k])
				newLines.append(stringLines[offset+k])
			stringLines = newLines
			valueLines = newValueLines
	return stringLines
	
parser = optparse.OptionParser("Syntax: %prog <datafile> --skip-rows 12 [--output-file <target datafile path>]")
parser.add_option("--skip-rows", dest="skip_rows", default="0", type="int", 
                  help="specify number of header rows to skip")
parser.add_option("-o", "--output-file", dest="output_file", default=None, type="string", 
                  help="optional output file path")

(options, args) = parser.parse_args()
if len(args) != 1:
	print ("Invalid syntax, use --help")
	exit (1)

# open input file
try:
	print("Reading file {}".format(args[0]))
	fobj = open(args[0], "r")
	lines = fobj.readlines()
except IOError as err:
	print(err.message())
	print("Error opening input file.")
	exit (1)
del fobj

rowCount = len(lines)
print("{} lines of data in source file".format(rowCount))

# open output file
outputfile = args[0] + ".out"
if options.output_file != None:
	outputfile = options.output_file

try:
	print("Writing to file {}".format(outputfile))
	fobj = open(outputfile, "w")
	
	# copy first skip_rows number of rows
	current = 0
	if options.skip_rows > 0:
		for i in range(0,options.skip_rows):
			# check that we have enough lines
			if i >= rowCount:
				print ("ERROR: Not enough lines in file (check skip-rows option).")
				exit (1)
			fobj.write(lines[i])
			if not lines[i].endswith("\n"):
				fobj.write("\n")
		current = options.skip_rows
				
	# check that we have enough lines
	if current >= rowCount:
		print ("No data lines in file")
		exit (1)
		
	# read and parse all lines
	valCount = -1
	valueLines = []
	for i in range(current, rowCount):
		lineStr = lines[i]
		if len(lineStr.strip()) == 0:
			if len(valueLines) == 0:
				print("ERROR: No data in file")
				exit (1)
			else:
				print("End of data section found in line #{}, done.".format(i+1))
				break
		# first line determines number of values
		vals = parseLine(lineStr)
		if valCount == -1:
			if len(vals) == 0:
				print ("ERROR: No data lines in file or empty line found at begin.")
				exit (1)
			valCount = len(vals)
		else:
			if len(vals) != valCount:
				print ("ERROR: Value count mismatch in line #{}.".format(i+1))
				exit (1)
		valueLines.append(vals)

	# now remove values iteratively
	lines = removeRedundantValues(valueLines, lines, current)

	# now write all remaining lines out
	for i in range(current, len(lines)):
		fobj.write(lines[i])
		if not lines[i].endswith("\n"):
			fobj.write("\n")

	del fobj
	print ("Done")
	exit (0)
	
except IOError as err:
	print(err.message())
	print("Error opening/writing to output file.")
	exit (1)
