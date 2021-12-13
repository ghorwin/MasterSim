# About MasterSim ...

_MasterSim_ is an FMI Co-Simulation master and programming library. It supports the 
Functional Mock-Up Interface for Co-Simulation in Version 1.0 and 2.0. Using the functionality of 
version 2.0, it implements various iteration algorithms that rollback FMU slaves and increase stablity of coupled simulation.

<img src="doc/screenshots/SplashScreen_MasterSim.png" width="300px"/>

_MasterSim_ is actively maintained/developed by Andreas Nicolai at the TU Dresden, Institut für Bauklimatik.

The main webpage of _MasterSim_ is https://bauklimatik-dresden.de/mastersim.

## Quality Assurance

We have continuous integration scripts running (build-test of all C/C++ code) and solver regression tests. For details, see [Jenkins](https://baukli01.arch.tu-dresden.de/jenkins).

| Test | Result|
|-----|-----|
| CI - Linux 64-bit (Ubuntu 20.04.3 LTS; Qt 12.9) | [![Build Status](https://baukli01.arch.tu-dresden.de/jenkins/buildStatus/icon?job=NR-Linux-MasterSim&style=plastic)](https://baukli01.arch.tu-dresden.de/jenkins/job/NR-Linux-MasterSim/)    |
| CI - Windows 64-bit (Win10, VC 2019, Qt 5.15.2) | [![Build Status](https://baukli01.arch.tu-dresden.de/jenkins/buildStatus/icon?job=NR64-Win-MasterSim&style=plastic)](https://baukli01.arch.tu-dresden.de/jenkins/job/NR64-Win-MasterSim/)   |
| CI - MacOS 64-bit (10.11 "El Capitan", Qt 5.11.3) | [![Build Status](https://baukli01.arch.tu-dresden.de/jenkins/buildStatus/icon?job=NR-IOS-MasterSim&style=plastic)](https://baukli01.arch.tu-dresden.de/jenkins/job/NR-IOS-MasterSim/) |

## Quick Overview

The _MasterSim_ user interface allows importing of slaves and graphically routing input/output variables:

<img src="doc/screenshots/MasterSim_0.8.0_network_schematics.png" width="600px"/>

Connections can be reviewed and parametrized in the connection view:

<img src="doc/screenshots/MasterSim_0.8.0_connection_view.png" width="600px"/>

The various numerical parameters (_MasterSim_ gives fine-grained control over what happens during the simulation):

<img src="doc/screenshots/mastersim_simulation_settings_en_win.png" width="600px"/>

Results can be easily analyzed with the free Post-Processing tool _PostProc 2_ (https://bauklimatik-dresden.de/postproc):

<img src="doc/screenshots/PostProc2.png" width="600px"/>

The welcome page of the software gives you a project overview and up-to-date development news.

<img src="doc/screenshots/MasterSim_0.9.0_welcome_page.png" width="600px"/>

