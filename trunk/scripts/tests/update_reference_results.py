# Helper script to update reference results.
#
# License: 
#   BSD License
#
# Authors: 
#   Andreas Nicolai <andreas.nicolai@tu-dresden.de>
#
# Script to update all files within reference directories (with given suffix) from currently computed cases
#
# Syntax:
# > update_reference_results.py <directory_suffix>


import os
import os.path
import optparse
import shutil

# *** main ***

# setup command line parser
parser = optparse.OptionParser()
(options, args) = parser.parse_args()

if len(args) < 1:
	print """
Syntax: update_reference_results.py <suffix>
	
Run this script in the directory of a single test case, for example 'data/tests/EN_10211_No1'.
The script will search for subdirectories with the given suffix, for example
'gcc_linux' and copy newly generated results over reference data files.
For each file replaced the script will print a line with the updated relative file path.
	"""	
	exit(1)
	
suffix = args[0]
print "Updating reference results for suffix '{}'".format(suffix)

# process all subdirectories with given suffix
rootDir = os.getcwd()
print "Processing base directory '{}'".format(rootDir)

refDirs = [fname for fname in os.listdir(rootDir) 
           if os.path.isdir(os.path.join(rootDir, fname)) and fname.endswith(suffix)]
refDirs.sort()

for d in refDirs:
	srcDir = d[:-(len(suffix)+1)]
	# process all files in each directory
	# if file with same name exists in base directory (without suffix), copy file over
	for root, dirs, files in os.walk(os.path.join(rootDir, d)):
		for f in files:
			print "Processing file '{}'".format(f) 
			fullPath = os.path.join(root, f)
			relFilePath = os.path.relpath(fullPath, d)
			# compose filepath of calculated files
			srcFilePath = os.path.join(rootDir, os.path.join(srcDir, relFilePath))
			# if file exists, copy it over
			if os.path.exists(srcFilePath):
				print "*** Updating {} ***".format(os.path.relpath(fullPath, rootDir))
				shutil.copyfile(srcFilePath, fullPath)

	
