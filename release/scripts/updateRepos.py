#!/usr/bin/python

# IBK Deployment Script System
# Copyright Andreas Nicolai <andreas.nicolai -at- tu-dresden.de>
# Released under BSD License.
#
#
# Script to update/checkout all repositories in products lists

import os
import platform
import argparse
import subprocess
import fileinput
import datetime
import shutil
import sys


def configCommandLineArguments():
	"""
	This method sets the available input parameters and parses them.

	Returns a configured argparse.ArgumentParser object.
	"""

	parser = argparse.ArgumentParser("parameter list")

	parser.add_argument('-p', '--product', dest='product', required=True, type=str, 
	                    help='Path to product definition file.')
	parser.add_argument('-w', '--working-directory', dest="workingDir", required=False, type=str, 
	                    help='Working directory to checkout product into (if working dir does not yet exist).')

	return parser.parse_args()


def parseRepoList(pathToRepoIni):
	repos = dict()
	
	# open repo file
	if not os.path.exists(pathToRepoIni):
		raise RuntimeError("Cannot file file '{}'".format(pathToRepoIni))
	fobj = open(pathToRepoIni, 'r')
	lines = fobj.readlines()
	fobj.close()
	del fobj	
	currentSection = ""
	for line in lines:
		line = line.rstrip()
		if len(line) == 0:
			continue
		if line[0] == ';':
			continue
		if line[0] == '[':
			pos = line.find(']')
			if pos == -1:
				raise RuntimeError("Malformed ini file '{}', line: '{}'".format(pathToRepoIni, line))
			currentSection = line[1:pos].strip()
			continue
		if currentSection == "":
			raise RuntimeError("Malformed ini file '{}', line: '{}'".format(pathToRepoIni, line))
		repos[currentSection] = line
	
	return repos

def updateRepos(workingDir, repos):
	for repo in repos:
		fullPathToRepo = os.path.join(workingDir, repo)
		if os.path.exists(fullPathToRepo):
			print "*** Updating working copy '{}'".format(repo)
			command = 'svn up "{}"'.format(repo)
			# Open sub-process and start build script.    
			p = subprocess.Popen(command, shell=True, cwd=workingDir)
		
			# p.communicate() waits for termination of subprocess
			(output, err) = p.communicate()        
		
			if p.returncode != 0:
				print "--------------------------------------------------------------------"
				raise RuntimeError('Error updating local working copy')    
		else:
			print "*** Checking out into working copy '{}'".format(repo)
			command = 'svn co "{}" "{}"'.format(repos[repo], repo)
			# Open sub-process and start build script.    
			p = subprocess.Popen(command, shell=True, cwd=workingDir)
		
			# p.communicate() waits for termination of subprocess
			(output, err) = p.communicate()        
		
			if p.returncode != 0:
				print "--------------------------------------------------------------------"
				raise RuntimeError('Error updating local working copy')    
				



#
# *** Main Application ***
#

# Reading user arguments
args = configCommandLineArguments()

try:
	if not os.path.exists(args.workingDir):
		print "Creating working directory '{}'".format(args.workingDir)
		os.mkdir(args.workingDir)
		if not os.path.exists(args.workingDir):
			raise RuntimeError("Working directory '{}' does not exist and cannot be created".format(args.workingDir))
	# try reading product repo list, repo is a dictionary with key=checkout-dir and value = svn-repo url
	repos = parseRepoList(args.product)

	# update repositories
	updateRepos(args.workingDir, repos)

except Exception as e:
	print(e)
	print '*** Error checkingout/updating repositories, aborting. ***'
	sys.exit(1)

sys.exit(0)
