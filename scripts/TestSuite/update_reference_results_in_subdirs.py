#!/usr/bin/python

# Script to update all files within reference directories (with given suffix) from currently computed cases
#
# > update_reference_results.py <directory_suffix>


import os
import os.path
import optparse
import shutil

from update_reference_results import copyIfNotEqualD6O
from update_reference_results import copyIfNotEqualSummary
from update_reference_results import processCaseDir

HELPTEXT = """
Syntax: update_reference_results_in_subdirs.py <suffix> [<fileWithFoldersToProcess>]
	
Run this script in the directory of a test suite, for example 'data/tests'.
The script will search within all subdirectories for subdirectories with the given suffix, for example
'gcc_linux' and copy newly generated results over reference data files.
Data files will only be updated if file content differs, hereby time stamp of creation is ignored.
For each file replaced the script will print a line with the updated relative file path.

If a second argument is given, this must be a file path to a text file, containing in each
line of the file a folder relative to the current working directory. In this case, not all
subdirectories of the current working directory are processed, but only those in the file.
Special handling is applied if the file paths in the list point to *.d6p file. In these cases, 
the parent directories are processed.
"""	

# *** main ***

# setup command line parser
parser = optparse.OptionParser()
(options, args) = parser.parse_args()

if len(args) < 1:
	print HELPTEXT
	exit(1)
	
suffix = args[0]
print "Updating reference results for suffix '{}'".format(suffix)

# get current working directory
rootDir = os.getcwd()

# this list contains file paths that should be considered when updating reference results
# <refdir>.<suffix>  --> <refdir> must be in caseDirFilter
caseDirFilter = set()

if len(args) > 1:
	# open file
	casesFile = open(args[1], mode='r')
	caseDirPaths = casesFile.readlines()
	# handle filepath that have d6p files
	for c in caseDirPaths:
		# skip empty lines
		c = c.strip()
		if c == "":
			continue
		# remove trailing project file names
		if c.endswith("d6p"):
			c = c[:-4]
		c = os.path.abspath(os.path.join(rootDir, c))
		caseDirFilter.add(c)

# process all subdirectories with given suffix
print "Processing base directory '{}'\n".format(rootDir)
caseDirs = [fname for fname in os.listdir(rootDir) if os.path.isdir(os.path.join(rootDir, fname))]
caseDirs.sort()

for c in caseDirs:
	processCaseDir(c, rootDir, suffix, caseDirFilter)

