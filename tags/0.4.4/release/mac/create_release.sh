#!/bin/bash
export PATH=~/Qt/4.8.7/bin:$PATH
python ../scripts/createRelease.py --product MasterSimulator --constants-path ../../MasterSim/src  $*

