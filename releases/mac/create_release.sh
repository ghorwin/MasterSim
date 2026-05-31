#!/bin/bash

# clone repo
if [ ! -d "MasterSim.git" ]; then
  echo "Cloning github repo"
  git clone https://github.com/ghorwin/MasterSim.git MasterSim.git
else
  echo "Reverting local changes and pulling newest revisions from github"
  (cd MasterSim.git; git reset --hard HEAD; git pull --rebase)
fi &&

# copy script directory and helper files
if [ ! -d "MasterSim.git/release" ]; then
  mkdir MasterSim.git/release
fi &&
if [ ! -d "MasterSim.git/release/scripts" ]; then
  mkdir MasterSim.git/release/scripts
fi &&
if [ ! -d "MasterSim.git/release/mac" ]; then
  mkdir MasterSim.git/release/mac
fi &&
if [ ! -d "MasterSim.git/release/mac/qt_menu.nib" ]; then
  mkdir MasterSim.git/release/mac/qt_menu.nib
fi &&

cp -R ../scripts/* MasterSim.git/release/scripts/ &&
cp  deploy.sh.in MasterSim.git/release/mac/deploy.sh.in &&
cp  Info.plist.in MasterSim.git/release/mac/Info.plist.in &&
cp -R qt_menu.nib/* MasterSim.git/release/mac/qt_menu.nib/ &&
cp  *.webloc MasterSim.git/release/mac/ &&

# build and deploy

cd MasterSim.git/release/mac &&
python ../scripts/createRelease.py --product MasterSimulator --constants-path ../../MasterSim/src  $*

