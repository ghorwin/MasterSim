import subprocess
import os

def filterFiles(fname):
	return fname.endswith(".h2")==True or fname.endswith(".cpp2")==True

allfiles = os.listdir('.')
files = filter(filterFiles, allfiles)

def refactorFile(fname):
	new_name = fname[:-1]
	# rename the files
	cmdline = "svn rename " + fname + " " + new_name
	print cmdline
	os.system(cmdline)

# process each file individually
for fname in files:
	refactorFile(fname)
