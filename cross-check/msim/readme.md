# Validated Cross-Check FMUs

Only those subdirectories are added to subversion, that contain validated (passed) or rejected msim projects. The latter must contain a 'rejected' file with some meaningful explanation why the test case was omitted.
For those cases where MasterSim cannot (and will not in future) pass the test, you can add a file 'failed' in the subdirectory with an explanation why MasterSim cannot/will not pass this test.

Once a test case has been successfully submitted via pull request to the master repository, you can add a file 'passed' inside the subdirectory to remember the test case as completed (it will then be skipped in future result tests).

## Test procedure

see ../readme.md

