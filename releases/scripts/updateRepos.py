#!/usr/bin/env python3

# Script to update/checkout all repositories in products lists
#
# > python3 updateRepos.py -p MasterSim -w /tmp/MasterSim

import os
import platform
import argparse
import subprocess
import fileinput
import datetime
import shutil
import sys

from build import getCurrentOS

def configCommandLineArguments():
	"""
	This method sets the available input parameters and parses them.

	Returns a configured argparse.ArgumentParser object.
	"""

	parser = argparse.ArgumentParser("updateRepos.py")

	parser.add_argument('-p', '--product', dest='product', required=True, type=str, 
	                    help='Path to product definition file.')
	parser.add_argument('-w', '--working-directory', dest="workingDir", required=False, type=str, 
	                    help='Working directory to checkout product into (if working dir does not yet exist).')

	return parser.parse_args()


def parseRepoList(pathToRepoIni):
	"""
	This function processes all ini files in `/products` and
	extracts information on the repository path.
	"""
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

def updateRepo(repoURL, repo, workingDir):
	targetDir = os.path.join(workingDir, repo)
	# check if repo exists, and if that is the case, only update the repo
	if os.path.exists(targetDir):
		print("*** Updating working copy '{}'".format(repo))
		# we distinguish between svn and git repos
		if repoURL.startswith("svn+ssh"):
			command = 'svn up'
		else:
			currentOS = getCurrentOS()
			if currentOS   == "linux":
				command = 'git reset --hard HEAD; git pull --rebase --recurse-submodules'
			else:
				command = 'git reset --hard HEAD & git pull --rebase --recurse-submodules'
		# Open sub-process
		p = subprocess.Popen(command, shell=True, cwd=targetDir)
		# p.communicate() waits for termination of subprocess
		(output, err) = p.communicate()        
	
		if p.returncode != 0:
			print("--------------------------------------------------------------------")
			raise RuntimeError('Error updating local working copy')    
	else:
		print("*** Checking out into working copy '{}'".format(repo))
		if repoURL.startswith("svn+ssh"):
			command = 'svn co "{}" "{}"'.format(repoURL, repo)
		else:
			command = 'git clone --recursive {} {}'.format(repoURL, repo)
		# Open sub-process
		p = subprocess.Popen(command, shell=True, cwd=workingDir)
		# p.communicate() waits for termination of subprocess
		(output, err) = p.communicate()        
	
		if p.returncode != 0:
			print("--------------------------------------------------------------------")
			raise RuntimeError('Error updating local working copy')    

def updateRepos(workingDir, repos):
	for repo in repos:
		updateRepo(repos[repo], repo, workingDir)


#
# *** Main Application ***
#

# Reading user arguments
args = configCommandLineArguments()

try:
	if not os.path.exists(args.workingDir):
		print("Creating working directory '{}'".format(args.workingDir))
		os.mkdir(args.workingDir)
		if not os.path.exists(args.workingDir):
			raise RuntimeError("Working directory '{}' does not exist and cannot be created".format(args.workingDir))
	# try reading product repo list, repo is a dictionary with key=checkout-dir and value = svn-repo url
	repos = parseRepoList(args.product)
	
	# update repositories
	updateRepos(args.workingDir, repos)

except Exception as e:
	print(e)
	print('*** Error checkingout/updating repositories, aborting. ***')
	sys.exit(1)

sys.exit(0)
