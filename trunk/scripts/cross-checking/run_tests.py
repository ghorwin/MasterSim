import os
import subprocess

ROOT_DIR="../../fmus"
OS_REF="linux64"

TEMPLATE="""
tStart          			0.0 s
tEnd            			1.0 s
hStart      				1e-7 s
hOutputMin  				0.01 s
MasterMode      			GAUSS_SEIDEL
ErrorControlMode			NONE

maxIterations    			1

simulator 0 0 Sim1 #ff447cb4 "%FMUFILENAME"
"""

RES_FILE="results.txt"

resfobj = open(RES_FILE, "w")

for root, dirs, files in os.walk(ROOT_DIR, topdown=False):
	d = root.split("/")
	if not OS_REF in d:
		continue
	for f in files:
		if f.endswith(".fmu"):
			relPath = os.path.relpath(os.path.join(root, f), ROOT_DIR)
			print relPath
			# generate MasterSim project file in root dir
			projectFileName = "{}.msim".format(os.path.join(root, f)).replace(".fmu", "")
			fobj = open(projectFileName, "w")
			projectContent = TEMPLATE.replace("%FMUFILENAME", f)
			fobj.write(projectContent)
			del fobj
			
			# execute simulation
			try:
				ret = subprocess.call(["./MasterSimulator", "--working-directory={}".format(root), projectFileName])
			except e as Exception:
				print e
				ret = 1
			if ret == 0:
				success = "Success"
			else:
				success = "Failed"
				
			resfobj.write("{:8s} {}".format(success, relPath ))
