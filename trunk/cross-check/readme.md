# MasterSim - FMI Cross-Check

This directory is the working directory for the cross-check scripts. The actual python 
scripts that run the cross-check are located in the `scripts` subdirectory.

The template msim-project file used to generate the simulations is located in `templates`.

## Preparation steps

1. fork the https://github.com/modelica/fmi-cross-check repo into your own github repo
2. clone the forked repo
3. edit the master cross_check.py script first and specify the correct paths to the repository 
   (see comments in the file)

## Run the cross-check scripts

Then, you can simply run the script:

> python scripts/cross_check.py

## Publish the results

... 
