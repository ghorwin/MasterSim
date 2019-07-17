# Validated Cross-Check FMUs

Only those subdirectories are added to subversion, that contain validated (passed) or rejected msim projects. The latter must contain a 'rejected' file with some meaningful explanation why the test case was omitted.
For those cases where MasterSim cannot (and will not in future) pass the test, you can add a file 'failed' in the subdirectory with an explanation why MasterSim cannot/will not pass this test.


## Test procedure

1. after generating msim projects, run the cases until the results match (adjust numerical parameters accordingly)
2. generate submission files via scripts/glob_results.py and create pull request for fmi-cross-check repo
3. if successfully submitted, create a file 'passed' with full version name of successful program version in each subdirectory and commit .msim file, reference result file and 'passed' file to the svn repo

