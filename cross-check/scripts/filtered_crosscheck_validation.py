#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This small script takes a single command line argument, which must be
# the name of one of the platforms to be tested against, usually 'darwin64', 'linux64', 'win64' or 'win32'.
# 
# Then, it runs the 'fmpy.validate_vendor_repo' script and only processes 'cs' FMU types and with the given platform.

import argparse
import sys
import os

import fmpy.cross_check.validate_vendor_repo

parser = argparse.ArgumentParser(description='Processes only the directories of the selected CS platform.')
parser.add_argument('platform', help='The platform id to test against.')

args = parser.parse_args()

print("Validating for platform '{}'".format(args.platform))

# check if base directory exists
if not os.path.exists('fmi-cross-check'):
	print("Missing directory 'fmi-cross-check'")
	exit(1)

# rename /me/ subdirectories


# run cross check
fmu_count, result_count, problems = fmpy.cross_check.validate_vendor_repo.validate_repo('fmi-cross-check', True, platform_filter=args.platform, fmi_type_filter='cs')

# print results

print("\n")
print("#################################")
print("%d problems found in %s" % (len(problems), 'fmi-cross-check'))
print("Validated %d FMUs and %d results" % (fmu_count, result_count))
print("#################################")
print("\n")

for problem in problems:
	print()
	print(problem)
	
