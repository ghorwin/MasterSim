# Test suite runner script
#
# License: 
#   BSD License
#
# Authors: 
#   Andreas Nicolai <andreas.nicolai@tu-dresden.de>
#
# Syntax:
# > python run_tests.py --path <path/to/testsuite> --solver <path/to/solver/binary> --extension <project file extension>
#
# Example:
# > python run_tests.py --path ../../data/tests --solver ./DelphinSolver --extension d6p

import subprocess		# import the module for calling external programs (creating subprocesses)
import sys
import os
import os.path
import shutil
import filecmp          # for result file comparison
import argparse
import platform         # to detect current OS
from colorama import *
from SolverStats import *
import config
from print_funcs import *


def configCommandLineArguments():
    """
    This method sets the available input parameters and parses them.

    Returns a configured argparse.ArgumentParser object.
    """

    parser = argparse.ArgumentParser("parameter list")

    parser.add_argument('-p', '--path', dest='path', required=True, type=str, 
                        help='Path to test suite root directory.')
    parser.add_argument('-s', '--solver', dest='solver', required=True, type=str, 
                        help='Path to solver binary.')
    parser.add_argument('-e', '--extension', dest="extension", required=True, type=str, 
                        help='Project file extension.')
    parser.add_argument('--no-colors', dest="no_colors", action='store_true', 
                        help='Disables colored console output.')
    parser.add_argument('--no-testinit', dest="no_testinit", action='store_true', 
                        help='Disables testinit on missing reference results (can be used to generate reference '
                        'results for all cases).')
    
    return parser.parse_args()


def checkResults(dir1, dir2):
    """
    Compares two result directories for equal contents.

    Arguments are dir1 (reference results) and dir2 (computed results).
    
    Compared are:
    
    - physical results
    - solver counters
    """
    try:
        # open stat files and compare them
        stats1 = SolverStats()
        if not stats1.read(dir1 + "/log/summary.txt"):
            return False
        stats2 = SolverStats()
        if not stats2.read(dir2 + "/log/summary.txt"):
            return False
        
        if not SolverStats.compareStats(stats1, stats2):
            printError("Mismatching statistics.")
            return False
        
        # compare all result files (d60, tsv), if any reference result files exist
        if os.path.exists(dir1 + "/results"):
            if not SolverStats.compareResults(dir1 + "/results", dir2 + "/results"):
                printError("Mismatching values.")
                return False
    except Exception as e:
        printError("Error comparing simulation results, error: {}".format(e))
    return True





# *** main script ***

args = configCommandLineArguments()

if not args.no_colors:
    init() # init ANSI code filtering for windows
    config.USE_COLORS = True

# process all directories under test suite directory
currentOS = platform.system()
compilerID = None
if currentOS   == "Linux" :
    compilerID = "gcc_linux"

elif currentOS == "Windows" :
    compilerID = "win_VC10"

elif currentOS == "Darwin" :
    compilerID = "gcc_mac"

if compilerID == None:
    printError("Unknown/unsupported platform")
    exit(1)
else:
    print "Compiler ID            : " + compilerID

print "Test suite             : " + args.path
print "Solver                 : " + args.solver
print "Project file extension : " + args.extension
print "\n"

# walk all subdirectories (except .svn) within testsuite and collect project file names
projects = []
for root, dirs, files in os.walk(args.path, topdown=False):
    for name in files:
        if name.endswith('.'+args.extension):
            projectFilePath = os.path.join(root, name)
            projects.append(projectFilePath)
            
projects.sort()

failed_projects = []

for project in projects:
    print project
    path, fname = os.path.split(project)
    #print "Path    : " + path
    #print "Project : " + fname

    cmdline = [args.solver, project]
    useTestInit = False
    
    # compose path of result folder
    resultsFolder = project[:-(1+len(args.extension))]
    referenceFolder = resultsFolder + "." + compilerID
    if not os.path.exists(referenceFolder):
        failed_projects.append(project)
        printError("Missing reference data directory '{}'".format(os.path.split(referenceFolder)[1]))
        if not args.no_testinit:
            cmdline.append("--test-init")
            useTestInit = True

    try:
        # run solver 
        FNULL = open(os.devnull, 'w')
        if platform.system() == "Windows":
            cmdline.append("-x")
            cmdline.append("--verbosity-level=0")
            retcode = subprocess.call(cmdline, creationflags=subprocess.CREATE_NEW_CONSOLE)
        else:
            retcode = subprocess.call(cmdline, stdout=FNULL, stderr=subprocess.STDOUT)
        # check return code
        if retcode == 0:
            # successful run
            if useTestInit > 0:
                printNotification("Init test successful.")
                # Note: project is already marked as failed, because of missing reference results.
            else:
                # now check against reference results
                if not checkResults(referenceFolder, resultsFolder):
                    if not project in failed_projects:
                        failed_projects.append(project) # mark as failed
                    printError("Mismatching results.")
        else:
            # failure to initialize or finish simulation
            if useTestInit > 0:
                printError("Init test failed, see screenlog file {}".format(os.path.join(os.getcwd(), 
                    resultsFolder+"/log/screenlog.txt"  ) ) )
                # Note: project is already marked as failed, because of missing reference results.
            else:
                if not project in failed_projects: # .index(project) == -1:
                    failed_projects.append(project)
                printError("Simulation failed, see screenlog file {}".format(os.path.join(os.getcwd(), 
                    resultsFolder+"/log/screenlog.txt"  ) ) )
    except OSError as e:
        printError("Error starting solver executable '{}', error: {}".format(args.solver, e))
        exit(1)

print ""
if len(failed_projects) > 0:
    print "Failed projects:"
    for p in failed_projects:
        printError(p)
    print ""
    printError("*** Failure ***")
    exit(1)

printNotification("*** Success ***")
exit(0)
