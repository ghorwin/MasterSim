#!/bin/bash


# Build script for building 'MasterSimulator' and all dependend libraries

# Command line options:
#   [reldeb|release|debug]		build type
#   [2 [1..n]]					cpu count
#   [gcc|icc]					compiler
#   [off|gprof]					gprof (includes gcc)
#   [off|threadChecker]			threadchecker (includes icc)
#   [off|omp]					openmp (gcc and icc)
#   [verbose]					enable cmake to call verbose makefiles
#   [lapack]					enable cmake to build with lapack support
#   [skip-test]					does not execute test script after successful build
#   [skip-man]					skips generation of man pages
#   []

# path export for mac
export PATH=~/Qt/5.11.3/gcc_64/bin:~/Qt/5.11.3/clang_64/bin:$PATH

CMAKELISTSDIR=$(pwd)/../..
BUILDDIR=$(pwd)/bb

# set defaults
CMAKE_BUILD_TYPE=" -DCMAKE_BUILD_TYPE:STRING=RelWithDebInfo"
MAKE_CPUCOUNT="2"
BUILD_DIR_SUFFIX="gcc"
COMPILER=""
SKIP_TESTS="false"
SKIP_MANPAGE="false"

# parse parameters, except gprof and threadchecker
for var in "$@"
do

	if [[ $var = "--help" ]];
	then
		echo "Command line options:"
		echo "  [reldeb|release|debug]      build type"
		echo "  [2 [1..n]]                  cpu count"
		echo "  [gcc|icc]                   compiler"
		echo "  [off|gprof]                 gprof (includes gcc)"
		echo "  [off|threadChecker]         threadchecker (includes icc)"
		echo "  [off|omp]                   openmp (gcc and icc)"
		echo "  [verbose]                   enable cmake to call verbose makefiles"
		echo "  [skip-test]                 does not execute test script after successful build"

		exit
	fi

    if [[ $var = *[[:digit:]]* ]];
    then
		MAKE_CPUCOUNT=$var
		echo "Using $MAKE_CPUCOUNT CPUs for compilation"
    fi

    if [[ $var = "omp"  ]];
    then
		CMAKE_OPTIONS="$CMAKE_OPTIONS -DUSE_OMP:BOOL=ON"
		echo "Using Open MP compile flags"
    fi

    if [[ $var = "debug"  ]];
    then
		CMAKE_BUILD_TYPE=" -DCMAKE_BUILD_TYPE:STRING=Debug"
		echo "Debug build..."
    fi

    if [[ $var = "release"  ]];
    then
		CMAKE_BUILD_TYPE=" -DCMAKE_BUILD_TYPE:STRING=Release"
		echo "Release build..."
    fi

    if [[ $var = "reldeb"  ]];
    then
		CMAKE_BUILD_TYPE=" -DCMAKE_BUILD_TYPE:STRING=RelWithDebInfo"
		echo "RelWithDebInfo build..."
    fi

    if [[ $var = "icc"  && $COMPILER = "" ]];
    then
		COMPILER="icc"
		BUILD_DIR_SUFFIX="icc"
		echo "Intel compiler build..."
	    # export intel compiler path
	    CMAKE_COMPILER_OPTIONS="-DCMAKE_C_COMPILER=icc -DCMAKE_CXX_COMPILER=icc"
	  fi

    if [[ $var = "gcc"  && $COMPILER = "" ]];
    then
		COMPILER="gcc"
		BUILD_DIR_SUFFIX="gcc"
		echo "GCC compiler build..."
		CMAKE_COMPILER_OPTIONS=""
	  fi

    if [[ $var = "verbose"  ]];
  	then
		CMAKE_OPTIONS="$CMAKE_OPTIONS -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON"
	  fi

	if [[ $var = "skip-test"  ]];
	then
		SKIP_TESTS="true"
	fi

	if [[ $var = "skip-man"  ]];
	then
		SKIP_MANPAGE="true"
	fi

	if [[ $var = "lapack"  ]];
	then
		CMAKE_OPTIONS="$CMAKE_OPTIONS -DLAPACK_ENABLE:BOOL=ON"
		echo "Building with lapack support for CVODE"
	fi

done

# override compiler options
for var in "$@"
do

    if [[ $var = "gprof" ]];
    then
		COMPILER="gcc"
		BUILD_DIR_SUFFIX="gcc"
		CMAKE_COMPILER_OPTIONS="-DCMAKE_CXX_FLAGS="'-pg'" -DCMAKE_EXE_LINKER_FLAGS="'-pg'""
		echo "Gprof build, forcing GCC build..."
    fi

    if [[ $var = "threadChecker"  ]];
    then
		COMPILER="icc"
		BUILD_DIR_SUFFIX="icc"
		echo "Using Threadchecker, forcing Intel compiler build..."
	    # export intel compiler path
	    CMAKE_COMPILER_OPTIONS="-DCMAKE_C_COMPILER=icc -DCMAKE_CXX_COMPILER=icc -DUSE_THREAD_CHECKER:BOOL=ON"
	fi

done


# create build dir if not exists
BUILDDIR=$BUILDDIR-$BUILD_DIR_SUFFIX
if [ ! -d $BUILDDIR ]; then
    mkdir -p $BUILDDIR
fi

cd $BUILDDIR && cmake $CMAKE_OPTIONS $CMAKE_BUILD_TYPE $CMAKE_COMPILER_OPTIONS $CMAKELISTSDIR && make -j$MAKE_CPUCOUNT &&

# back to top-level
cd $CMAKELISTSDIR &&
# create top-level dir
mkdir -p $CMAKELISTSDIR/bin/release &&
echo "*** Copying mastersim to bin/release ***" &&
cp $BUILDDIR/MasterSimulator/mastersim $CMAKELISTSDIR/bin/release/mastersim && 
if [[ $SKIP_MANPAGE = "false"  ]]; then
  echo "  ** Generating man page mastersim.1 **"
  $CMAKELISTSDIR/bin/release/mastersim --man-page > $CMAKELISTSDIR/MasterSimulator/doc/mastersim.1
fi &&

# UI only exists when Qt is enabled
if [ -e $BUILDDIR/MasterSimulatorUI/mastersim-gui ]; then
  echo "*** Copying mastersim-gui to bin/release ***" &&
  cp $BUILDDIR/MasterSimulatorUI/mastersim-gui $CMAKELISTSDIR/bin/release/mastersim-gui &&
  # next call may fail on GitHub actions, so we do not require this to succeed
  if [[ $SKIP_MANPAGE = "false"  ]]; then
	echo "  ** Generating man page mastersim-gui.1 **"
	$CMAKELISTSDIR/bin/release/mastersim-gui --man-page > $CMAKELISTSDIR/MasterSimulatorUI/doc/mastersim-gui.1
  fi
fi &&

# UI on Mac only exists when Qt is enabled and buiding on Mac
if [ -e $BUILDDIR/MasterSimulatorUI/mastersim-gui.app ]; then
  # remove potentially existing app bundle
  if [ -e $CMAKELISTSDIR/bin/release/MasterSim.app ]; then
    rm -rf $CMAKELISTSDIR/bin/release/MasterSim.app
  fi &&
  echo "*** Copying MasterSimulatorUI.app to bin/release ***" &&
  cp -r $BUILDDIR/MasterSimulatorUI/mastersim-gui.app $CMAKELISTSDIR/bin/release/MasterSim.app
fi &&
cd $BUILDDIR/.. &&

echo "*** Build MasterSim ***" &&
if [[ $SKIP_TESTS = "false"  ]]; then
./run_tests.sh
fi
