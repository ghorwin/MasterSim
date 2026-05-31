#!/bin/bash

# Build script for building application and all dependend libraries

# Command line options:
#   [reldeb|release|debug]      build type
#   [2 [1..n]]                  cpu count
#   [gcc|icc]                   compiler
#   [off|gprof]                 gprof (includes gcc)
#   [off|threadChecker]         threadchecker (includes icc)
#   [off|omp]                   openmp (gcc and icc)
#   [verbose]                   enable cmake to call verbose makefiles
#   [lapack]                    enable cmake to build with lapack support
#   [skip-test]                 does not execute test-init script after successful build
#   [no-gui]                    does not build Qt based libs and user interface
#   [nightly]                   enables nightly build mode (shows startup warning dialog)
#   [deploy]                    set IBK_DEPLOYMENT flag during build

# Qt installed via aqtinstall (see ~/install-qt-6.9.3.sh).
# Override AQT_QT_VERSION / AQT_QT_PREFIX in the environment to point at a different install.
AQT_QT_VERSION="${AQT_QT_VERSION:-6.9.3}"
AQT_QT_PREFIX="${AQT_QT_PREFIX:-$HOME/Qt/${AQT_QT_VERSION}/gcc_64}"

if [ -d "$AQT_QT_PREFIX" ]; then
	echo "Using aqt Qt at $AQT_QT_PREFIX"
	export PATH="$AQT_QT_PREFIX/bin:$PATH"
	export CMAKE_PREFIX_PATH="$AQT_QT_PREFIX${CMAKE_PREFIX_PATH:+:$CMAKE_PREFIX_PATH}"
	export LD_LIBRARY_PATH="$AQT_QT_PREFIX/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
	export QT_PLUGIN_PATH="$AQT_QT_PREFIX/plugins"
	export QML2_IMPORT_PATH="$AQT_QT_PREFIX/qml"
	# bake aqt-Qt into the binary's RPATH so it does not silently fall back to system Qt (Fedora ships 6.11)
	CMAKE_OPTIONS="$CMAKE_OPTIONS -DCMAKE_BUILD_RPATH=$AQT_QT_PREFIX/lib -DCMAKE_INSTALL_RPATH=$AQT_QT_PREFIX/lib -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON"
else
	echo "WARN: $AQT_QT_PREFIX not found, falling back to legacy/system Qt paths"
	# legacy fallback (mac brew + old aqt locations)
	export PATH=~/Qt/5.15.2/gcc_64/bin:~/Qt/5.15.2/clang_64/bin:~/Qt/5.11.3/gcc_64/bin:~/Qt/5.11.3/clang_64/bin:$PATH
fi

CMAKELISTSDIR=$(pwd)/..
BUILDDIR=$(pwd)/"bb"

# set defaults
CMAKE_BUILD_TYPE=" -DCMAKE_BUILD_TYPE:STRING=RelWithDebInfo"
MAKE_CPUCOUNT=$(nproc 2>/dev/null || sysctl -n hw.logicalcpu 2>/dev/null || echo 8)
BUILD_DIR_SUFFIX="gcc"
BUILD_TYPE_SUFFIX="debug"
DEPLOYMENT_SUFFIX=""
QT_VERSION_SUFFIX="-qt"
COMPILER=""
SKIP_TESTS="false"
DISABLE_GUI=0


# parse parameters, except gprof and threadchecker
for var in "$@"
do

	if [[ $var =~ ^[0-9]+$ ]];
	then
		MAKE_CPUCOUNT=$var
		echo "Using $MAKE_CPUCOUNT CPUs for compilation (user-specified)"
	fi

	if [[ $var = "omp"  ]];
	then
		CMAKE_OPTIONS="$CMAKE_OPTIONS -DUSE_OMP:BOOL=ON"
		BUILDDIR=$(pwd)/"bb-omp"
		echo "Using Open MP compile flags"
	fi

	if [[ $var = "nightly" ]];
	then
		CMAKE_OPTIONS="$CMAKE_OPTIONS -DBUILD_NIGHTLY:BOOL=ON"
		echo "Nightly build mode enabled"
	fi

	if [[ $var = "deploy"  ]];
	then
		CMAKE_OPTIONS="$CMAKE_OPTIONS -DIBK_DEPLOYMENT:BOOL=ON"
		DEPLOYMENT_SUFFIX="-deployment"
		echo "Deployment build mode enabled"
	fi

	if [[ $var = "no-gui"  ]];
	then
		DISABLE_GUI=1
	fi

	if [[ $var = "debug"  ]];
	then
		CMAKE_BUILD_TYPE=" -DCMAKE_BUILD_TYPE:STRING=Debug"
		BUILD_TYPE_SUFFIX="debug"
		echo "Debug build..."
	fi

	if [[ $var = "release"  ]];
	then
		CMAKE_BUILD_TYPE=" -DCMAKE_BUILD_TYPE:STRING=Release"
		BUILD_TYPE_SUFFIX="release"
		echo "Release build..."
	fi

	if [[ $var = "reldeb"  ]];
	then
		CMAKE_BUILD_TYPE=" -DCMAKE_BUILD_TYPE:STRING=RelWithDebInfo"
		BUILD_TYPE_SUFFIX="reldeb"
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

#echo $DISABLE_GUI
#echo $CMAKE_OPTIONS

if  [[ $DISABLE_GUI = 1 ]];
then
	CMAKE_OPTIONS="$CMAKE_OPTIONS -DDISABLE_QT:BOOL=ON"
	echo "Disabling Qt libs"
	QT_VERSION_SUFFIX=""
else
	CMAKE_OPTIONS="$CMAKE_OPTIONS -DDISABLE_QT:BOOL=OFF"
	echo "Building with Qt enabled"
fi

# create build dir if not exists (include Qt version suffix)
BUILDDIR=$BUILDDIR-$BUILD_DIR_SUFFIX$QT_VERSION_SUFFIX-$BUILD_TYPE_SUFFIX$DEPLOYMENT_SUFFIX
if [ ! -d $BUILDDIR ]; then
	mkdir -p $BUILDDIR
fi

cd $BUILDDIR && cmake $CMAKE_OPTIONS $CMAKE_BUILD_TYPE $CMAKE_COMPILER_OPTIONS $CMAKELISTSDIR && make -j$MAKE_CPUCOUNT &&
cd $CMAKELISTSDIR &&
mkdir -p bin/release &&
# back to top-level
cd $CMAKELISTSDIR &&
# create top-level dir
mkdir -p bin/release &&
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

