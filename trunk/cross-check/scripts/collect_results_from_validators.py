#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script to glob results of different vendors/tools for a specific Test FMU and
copy the files into a single directory. Hereby, the '{modelName}_out.csv' files are renamed to
'results_{vendor/tool}_{version}.csv', so that there are identified.

Only files from directories that contain the 'passed' file are copied. Also, there must not
be a file 'notCompliantWithLatestRules' in the same directory.


Start the script within fmi-cross-check directory. It will process all subdirectories in the
'results' directory and copy the result files to the respective fmu-case directory.

For example, the result file:

  results/2.0/cs/linux64/MasterSim/0.7.0/JModelica.org/1.15/ControlledTemperature/ControlledTemperature_out.csv

will be copied to

  fmus/2.0/cs/linux64/JModelica.org/1.15/ControlledTemperature/result_MasterSim_0.7.0.csv
  
It also creates a file that lists for each fmu test case the number of successfully reported results.
"""


import os
import shutil

FMI_CROSS_CHECK_DIRECTORY = "fmi-cross-check"

fullPath = os.path.abspath(FMI_CROSS_CHECK_DIRECTORY)
fullPath = fullPath.replace('\\', '/') # windows fix
fullPathParts = fullPath.split('/')

if not os.path.exists(fullPath + "/results"):
	print("Expected subdirectory 'results' within this directory.")
	exit(1)

print("Parsing available results in fmi-cross-check directory structure")


fmuResults = dict()

for root, dirs, files in os.walk(fullPath, topdown=False):

	root = os.path.join(fullPath, root) # full path to current fmu file
	rootStr = root.replace('\\', '/') # windows fix
	pathParts = rootStr.split('/') # split into component
	pathParts = pathParts[len(fullPathParts):] # keep only path parts below toplevel dir

	# we only process directories that can actually contain models, that means more than 4 path parts
	if len(pathParts) < 5:
		continue

	relPath = '/'.join(pathParts[1:]) # compose relative path
	# filter out everything except the fmus sub-directory
	if pathParts[0] != "results":
		continue

	# filter out Model Exchange fmus
	if pathParts[2] == "me":
		continue
	
	# now find xxx_out.csv files
	for name in files:
		if name.find("out.csv") != -1:
			# check if directory contains 'passed' file
			if not os.path.exists(root + "/passed"):
				continue
			
			# check if directory contains 'notCompliantWithLatestRules' file
			if os.path.exists(root + "/notCompliantWithLatestRules"):
				continue
			
			# compose target file name
			targetFile = "results_" + pathParts[4] + "_" + pathParts[5] + ".csv"
			
			# compose target path
			pathPartsFMU = [ "fmus", pathParts[1], pathParts[2], pathParts[3], pathParts[6], pathParts[7], pathParts[8] ]
			fmuDirPath = "/".join(pathPartsFMU)
			
			resFile = os.path.join(fullPath, fmuDirPath) + "/" + targetFile
			
			# copy source file to target file
			shutil.copyfile(root + "/" + name, resFile)
			
			if not fmuDirPath in fmuResults:
				fmuResults[fmuDirPath] = 0
			fmuResults[fmuDirPath] = fmuResults[fmuDirPath] + 1
			
		

print("Summary of collected results:")

fmuPaths = sorted(fmuResults.keys())
for fmuDirPath in fmuPaths:
	print ("{:76s} : {:>3d}".format(fmuDirPath, fmuResults[fmuDirPath]))
	
	
