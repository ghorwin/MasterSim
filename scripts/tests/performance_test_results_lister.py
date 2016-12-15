#!/usr/bin/python

# Script to parse all summary.txt files from subdirectories and list subdirectories sorted according to 
# wall clock times sorted.
#
# Written by Andreas Nicolai (andreas.nicolai -at- tu-dresden.de), distributed under the BSD license
#
# Syntax:
# > performance_test_results_lister.py <base_directory>
#
# The base directory is expected to contain project result subdirectores which in turn contain the files
# logs/summary.txt. Only those directories are considered.
# All subdirectories must hold summary.txt files that match with respect to statistics counters.

import os
import os.path
import optparse
import shutil

from SolverStats import *

# *** main ***

# setup command line parser
parser = optparse.OptionParser()
(options, args) = parser.parse_args()

if len(args) < 1:
	print """
Syntax: performance_test_results_lister.py <base_directory>
	
The base directory is expected to contain project result subdirectores which in turn contain the files
logs/summary.txt. Only such directories are considered.

All subdirectories must hold summary.txt files that match with respect to statistics counters.
	"""	
	exit(1)
	
baseDir = args[0]

if not os.path.exists(baseDir):
	baseDir = os.path.join(os.getcwd(), baseDir)
	if not os.path.exists(baseDir):
		print "Directory '{}' does not exist or is not accessible".format(baseDir)
	

print "Gathering results in directory '{}'".format(baseDir)

refDirs = [fname for fname in os.listdir(baseDir) 
           if os.path.isdir(os.path.join(baseDir, fname))]

reference = None
referencePath = None

resMap = dict()

for d in refDirs:
	# check if logs/summary.txt is available
	summaryFilePath = os.path.join(baseDir, d + "/log/summary.txt")
	if not os.path.exists(summaryFilePath):
		continue
	print "Processing Directory '{}'".format(d)
	
	st = SolverStats()
	if not st.read(summaryFilePath):
		print "  Error reading summary.txt, skipped."
		continue
	if reference == None:
		# first summary.txt, store as reference
		reference = st
		referencePath = d
	else:
		# check that content matches
		if not SolverStats.compareStats(reference, st):
			print "  Statistics differ compared to reference stats from directory '{}'".format(referencePath)
			exit(1)
		
	# store with wall clock time as key in map
	resMap[st.timers["WallClockTime"]] = (d, st)
	

# sort map based on counters
wct = resMap.keys()
wct.sort()

for w in wct:
	print "  {:12.2f}  {}".format(w, resMap[w][0])
