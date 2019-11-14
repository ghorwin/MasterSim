Release Notes and ChangeLog for MasterSimulator
===============================================

Changelog for Version 0.8.2 (14.11.2019)
----------------------------------------

It is done. 60 pages of user manual in one week! Thanks to Asciidoctor even
with a decent layout. And, interestingly, by documenting all the features
very neatly, quite a few bugs appeared (and where fixed right away):

* Feature:              - Welcome page shows thumbnails of simulation scenarios
* Bugfix:  [Ticket #73] - Pass start time via setupExperiment() to slaves
* Bugfix:  [Ticket #72] - Fix unit conversion in output files
* Bugfix:  [Ticket #71] - Block network does not remember to show model.png image from FMU as block image
* Bugfix:  [Ticket #70] - Fix evaluation for file reader slaves and bool/​int/​enum values
* Feature: [Ticket #69] - Add support for csv excel export flavor
* Feature: [Ticket #67] - Write html color codes in project file in 6 digit form when alpha=ff
* Bugfix:  [Ticket #66] - Fix header of progress.txt
* Bugfix:  [Ticket #65] - MasterSim UI should check for valid slave names (no spaces!)
* Bugfix:  [Ticket #62] - results (values) are written in the wrong order.

The documentation is available on https://bauklimatik-dresden.de/mastersim/documentation.php.


Changelog for Version 0.8.1 (07.11.2019)
----------------------------------------

Only two days later the next release! Well, two small and meaningful extensions/bug fixes were
added and also some usability upgrades. Most useful is the ability to
define transformation rules between output and input variables (e.g. for flux sign
inversion or unit conversion). Also important is a performance upgrade that allows 
usage of FMUs with many (> 10000) exported parameters.

* Feature: [Ticket #64] - Improve parameter editing - currently bad scalability 
                          when importing FMUs with >10000 parameters
* Feature: [Ticket #61] - Apply changes in simulation settings when clicking on 
                          start button or pressing F9 for simulation start
* Feature: [Ticket #60] - Add properties to connections, such as unit conversions 
                          or sign inversions
* Feature: [Ticket #55] - Detect data type of csv-input slave's variables from connection graph


Changelog for Version 0.8.0 (05.11.2019)
----------------------------------------

This is a major update, which first introduces graphical routing
and a simulation network display (including printing/export to PDF).
Also, a lot of usability features have been added to the user interface.

* Feature: [Ticket #59] - Show unit of variables in connection view
* Feature: [Ticket #38] - Add button to clear start page of recently used projects
* Feature: [Ticket #23] - Implement slope-based error test using history information only
* Feature: [Ticket #12] - Highlight invalid connections in the connection graph
* Feature: [Ticket # 2] - Block-Modelling Data Structure and Interface to library


Changelog for Version 0.7.1 (03.08.2019)
----------------------------------------

This is a minor update, with only fine-tuning and a compatibility feature needed
for cross-checking/validation of MasterSim (the 'prevent overstepping' feature).
Also, on the Mac starting the simulation spawns a new terminal application window,
so no more running jobs in the background.

* Feature: Ticket #52 - Add option to 'prevent overstepping of end time'
* Feature: Ticket #54 - Open Terminal.app on MacOS when launching MasterSimulator
* Feature: Ticket #56 - Remove target directory when running new simulation from GUI
* Feature: Ticket #57 - Fix problem when FMU uses same value reference for several (different) variables


Changelog for Version 0.7.0 (09.07.2019)
----------------------------------------

This release is centered around standard compliance - a lot of small adjustments were
necessary, to run FMUs from very different tools.

One major improvement is the ability of the user interfance and solver to treat csv-format 
time series files (format specs see wiki) as a calculation slave that generates variables
by means of linear interpolation between tabulated values.

Another major addition is the ability to specify slave-specific parameter values in the user interface.

Here's a list of all features and bug fixes (details can be checked in the ticket system of the 
SourceForge project):

* Feature: Ticket #50 - Enable output of parameters and locals
* Feature: Ticket #48 - Detect synonymous variables and create a file that holds this information
* Feature: Ticket #47 - Distinguish between valuereference and value index when writing outputs
* Feature: Ticket #46 - Raise error if simulation is started that does not generate any outputs
* Feature: Ticket #43 - Add UI support for selection csv/​tsv files as input files.
* Feature: Ticket #42 - Add test case for FileReader slave
* Feature: Ticket #41 - Add feature that keeps application settings when upgrading to newer version
* Feature: Ticket #40 - Add shortcuts Ctrl+O (open project) and F9 (run simulation)
* Feature: Ticket #39 - Read internal variables and add option to write outputs for internal variables
* Bugfix:  Ticket #37 - Fix start page (recently opened projects and examples)
* Feature: Ticket #31 - Allow specifying fixed/​tunable parameters in GUI
* Feature: Ticket #29 - Enable reading of CSV-files with input variables and using them in connection graph

Changelog for Version 0.6.0 (23.05.2019)
----------------------------------------

Key focus in this release was the ability to run quite a lot of the FMU
test cases from the FMI cross-check repository. Interestingly, this 
involved reading up a bit on the standard specs and finding out, that quite
a few descriptions are a bit ambiguous or at least unclear, for example
whether the 'unit' attribute of Real-TypeDefinitions is mandatory or now.
In my opinion it is, yet Dymola doesn't write a unit... Anyhow, MasterSim
now treats it as optional and uses an undefined unit for outputs.

Also, output files are now written in standard csv-format, that can be easily 
imported into both PostProc 2 and Excel and the like. 

Here's a list of all features and bug fixes:

* Feature: Ticket #36 - Make _unit_ attribute optional in TypeDefinitions section of model description file.
* Bugfix:  Ticket #35 - Missing 'variability' attribute shouldn't prevent MasterSim from running the FMU
* Feature: Ticket #34 - Change output format to csv
* Feature: Ticket #33 - Automatic stepsize calculation when stepsize = 0
* Feature: Ticket #30 - Ensure that integration does not surpass end time point
* Feature: Ticket #27 - Automatically show log file after simulation has finished
* Bugfix:  Ticket #26 - Fix configuration/​settings name (registry/​.config file path)
* Bugfix:  Ticket #25 - Logfile cannot be opened from UI when filename contains a '.'
* Feature: Ticket #24 - Improve message output so that it is easier to grep through results
* Feature: Ticket #22 - Adjust format of step-statistics file to be usable in PostProcessing
* Feature: Ticket #21 - MasterSim should be deployed with icon files and create a .desktop file on Linux
* Feature: Ticket #20 - Iterative algorithms without time step adjustment should continue when max iters have been reached
* Feature: Ticket #19 - Improve diagnostic messages from iterative algorithms
* Feature: Ticket #18 - Add button in UI to show simulation log file
* Feature: Ticket #17 - Match timesteps in all output files
* Feature: Ticket #16 - Improve simulation log
* Feature: Ticket #15 - Boolean outputs are written to integer output file
* Feature: Ticket #14 - Improve display of project file comments on welcome page
* Feature: Ticket #13 - Add summary.txt output for regression test suite
* Feature: Ticket #11 - When loading a project, automatically analyze FMUs in the background without message window
* Feature: Ticket #10 - Enhance output of FMU analysis in UI
* Bugfix:  Ticket #9  - MasterSimUI fails to read modelDescription generated by OpenModelica 1.13.0
* Feature: Ticket #8  - Extend error test with slope check
* Bugfix:  Ticket #7  - Fix "reload project on external file change"
* Feature: Ticket #6  - Associate file extension msim with MasterSimulator user interface

Changelog for Version 0.5.3 (24.09.2018)
----------------------------------------

* Feature: Ticket #30 - Ensure that integration does not surpass end time point
* Feature: Ticket #23 - Implement slope-based error test using history information only
* Feature: Ticket #11 - When loading a project, automatically analyze FMUs in
                        the background without message window
* Feature: Ticket #27 - Automatically show log file after simulation has finished
* Bugfix:  Ticket #26 - Fix configuration/​settings name (registry/​.config file path)
* Bugfix:  Ticket #25 - Logfile cannot be opened from UI when filename contains a .
* Feature: Ticket #24 - Improve message output so that it is easier to grep through results
* Feature: Ticket #14 - Improve display of project file comments on welcome page
* Feature: Ticket #8  - Extend error test with slope check
* Feature: Ticket #22 - Adjust format of step-statistics file to be usable in PostProcessing
* Feature: Ticket #21 - MasterSim should be deployed with icon files and create a .desktop file on Linux

Changelog for Version 0.5.2 (02.08.2018)
----------------------------------------

* Feature: Ticket #20 - Iterative algorithms without time step adjustment
  should continue when max iters have been reached
* Feature: Ticket #19 - Improve diagnostic messages from iterative algorithms
* Feature: Ticket #18 - Add button in UI to show simulation log file

Changelog for Version 0.5.1 (23.01.2018)
----------------------------------------

* Bugfix: Ticket #15: Boolean outputs are written to integer output file
  (also fixes the segfault when having booling and integer outputs)
* Feature: Ticket #16: Improve simulation log (add solver version for bug reporting)
* Feature: Ticket #17: Match timesteps in all output files (use same number precision for time points)

Changelog for Version 0.5.0 (17.11.2017)
----------------------------------------

* Bugfix: Ticket #7: Fixed reloading of project files
  modified by external applications (editors)
* Bugfix: Ticket #9: MasterSimUI fails to read modelDescription
  generated by OpenModelica 1.13.0 (to strict error check)

New feature:

* Ticket #13: Added summary.txt output for regression test suite
  (automated regression testing is now supported and used)
* added support for whitespaces and linebreaks (encoded) in string parameters when
  specified in project file

Changelog for Version 0.4.4 (15.12.2016)
----------------------------------------

* Bugfix: Minimum time step size is now honored by the error test
* Bugfix: Output time grid is kept fixed even for variable stepping runs

New features:

*    solver and algorithm options are now checked more thoroughly, so that
     invalid combinations are flagged during initialization
*    improved algorithm option selection, so that algorithm variants can be
     specified more clearly (see also wiki-pages on algorithms and project file)
*    the leading comment block in a project file is now preserved, so annotations
     remain there when project is edited in user interface
*    step statistics are now written to the `log/stepstats.txt` file when master
     is run with verbosity level > 1

Changelog for Version 0.4.3 (06.10.2016)
----------------------------------------

* 32bit and 64bit Windows platforms supported
* Bugfix: Main window short cuts work again
* Bugfix: Project file format correctly written from user interface (parameter format fixed)

Changelog for Version 0.4 (14.06.2016)
--------------------------------------

* Bugfix: when adding slaves before project had been saved already, UI got stuck
* Stability improvement: added Gauss-Jacobi initial value iteration (when FMUs are in initialization mode).

Changelog for Version 0.3 (13.06.2016)
--------------------------------------

* User Interface is now completed to a point where definition of
  master scenarios is possible
* Project file format has changed slightly, parameter names are now
  the same as member variables in project (simplifies code maintenance)

Things you can do:

* you can add slaves
* you can define connections, even with the "same-name" auto-connection feature
* you can specify simulation parameters and run the simulation


Changelog for Version 0.2 (03.06.2016)
--------------------------------------

* UI Implementation has progressed a lot, Simulation can be started from UI
  and solver parameters can be edited
* project file format has been extended
* translation has been updated
* slave adding/removing/editing is implemented


Changelog for Version 0.1
-------------------------

First release, so there is no changelog yet. See svn-commits for details.
