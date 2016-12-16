# Python script for updating include guards based on header file names
import subprocess
import os

def filterHeader(fname):
	return fname.endswith(".h")==True

def refactorHeader(fname):
	#print 'Refactoring file "'+ fname +'"'
	# general file checks
	try:
		fobj = open(fname, 'r')
		lines = fobj.readlines()
	except IOError:
		print fname + '  Error opening/reading file'
		fobj.close
		return
	fobj.close
	del fobj

	include_guard = fname[:-2]+'H'
	print fname
	
	# find line of first include guard
	for i in range(0,len(lines)):
		if lines[i].find("#ifndef") != -1:
			# TODO : everything up to here needs to be replaced with our header
			lines[i] = '#ifndef ' + include_guard + '\n'
			lines[i+1] = '#define ' + include_guard + '\n'
			break
	
	# find last line of include guard
	for j in range(0,len(lines)):
		i = len(lines)-j-1
		if lines[i].find("#endif") != -1:
			lines[i] = '#endif // ' + include_guard + '\n'
			break
	
	fobj = open(fname, 'w')
	fobj.writelines(lines)
	
# begin main script
allfiles = os.listdir('.')
headers = filter(filterHeader, allfiles)

# process each file individually
for fname in headers:
	refactorHeader(fname)
