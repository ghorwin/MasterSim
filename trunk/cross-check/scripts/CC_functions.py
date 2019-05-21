# -*- coding: utf-8 -*-
"""
Cross-check utility functions for parsing the files:

- <model>_cc.csv  - predefined input to variables/parameters
- <model>_ref.opt - simulation parameters
- <model>_ref.csv - reference results

@author: Original author Hauke, revised and rewritten by Andreas
"""
import numpy as np
import shutil
import subprocess
import os

def readSimParameters(file_opt, file_ref):
	"""
	Reads the *.opt file, returns the tuple (StartTime, StopTime, StepSize, RelTol).
	"""

	# read Parameter from opt-file
	data_ref = np.loadtxt(file_ref, skiprows=1, delimiter=',')
	tstep_write = data_ref[1,0] - data_ref[0,0]
	f = open(file_opt,'r')
	cont = f.readlines()
	f.close()
	for n in range(len(cont)):
		if 'StartTime' in cont[n]:
			tstart = cont[n][10:].replace(',','').strip()
		if 'StopTime' in cont[n]:
			tstop = cont[n][9:].replace(',','').strip()
		if 'StepSize' in cont[n]:
			tstep = cont[n][8:].replace(',','').strip()    
		if 'RelTol' in cont[n]:
			rTol = cont[n][7:].replace(',','').strip()    

	return float(tstart), float(tstop), float(tstep), float(rTol), float(tstep_write)


# %%
def checkParameters(tstart, tstop, tstep):
	if tstart<0 or tstop<1e-4 or tstep<1e-12:
		dummy=0
#        raise Exception('\nSimulationsparameter aus .opt-Datei wahrscheinlich falsch!\n')

# %%
def createMasterSim(standard_ms, file_ms, modelname, tstart, tstop, tstep, tstep_write, rTol):
	# create MasterSim file
	shutil.copyfile(standard_ms, file_ms)     
		# read file
	f = open(file_ms, 'r')
	cont = f.readlines()
	f.close()
	# change Parameters in file                      
	cont[0] = cont[0][:21] + str(tstart) + ' s\n'
	cont[1] = cont[1][:21] + str(tstop) + ' s\n'
	cont[5] = cont[5][:21] + str(tstep_write) + ' s\n'
	cont[6] = cont[6][:21] + str(tstep_write) + ' s\n'
	cont[10] = cont[10][:21] + str(rTol) + '\n'
	# set fmu
	cont[15] = 'simulator 0 0 ' + modelname + ' #ffa0522d' + ' "{}.fmu"'.format(modelname)
	# write file
	f = open(file_ms, 'w')
	f.writelines(cont)
	f.close()

# %%    
def runSimulation(solver_win32, solver_win64, file_ms):
	# choose solver
	if 'win64' in file_ms:
		solver = solver_win64
	elif 'win32' in file_ms:
		solver = solver_win32
	else:
		raise Exception('\nunknown system platform!')
	# set command and run        
	command = '"' + solver + '" --verbosity-level=1 -x "' + file_ms + '"'            
	proc = subprocess.Popen(command)
	print('Simulation running...\n')
	proc.wait() # wait for process to finish
	print('Simulation finished\n')    

# %%    
def readMasterSimOutput(model_path, modelname, sort_abc):

	def getHeader(file):
		f = open(file,'r')     
		cont = f.readlines()
		for n in range(len(cont)):
			if cont[n].startswith('QUANTITY') :
				ind = cont[n].find('=')
				header = cont[n][ind+1:].split('|')
				break
		for n in range(len(header)):
			header[n] = header[n].strip().replace(modelname+'.','')
		return header

	# find all existing d6o files
	result_path = model_path + '\\0MS_' + modelname + '\\results\\'
	if os.path.exists(result_path):
		allfiles_d6o = os.listdir(result_path)
		if not allfiles_d6o:
			raise Exception('keine result Dateien vorhanden. Simulation fehlgeschlagen?')
	else:
		raise Exception('results Ordner nicht vorhanden. Simulation fehlgeschlagen?')

	# read data and merge in one array 
	header = ['time',]
	header += getHeader(result_path + allfiles_d6o[0])
	data = np.loadtxt(result_path + allfiles_d6o[0], skiprows=15)   
	data_cc = data # output data from first file
	for file_d6o in allfiles_d6o[1:]: # output data from following files
		data = np.loadtxt(result_path + file_d6o, skiprows=15)    
		header += getHeader(result_path + file_d6o)
		data_cc = np.append(data_cc,data[:,1:],axis=1)


	# if there is a txt-file specifying the correct outputs, sort them according to the numbers provided in the file 
	if os.path.exists(model_path + '/0_output_numbers.txt'):
		chosen_outputs = np.loadtxt(model_path + '/0_output_numbers.txt', delimiter=',',dtype=np.int_)
		data_cc = data_cc[:,chosen_outputs]          
		header = [ header[i] for i in chosen_outputs] # sort
		print('found and read "0_output_numbers.txt"\n')
	elif sort_abc:
		# sort header alphabetically and sort data accordingly
		inds = np.argsort(header[1:]) # get indizes from sorting (without "time")
		inds = np.insert(inds+1,0,0) # "time" is taken into account 
		data_cc = data_cc[:,inds]   # sort
		header = [ header[i] for i in inds] # sort
		if np.any(inds!=np.arange(0,len(header))):
			print('Data has been sorted alphabetically\n')

	return data_cc, header      


# %% 
def CheckandCutTimestep(data, tref):
	msg = ''
	# check if first value is 0 and delete it if so
	if data[0,0] == 0:
		data = np.delete(data,0,0)
	if data[0,0] != tref[0]: 
		msg += 'first time point invalid!\n'

	# check if last values are bigger than last time point and delete them
	ind = np.argwhere(data[:,0]>tref[-1])     
	if len(ind)>0:
		data = np.delete(data,ind,0)
		print('found time points higher than tstop and deleted them\n')

	if abs(data[-1,0] - tref[-1])>1e-6:
		msg += 'last time point invalid!\n'

	# check for constant timestep
	dt = np.diff(data[:,0])            
	if (abs(dt-dt[0])>1e-8).any():
		msg += 'time step not constant!\n'

	return data, msg            


## %% 
#def compareFiles(file_cc, file_ref, tref):
#    # compare time dependent data from cc and ref, tref is the time vector along which shall be compared
#    # hint: create tref using tref = np.arange(tstart,tstop,tstep)+tstep - thus tref is without tstart but including tstop
#    
#    # function that returns indices of t where t==tref
#    def refFilter(t,tref):
#        index=[]
#        for n in range(len(tref)):
#            for m in range(len(t)):
#                if abs(t[m]-tref[n])<1e-7:
#                    index.append(m)   
#                    break
#                elif m==len(t)-1: 
#                    index.append(-1)
#        return np.array(index)
#                 
#    # read data
#    data_cc = np.loadtxt(file_cc, skiprows=1, delimiter=',')    
#    data_ref = np.loadtxt(file_ref, skiprows=1, delimiter=',')    
#    
#    # filter time with respect to tref
#    ind_cc = refFilter(data_cc[:,0],tref)
#    ind_ref = refFilter(data_ref[:,0],tref)
#    if (ind_cc==-1).any():
#        raise Exception('missing point in time vector of data_cc!')
#    if (ind_ref==-1).any():
#        raise Exception('missing point in time vector data_ref!')
#    
#    # filter data    
#    data_cc_f = data_cc[ind_cc,:] 
#    data_ref_f = data_ref[ind_ref,:] 
#    
#    # compare data
#    diff = data_cc_f - data_ref_f
#    rmse = np.sqrt(np.sum((diff)**2)/len(data_cc_f))
#    
#    return rmse
#    
#    




