Release Notes and ChangeLog for MasterSimulator
===============================================

Changelog for Version 0.4.3
---------------------------

* 32bit and 64bit Windows platforms supported
* Bugfix: Main window short cuts work again
* Bugfix: Project file format correctly written from user interface (parameter format fixed)

Changelog for Version 0.4
-------------------------

* Bugfix: when adding slaves before project had been saved already, UI got stuck
* Stability improvement: added Gauss-Jacobi initial value iteration (when FMUs are in initialization mode).

Changelog for Version 0.3
-------------------------

* User Interface is now completed to a point where definition of
  master scenarios is possible
* Project file format has changed slightly, parameter names are now
  the same as member variables in project (simplifies code maintenance)

Things you can do:

* you can add slaves
* you can define connections, even with the "same-name" auto-connection feature
* you can specify simulation parameters and run the simulation


Changelog for Version 0.2
-------------------------

* UI Implementation has progressed a lot, Simulation can be started from UI
  and solver parameters can be edited
* project file format has been extended
* translation has been updated
* slave adding/removing/editing is implemented


Changelog for Version 0.1
-------------------------

First release, so there is no changelog yet. See svn-commits for details.

