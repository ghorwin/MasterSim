import subprocess
import os

def filterHeader(fname):
	return fname.endswith(".h")==True

def filterCPP(fname):
	return fname.endswith(".cpp")==True

allfiles = os.listdir('.')
headers = filter(filterHeader, allfiles)

def refactorFile(fname):
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
	status = ""
	
	# loop over all lines, find class name, find namespace
	have_namespace = False
	class_name = ""
	have_multiple_classes = False
	for line in lines:
		if line.find("class")==0:
			# already have a class in this file?
			if  class_name != "":
				print fname + '  Multiple class declarations!'
				have_multiple_classes = True
			else:
				# parse class name
				tokens = line.split()
				class_name = tokens[1]
	
	# search for namespace
	for line in lines:
		if line.find("namespace IBK {") == 0:
			have_namespace = True
	if (not have_namespace):
		print fname + '  Missing "namespace IBK {"'
	
	# no class, return
	if class_name == "":
		return
	# if we have a single class file, compose file names and include guards
	if have_multiple_classes:
		return
		
	header_name = 'IBK_' + class_name + '.h2'
	cpp_name = 'IBK_' + class_name + '.cpp2'
	include_guard = 'IBK_' + class_name + 'H'
	# skip fnames which already are correct
	if (header_name == fname):
		print fname + '  already has correct filename'
		return
	print fname
	print '  Header:       ' + header_name
	print '  CPP:          ' + cpp_name
	print '  IncludeGuard: ' + include_guard
	
	# try to open corresponding CPP file
	cppfname = fname[:-2] + '.cpp'
	have_cppfile = False
	if os.path.exists(cppfname):
		# also check all corresponding CPP files
		have_cppfile = True
		#print cppfname
	
	# rename the files
	os.system("svn rename " + fname + " " + header_name)
	if have_cppfile:
		os.system("svn rename " + cppfname + " " + cpp_name)

# process each file individually
for fname in headers:
	refactorFile(fname)
