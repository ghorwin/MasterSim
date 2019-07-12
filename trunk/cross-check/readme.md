# MasterSim - FMI Cross-Check

This directory is the working directory for the cross-check scripts. The actual python 
scripts that run the cross-check are located in the `scripts` subdirectory.

The template msim-project file used to generate the simulations is located in `templates`.

## Preparation steps

1. clone the repository https://github.com/modelica/fmi-cross-check into this directory

## Generate the MasterSim project files

Run the script from within a temporary directory, for example:

> mkdir tmp
> cd tmp
> python ../scripts/cross_check.py ../fmi-cross-check 

to generate not-yet existing MasterSim project files. Delete existing msim-files to recreate 
all project files. You can filter FMI version, tool vendor, platform.

## Test-initialize all MasterSim project files

Run from within the temporary directory.

> python ../scripts/test_init.py

This will generate a list of test cases that could be successfully initialized.

## Run and check all test FMUs

-> do this one-by-one and compare reference results with computed results

## Publish the results

Preparation: generate at least one representative figure

(only for MasterSim developers)

1. Fork the original https://github.com/modelica/fmi-cross-check repo into your own repository
2. clone this repository somewhere
3. ... 

