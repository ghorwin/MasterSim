# MasterSim - FMI Cross-Check Procedure

This directory is the working directory for the cross-check scripts. The actual python 
scripts that run the cross-check are located in the `scripts` subdirectory.

## Preparation steps

1. fork the repository https://github.com/modelica/fmi-cross-check into your own github account
2. clone your forked repo into this directory, such that it resides in 'fmi-cross-check'
3. install python dependencies:

```bash
> pip3 install fmpy
> pip3 install scipy
```

## Generate the MasterSim project files

Run the script from within this directory:

```bash
> python scripts/generate_mastersim_projects.py -v 2 -t Test-FMUs
```

where the argument after -v is either 1 or 2 (for FMI version), and the argument after -t is the name of the tool/vendor.

This will generate msim-files inside the subdirectory `msim`, that should be ready for simulation (though you may need to tweak a few parameters). Note, for each test case, an own subdirectory is created below `msim`.

*Note:* If you had already existing msim files (potentially with local modifications), the script will **not** overwrite these project files (so, you won't loose your own modifications).


## Run and adjust the MasterSim project files

Open the individual msim-files with MasterSim and run them. 

*Note:* If you continue to the next step, they will be automatically executed the first time results shall be evaluated.


## Extract required values from simulation results

Run in current directory:

```bash
> python scripts/glob_results.py
```

This script will process all newly created results, compare them (briefly) against reference values and create required result files (passed, README.md, xxx_out.csv) 
in result subdirectory of fmi-cross-check repo.

### Mark invalid tests

When a test cannot be checked, the directory can be rejected and thus be excluded from future tests in glob_results.py. In this case, create a file `rejected` with some explanation in the working directory for the test case (within the msim-subdirectory).

## Publish the results

### Check compliance before publishing

In this directory, call:

```bash
> python3 -m fmpy.cross_check.validate_vendor_repo  fmi-cross-check
```

to check, if the newly created submission files will pass the commit test.

### Commit and push to your forked repository

Now stage and commit the new files. For each subdirectory this will be, either:

* passed
* README.md
* {modelName}_out.csv

or 

* rejected

Push your changes to your forked github repo.

### Make sure your local fork is in sync with the upstream repository

1. add https://github.com/modelica/fmi-cross-check as upstream repository  (you have to do this only once)

    # within the fmi-cross-check working directory
    > git remote add upstream https://github.com/modelica/fmi-cross-check

2. fetch changes from upstream
3. merge upstream changes into your forked copy

    > git fetch upstream
    > git merge upstream/master

See https://help.github.com/en/articles/syncing-a-fork for more info.

In case of merged changes, create a merge commit and push this to your forked github repo.

### Create a pull request

Now open your github repo in a webbrowser and create a new pull request. The pull request will show the changes you made over the upstream repository and also (a little bit down on the page, when you click on 'view pull request') the status of the validation script running on CircleCI.

Once the pull request has been successfully checked, the maintainer of the https://github.com/modelica/fmi-cross-check will take your changes into the central repository.


## Update local validation database

The `glob_results.py` script maintains a csv-database-file `results/cross-check-results.csv` of the performed and completed validation tests. It will be updated whenever you run `glob_results.py`. In order to keep your test cases validation info, even when changing to another platform, you must check in the successfully validated working directory within `msim` and also create a `passed` file in each test-case working directory. The file should contain the version of MasterSim, that was used to validate the test. Usually, this will be a text like 'MasterSim_0.7.0'.
