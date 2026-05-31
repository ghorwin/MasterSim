#!/bin/bash

# Execute in top-level directory, i.e. 
#
# > scripts/update_source_code_archive.sh

rm -rf mastersim-*/ > /dev/null

echo "Removed all previous files in directory."
echo "Press any key to continue or CTRL+C to abort!"
read -t 10 -n 1

# clone/update current source repository
echo "*** STEP 1 : cloning MasterSim.git ***" &&

if [ ! -d "MasterSim-git" ]; then
  echo "Cloning github repo"
  git clone https://github.com/ghorwin/MasterSim.git MasterSim-git
else
  echo "Reverting local changes and pulling newest revisions from github"
  (cd MasterSim-git && git reset --hard HEAD && git clean -fdx && git pull --rebase)
fi &&
du -h --summarize MasterSim-git/  && 

# extract version number
VERSION=$(python3 scripts/extractVersion.py MasterSim-git/MasterSim/src/MSIM_Constants.cpp) &&
TARGETDIR=mastersim-$VERSION &&

# copy directory
echo "*** STEP 2 : Creating src directory $TARGETDIR ***" &&
if [ ! -d $TARGETDIR ]; then
	mkdir $TARGETDIR
fi &&
rsync -a --delete --exclude=".*" MasterSim-git/ $TARGETDIR/ && 
du -h --summarize $TARGETDIR/ &&

# step 3 - remove unwanted files that should not go into the source code archive
#          end help reducing archive size

echo "*** STEP 3 : Cleaning out source directory ***" &&
rm -rf $TARGETDIR/third-party &&
rm -rf $TARGETDIR/doc &&
mv $TARGETDIR/data/examples/linux64 $TARGETDIR/examples &&
rm -rf $TARGETDIR/data &&
mkdir --parents $TARGETDIR/data/examples/ &&
mv $TARGETDIR/examples $TARGETDIR/data/examples/linux64  &&
rm -rf $TARGETDIR/cross-check &&
rm -rf $TARGETDIR/externals/zlib &&
du -h --summarize $TARGETDIR/ &&

echo "*** STEP 4 : Editing top-level CMakeLists.txt file ***" &&

sed -i '20i ADD_DEFINITIONS( -DIBK_BUILDING_DEBIAN_PACKAGE )' $TARGETDIR/CMakeLists.txt &&

echo "*** STEP 5 : Creating source code archive ***" &&

# create tar.gz or tar.xz (the latter has better compression)
# tar -czvf mastersim_$VERSION.orig.tar.gz $TARGETDIR/
tar cf - $TARGETDIR/ | xz -z - > mastersim_$VERSION.orig.tar.xz &&
du -h --summarize mastersim_$VERSION.orig.tar.xz
