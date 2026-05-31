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
if [ ! -d "MasterSim.git/release/linux" ]; then
  mkdir MasterSim.git/release/linux
fi && 

cp -R ../scripts/* MasterSim.git/release/scripts/ && 
cp deploy.sh.in MasterSim.git/release/linux/deploy.sh.in && 
cp readme.txt MasterSim.git/release/linux/readme.txt && 

# build and deploy

cd MasterSim.git/release/linux && 
python3 ../scripts/createRelease.py --product MasterSimulator --constants-path ../../MasterSim/src  $* &&
cd - > /dev/null &&

# move software archive to current working directory
mv -f MasterSim.git/release/linux/*.7z .


