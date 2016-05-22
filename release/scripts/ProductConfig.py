#!/usr/bin/python

# IBK Deployment Script System
# Copyright Andreas Nicolai <andreas.nicolai -at- tu-dresden.de>
# Released under BSD License.
#
#
# This file holds the class ProductConfig which collects all information about a software product
# to build and to create a release for it.

import os
import platform
import datetime
import shutil
import sys

class ProductConfig:
	def __init__(self):
		self.productID = None
		self.version = None
		self.longVersion = None
		self.parameters = dict()
		
	def parseProductsIni(self, product, pathToProductsInfo):
		self.productID = product
		# open products ini, if existing, 
		if not os.path.exists(pathToProductsInfo):
			return
		fobj = open(pathToProductsInfo, 'r')
		lines = fobj.readlines()
		productFound = False
		inSection = False
		for line in lines:
			line = line.rstrip()
			if len(line) == 0:
				continue
			if line[0] == ';':
				continue
			if line[0] == '[':
				if inSection:
					break
				pos = line.find(']')
				if pos == -1:
					continue
				sectionTitle = line[1:pos].strip()
				if sectionTitle == product:
					inSection = True
					productFound = True
				continue
			if not inSection:
				continue
			pos = line.find('=')
			if pos != -1:
				key = line[0:pos].strip()
				value = line[pos+1:].strip()
				self.parameters[key] = value
		fobj.close()
		del fobj
		
		if not productFound:
			raise RuntimeError("Configuration for product '{}' not found.".format(product))
