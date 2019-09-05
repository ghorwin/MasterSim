#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from CSVFile import *
import numpy as np
from fmpy.cross_check import validate_signal

"""Looks for a  xxx_ref.csv file in the current directory.
Reads it, and generates the min/max curves, which are stored in file "minmax.csv"
"""


for f in os.listdir("."):
	if f.find("_ref.csv") != -1:
		# open file and read it
		refFile = CSVFile()
		refFile.read(f)
		# generate min/max curves for all variables in file
		
		minMaxFile = CSVFile()
		for l in refFile.content:
			# copy time points
			minMaxFile.content.append( [ l[0] ])
		# copy time caption
		minMaxFile.captions.append( refFile.captions[0] )
		
		# now process all variables
		for i in range(1,len(refFile.captions)):
			# add columns for min and max value
			c = refFile.captions[i]
			minIdx = len(minMaxFile.captions)
			maxIdx = minIdx + 1
			minMaxFile.captions.append("min_" + c) 
			minMaxFile.captions.append("max_" + c)
			
			y_ref = refFile.values[i-1]
			y_res = y_ref
			t_ref = refFile.time
			t_start = t_ref[0]
			t_stop = t_ref[-1]
			t_band, y_min, y_max, outliers = validate_signal(t=t_ref, y=y_res, t_ref=t_ref, y_ref=y_ref, t_start=t_start, t_stop=t_stop)
		
			with open("minmax_"+c+".tsv", 'w') as minmaxF:
				minmaxF.write("Time [s]\tmin_{}\tmax_{}\n".format(c,c))
				
				for j in range(len(y_min)):
					minmaxF.write("{:g}\t{:g}\t{:g}\n".format(t_band[j], y_min[j], y_max[j]))
	

