MasterSim for macOS
===================

Installation
------------
Drag "MasterSim.app" onto the "Applications" folder.

The application is a universal binary and runs natively on Apple Silicon (arm64)
and Intel (x86_64) Macs, macOS 11 (Big Sur) or newer.

Command line solver
-------------------
The command line solver is contained in the application bundle:

    /Applications/MasterSim.app/Contents/MacOS/mastersim --help

Tip: create a link so that 'mastersim' is found in your PATH, e.g.

    sudo ln -s /Applications/MasterSim.app/Contents/MacOS/mastersim /usr/local/bin/mastersim

FMUs and processor architecture
-------------------------------
FMI 1.0/2.0 FMUs only contain one macOS binary directory (binaries/darwin64). If an
FMU contains only an x86_64 library, run MasterSim on Apple Silicon under Rosetta:

    arch -x86_64 /Applications/MasterSim.app/Contents/MacOS/mastersim project.msim

Documentation
-------------
MasterSim_Manual.pdf (English), MasterSim_Handbuch.pdf (German) and online:
https://bauklimatik-dresden.de/mastersim

License: BSD 3-Clause, see LICENSE.txt
