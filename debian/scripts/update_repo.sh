#!/bin/bash

if [ ! -d "MasterSim-git" ]; then
  echo "Cloning github repo"
  git clone https://github.com/ghorwin/MasterSim.git MasterSim-git
else
  echo "Reverting local changes and pulling newest revisions from github"
  (cd MasterSim-git; git reset --hard HEAD; git pull --rebase)
fi
