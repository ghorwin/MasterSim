#!/usr/bin/python

# create_class.py
# Copyright 2010 Andreas Nicolai <andreas.nicolai -[at]- tu-dresden.de>
#
# Script to automatically create a class header and implementation template
# including license section, namespace, and class declaration
#
# Before using this script adjust the constants below.

import sys

# namespace definition, enter empty string to omit namespace 
NAMESPACE = "IBK"

# Author token
AUTHOR = "Andreas Nicolai <andreas.nicolai -[at]- tu-dresden.de>"

# Copyright 
COPYRIGHT = "2011"

# License header, <author> is replaced with AUTHOR constant
LICENSE = """/*	IBK Library
	Copyright (C) <copyright>  <author>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
"""

def syntax():
	print 'Syntax: create_class.py <class name>'
	print 'Write class name in camel case.'

# check command line arguments
if len(sys.argv) != 2:
	syntax()
	exit(1)
	
classname = sys.argv[1]
if len(classname) < 3:
	print 'Error: Class name too short!'
	syntax()
	exit(1)

print 'Creating files for class "' + classname + '"'

# write header file
try:
	class_hdr = classname + ".h"
	class_hdr_obj = open(class_hdr, 'w')

	# write license information
	license = LICENSE.replace("<copyright>",COPYRIGHT).replace("<author>",AUTHOR)
	class_hdr_obj.write(license + "\n")

	# write include guard
	class_hdr_obj.write("#ifndef " + classname + "H\n")
	class_hdr_obj.write("#define " + classname + "H\n\n")

	# write namespace
	if not NAMESPACE=="":
		class_hdr_obj.write("namespace " + NAMESPACE + " {\n\n")

	# write class documentation header
	class_hdr_obj.write("/*!	\\brief Declaration for class " + classname + "\n")
	class_hdr_obj.write("	\\author " + AUTHOR + "\n")
	class_hdr_obj.write("	\n")
	class_hdr_obj.write("*/\n")
	
	class_hdr_obj.write("class " + classname + " {\n")
	class_hdr_obj.write("public:\n")
	class_hdr_obj.write("	/*! Default constructor. */\n")
	class_hdr_obj.write("	" + classname + "();\n")
	class_hdr_obj.write("	/*! Default destructor. */\n")
	class_hdr_obj.write("	~" + classname + "();\n\n")
	class_hdr_obj.write("	// *** PUBLIC MEMBER VARIABLES ***\n\n")
	class_hdr_obj.write("\nprivate:\n\n")
	class_hdr_obj.write("	// *** PRIVATE MEMBER FUNCTIONS ***\n\n")
	class_hdr_obj.write("	// *** PRIVATE MEMBER VARIABLES ***\n\n")
	class_hdr_obj.write("};\n\n")
	if not NAMESPACE=="":
		class_hdr_obj.write("} // namespace " + NAMESPACE + "\n\n")

	class_hdr_obj.write("#endif // " + classname + "H\n")
except IOError as (errno, strerror):
	print 'Cannot open ', class_hdr
	print "I/O error({0}): {1}".format(errno, strerror)
	exit(1)
del class_hdr_obj


# write implementation file
try:
	class_cpp = classname + ".cpp"
	class_cpp_obj = open(class_cpp, 'w')

	# write license information
	license = LICENSE.replace("<copyright>",COPYRIGHT).replace("<author>",AUTHOR)
	class_cpp_obj.write(license + "\n")

	# write include
	class_cpp_obj.write("#include \"" + classname + ".h\"\n\n")

	# write namespace
	if not NAMESPACE=="":
		class_cpp_obj.write("namespace " + NAMESPACE + " {\n\n")

	class_cpp_obj.write(classname + "::" + classname + "() {\n")
	class_cpp_obj.write("}\n\n")
	class_cpp_obj.write(classname + "::~" + classname + "() {\n")
	class_cpp_obj.write("}\n\n")
	if not NAMESPACE=="":
		class_cpp_obj.write("} // namespace " + NAMESPACE + "\n\n")
except IOError as (errno, strerror):
	print 'Cannot open ', class_cpp
	print "I/O error({0}): {1}".format(errno, strerror)
	exit(1)
del class_cpp_obj
