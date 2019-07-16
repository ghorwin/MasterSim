#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Cross-check helper script, processes simulation results and generates/updates the files needed for publishing the cross-check results.

Steps:

- process entire directory structure in fmi-cross-check
- for each directory (cs only), generate local path as does the 'generate_mastersim_projects.py' script

For example: 
  from   ./fmi-cross-check/fmus/1.0/cs/linux64/JModelica.org/1.15/ControlledTemperature
  to     ./msim/fmus_1.0_cs_linux64_JModelica.org_1.15_ControlledTemperature
  (working directory)
  
- check if a msim project file exists in this working directory
- check if a result directory exists
- check if a values.csv file exists
- read 'values.csv' file
- read 'referenceValues.csv' file
- read 'rejected', 'fail', 'success' files if existing in working directory
- create README.md file in cross-check directory with documentation of the validation process
- if 'fail' exists, copy content of 'fail' file to README.md, create 'failed' file; done
- if 'rejected' exists, copy content of 'rejected' file to README.md, create 'rejected' file; done
- perform fairly strict value comparison for all quantities in 'referenceValues.csv'
  - for each variable compared, add a line with the absolute and relative norm in README.md

- populate summary db table with following data for each case:
  - case path
  - cs version
  - tool
  - platform
  - state (failed, rejected, success)
  - comment
  - details (content of readme file)
  
  summary db file should be read/appended (to support cross-platform testing)
"""

