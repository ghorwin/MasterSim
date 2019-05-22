#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
Cross-check script, processes FMU import test cases.
Copyright 2019, Andreas Nicolai <andreas.nicolai@gmx.net>

> python cross_check.py [-d <directory>] [-t cs|me] [-v 1|2] [-p <platform>] <fmi-directory>

Arguments:

    -d <directory>  Base/vendor directory name to process; use all if omitted
    -t cs|me        If specified use only Co-Simulation cases or Model Exchange cases; use all if omitted
    -v 1|2          Use either version 1 or 2 of the standard; use all if omitted
    -p <platform>   Platform (win32, win64, darwin64, linux32, linux64); use 64-bit version of current OS if omitted

    <fmi-directory> Root directory of fmi-cross-check repository.

Run this script from within a temporary directory where generated files can be placed.

Example:

> python cross_check.py -d FMI-Tests -t cs -v 1 -p linux64 ../fmi-cross-check

Will process all test cases within directory:

    ../fmi-cross-check/fmus/1.0/cs/linux64/Test-FMUs

"""


import numpy as np
import matplotlib.pyplot as pl
import os
import platform
import argparse

import CC_functions as cc
import MasterSimTestGenerator as msimGenerator


# *** main program ***


# command line arguments
parser = argparse.ArgumentParser(description="Runs cross-check tests for MasterSim")
parser.add_argument('-d', action="store", dest="vendorDir", help="Vendor/base directory to process")
parser.add_argument('-t', action="store", dest="fmiType", help="Operation mode (cs|me).")
parser.add_argument('-v', action="store", dest="fmiVersion", help="FMI version to use.")
parser.add_argument('-p', action="store", dest="platform", help="Platform (win32, win64, darwin32, darwin64, linux32, linux64).")
parser.add_argument(action="store", dest="fmiDirectory", help="Root directory with fmi-cross-check files.")

args = parser.parse_args()



# now process all subdirectories of root dir accordingly and filter out not selected directories
# create a list of all models to run
fullPath = os.path.abspath(args.fmiDirectory)
fullPathStr = fullPath.replace('\\' ,'/') # windows fix
fullPathParts = fullPathStr.split('/')

fmuList = []

print("Collecting list of FMUs to import and test-run")
for root, dirs, files in os.walk(fullPath, topdown=False):
   # split root directory into components
   
   root = os.path.join(fullPath, root)
   rootStr = root.replace('\\', '/') # windows fix
   pathParts = rootStr.split('/')
   pathParts = pathParts[len(fullPathParts):]
      
   # we only process directories that can actually contain models
   if len(pathParts) < 5:
      continue
   
   relPath = '/'.join(pathParts[1:])

   # filter out everything except the fmus sub-directory
   if pathParts[0] != "fmus":
      continue
      
   # filter out fmi version if given
   if args.fmiVersion != None:
      found = False
      if args.fmiVersion == "1" and pathParts[1] == "1.0":
         found = True
      if args.fmiVersion == "2" and pathParts[1] == "2.0":
         found = True
      if not found:
         continue
      
   # filter out fmi type if given
   if args.fmiType != None:
      found = False
      
      if args.fmiType == "cs" and pathParts[2] == "cs":
         found = True
      if args.fmiType == "me" and pathParts[2] == "me":
         found = True
      if not found:
         continue
      
   # filter out platform, if given
   osType = args.platform
   if osType == None:
      s = platform.system()
      if s == "Linux":
         osType = "linux64"
      elif s == "Windows":
         osType == "win64"
      else:
         osType == "darwin64"
   
   if pathParts[3] != osType:
      continue

   # now find .fmu files
   for name in files:
      e = os.path.splitext(name)
      if len(e) == 2 and e[1] == '.fmu':
         fmuPath = os.path.join(root, name)
         fmuList.append(fmuPath[:-4]) # strip the trailing .fmu
         print("  " + os.path.join(relPath, name))

print("{} FMUs to test".format(len(fmuList)))

passedFMUs = []
# read list of passed test cases
if os.path.exists("passed_tests.txt"):
   fobj = open("passed_tests.txt", 'r')
   passedFMUs = fobj.readlines()
   fobj.close()
   del fobj
   

# for each fmu, create an instance of our MasterSimImportTester class, parse the input/reference/options files 
# and then run the test
newFmuList = fmuList[:]
for fmuCase in fmuList:
   # check if case has alreade been completed successfully
   if fmuCase in passedFMUs:
      print('Case {} already completed, skipped'.format(fmuCase))
      continue

   # setup test generator (parse input files)
   masterSim = msimGenerator.MasterSimTestGenerator()
   masterSim.setup(fmuCase)
   
   # generate path to MasterSim working directory
   relPath = os.path.split(os.path.relpath(fmuCase, fullPathStr))[0] # get relative path to directory with fmu file
   relPath = relPath.replace('\\', '_') # windows fix
   relPath = relPath.replace('/', '_')

   # generate MasterSim file
   masterSim.generateMSim(relPath)
   
   # run MasterSim, expects MasterSimulator executable to be in the path
   res = masterSim.run()
   if not res:
      continue

   # read results
   
   # mark fmuCase as completed
      newFmuList.append(fmuCase)
      

# write completed FMU list
fobj = open("passed_tests.txt", 'w')
passedFMUs = "\n".join(newFmuList)
fobj.write(passedFMUs)
fobj.close()
del fobj


exit(1)





# %% Input   
model_number = 1 # starting from 1

#sort_abc = False
sort_abc = True

fmiversion = '1.0'
platform = 'linux64'

# neu
path0 = '/home/ghorwin/git/fmi-cross-check/fmus/'

#path_vendor = '20sim/4.6.4.8004/'
#path_vendor = 'AMESim/15/'
#path_vendor = 'ASim/2017/'
#path_vendor = 'Adams/2017.2/'
#path_vendor =  'CATIA/R2016x/'
#path_vendor = 'ControlBuild/2016/'
#path_vendor =  'DS_FMU_Export_from_Simulink/2.3.0/'
#path_vendor =  'Dymola/2017/'
#path_vendor =  'EcosimPro/5.6.1/'
#path_vendor = 'Easy5/2017.1/'
#path_vendor = 'FMIToolbox_MATLAB/2.3/'
#path_vendor = 'FMUSDK/2.0.4/'
#path_vendor = 'JModelica.org/1.15/'
#path_vendor = 'MapleSim/2018/'
#path_vendor = 'MWorks/2016/'
#path_vendor = 'PROOSIS/3.8.1/'
#path_vendor = 'Silver/3.3/'
#path_vendor = 'SimulationX/3.7.41138/'
path_vendor = 'Test-FMUs'

path1 = path0 + fmiversion + '/cs/' + platform + '/' + path_vendor


modelnames = os.listdir(path1)
if model_number<=len(modelnames):
    modelname = modelnames[model_number-1]
    print('\nModel: ' + modelname + '\n')
else:
    raise Exception('model_number zu hoch!')    


# path generation
solver_linux64 = '/home/ghorwin/svn/mastersim-code/bin/release/MasterSimulator'
solver_win32 = 'C:/Program Files (x86)/IBK/MasterSimulator 0.5/MasterSimulator.exe'
solver_win64 = 'C:/Program Files/IBK/MasterSimulator 0.5/MasterSimulator.exe'
model_path = path1 + modelname
file_cc = model_path + '/' + modelname + '_cc.csv'
file_ref = model_path + '/' + modelname + '_ref.csv'
file_ms =  model_path + '/0MS_' + modelname + '.msim'
file_opt = model_path + '/' + modelname + '_ref.opt'
standard_ms = r'C:\Daten\SimQuality_cloud\SimQuality\AP8\cross-check\0MS_standard.msim'

# %% main code 

# delete any passed or failed files
if os.path.exists(model_path + '/failed'):
    os.remove(model_path + '/failed')
if  os.path.exists(model_path + '/passed'):
    os.remove(model_path + '/passed')    

# read and check simulation parameters
(tstart, tstop, tstep, rTol, tstep_write) = cc.readSimParameters(file_opt, file_ref)
cc.checkParameters(tstart, tstop, tstep)

# create MasterSim file
cc.createMasterSim(standard_ms, file_ms, modelname, tstart, tstop, tstep, tstep_write, rTol)
       
# run Simulation
cc.runSimulation(solver_win32, solver_win64, file_ms)

# find and read output files
data_cc, header_cc = cc.readMasterSimOutput(model_path, modelname, sort_abc)

# read data and header from ref file
data_ref = np.loadtxt(file_ref, skiprows=1, delimiter=',')
Nvars = np.size(data_ref, axis=1) - 1
f = open(file_ref,'r')
header_ref = f.readline().replace('\n','')
header_ref = header_ref.split(',')




# modify anything here...    
#data_cc = np.delete(data_cc,-1,0)
tstep = 0.01







## ceck timestep
#tref = np.arange(tstart,tstop,tstep)+tstep
#data_cc, msg_cc = cc.CheckandCutTimestep(data_cc, tref)
#if len(msg_cc)>1:
#    print('cc data:\n' + msg_cc)
#data_ref, msg_ref = cc.CheckandCutTimestep(data_ref, tref)
#if len(msg_ref)>1:
#    print('ref data:\n' + msg_ref)

# ionterpolate

tref = np.arange(tstart,tstop,tstep)+tstep
data_cc_int = np.zeros((len(tref),Nvars+1))
data_cc_int[:,0] = tref
for n in range(Nvars):
    data_cc_int[:,n+1] = np.interp(tref,data_cc[:,0], data_cc[:,n+1])
data_ref_int = np.zeros((len(tref),Nvars+1))
data_ref_int[:,0] = tref
for n in range(Nvars):
    data_ref_int[:,n+1] = np.interp(tref,data_ref[:,0], data_ref[:,n+1])    


# %% make some checks
#if np.size(data_cc, axis=0) != np.size(data_ref, axis=0):
#    print('ungleicher Ausgabezeitschritt!')
#    diff = np.size(data_cc, axis=0) - np.size(data_ref, axis=0)
#    diff_rel = (np.size(data_cc, axis=0) - np.size(data_ref, axis=0))/np.size(data_ref, axis=0)
#    print('Differenz: {0:3d} Schritt(e) bzw. {1:5.2f}%'.format(diff, diff_rel*100))
    
if np.size(data_cc, axis=1) != np.size(data_ref, axis=1):
    print('\n ->ungleiche Anzahl an Ausgaben!\n\n')


# %% write cc-file
np.savetxt(file_cc, data_cc, fmt='%.5f', delimiter=',', header=','.join(header_cc), comments='')


# %% compare and compute rmse
RMSE=np.ones(Nvars)*100
for n in range(Nvars):
    diff = data_cc_int[:,n+1] - data_ref_int[:,n+1]
    RMSE[n] = np.sqrt(np.sum((diff)**2))#/len(data_cc_int[:,n+1]))
    pl.figure(n)
    pl.subplot(2,1,1)
    pl.plot(data_ref[:,0],data_ref[:,n+1],'o--k',linewidth=2.0, label='ref')
    pl.plot(data_cc[:,0],data_cc[:,n+1],'-r', label='MSim')
    pl.legend()
    pl.subplot(2,1,2)
    pl.plot(data_ref_int[:,0],diff,'m', label='Differenz')
    pl.legend()    

# %% create passed or failed file
print('RMSE:  {}\n'.format(str(RMSE)))
print('highest RMSE: {}'.format(max(RMSE)))
if (RMSE<1).all()    :
    f = open(model_path + '/passed','w')
    f.close()
    print('\nPASSED!')
else:
    f = open(model_path + '/failed','w')
    f.close()
    print('\nFAILED!')
    








