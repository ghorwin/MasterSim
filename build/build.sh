#!/bin/bash

# Build script for MasterSim, its user interface, the test FMUs and all dependent libraries

# Command line options:
#   [reldeb|release|debug]		build type
#   [2 [1..n]]					cpu count
#   [off|gprof]					gprof build
#   [off|omp]					openmp
#   [verbose]					enable cmake to call verbose makefiles
#   [deployment|deploy]			set DEPLOYMENT (and IBK_DEPLOYMENT) flag during build
#   [nightly]					enables nightly build mode (shows startup warning dialog)
#   [debian]					set IBK_BUILDING_DEBIAN_PACKAGE flag (FHS install layout, implies deployment)
#   [skip-test]					do not run the test suite after a successful build
#   [no-gui]					do not build the Qt based libraries and user interface
#
# Executables are placed in <repo-root>/bin/release (or bin/debug for debug builds),
# libraries in the 'lib' subdirectory of the build directory (see externals/IBK/build.cmake).

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
CMAKELISTSDIR="$(cd -- "${SCRIPT_DIR}/.." && pwd)"
BUILDDIR="${SCRIPT_DIR}/${MASTERSIM_BUILD_DIR_PREFIX:-bb}"

# Qt installed via aqtinstall (see ~/install-qt-6.9.3.sh) or the Qt online installer.
# Override AQT_QT_VERSION / AQT_QT_PREFIX in the environment to point at a different install.
# The Qt kit directory depends on the platform: gcc_64 on Linux, macos (Qt 6) or clang_64 (older
# installs) on macOS.
AQT_QT_VERSION="${AQT_QT_VERSION:-6.9.3}"
if [[ -z "$AQT_QT_PREFIX" ]]; then
	if [[ "$(uname)" = "Darwin" ]]; then
		QT_KIT_CANDIDATES="macos clang_64"
	else
		QT_KIT_CANDIDATES="gcc_64"
	fi
	for kit in $QT_KIT_CANDIDATES; do
		AQT_QT_PREFIX="$HOME/Qt/${AQT_QT_VERSION}/$kit"
		if [ -d "$AQT_QT_PREFIX" ]; then
			break
		fi
	done
fi

# Locate cmake: use the one in PATH, otherwise fall back to common install locations
# (the CMake shipped with the Qt installer, the CMake.app bundle and Homebrew on macOS).
# Override with the CMAKE environment variable.
if [[ -z "$CMAKE" ]]; then
	if command -v cmake >/dev/null 2>&1; then
		CMAKE=cmake
	else
		for candidate in \
			"$HOME/Qt/Tools/CMake/bin/cmake" \
			"$HOME/Qt/Tools/CMake/CMake.app/Contents/bin/cmake" \
			"/Applications/CMake.app/Contents/bin/cmake" \
			"/opt/homebrew/bin/cmake" \
			"/usr/local/bin/cmake"
		do
			if [ -x "$candidate" ]; then
				CMAKE="$candidate"
				break
			fi
		done
	fi
	if [[ -z "$CMAKE" ]]; then
		echo "ERROR: cmake not found. Install cmake, add it to PATH or set the CMAKE environment variable." >&2
		exit 1
	fi
fi
echo "Using cmake: $CMAKE"

# set defaults
CMAKE_BUILD_TYPE=" -DCMAKE_BUILD_TYPE:STRING=Release"
MAKE_CPUCOUNT=$(nproc 2>/dev/null || sysctl -n hw.logicalcpu 2>/dev/null || echo 8)
BUILD_DIR_SUFFIX="gcc"
BUILD_TYPE_SUFFIX="release"
DEPLOYMENT_SUFFIX=""
QT_SUFFIX="-qt"
CMAKE_COMPILER_OPTIONS=""
CMAKE_OPTIONS=""
USE_OMP="OFF"
DEPLOYMENT="OFF"
NIGHTLY="OFF"
DEBIAN_PACKAGE="OFF"
VERBOSE_MAKEFILE="OFF"
GPROF=0
SKIP_TESTS=0
DISABLE_QT="OFF"

USAGE="Usage: $(basename "$0") [reldeb|release|debug] [<cpu count>] [gprof] [omp] [verbose] [deployment|deploy] [debian] [nightly] [skip-test] [no-gui]"

# parse parameters
for var in "$@"
do

	if [[ $var =~ ^[0-9]+$ ]];
	then
		MAKE_CPUCOUNT=$var
		echo "Using $MAKE_CPUCOUNT CPUs for compilation (user-specified)"

	elif [[ $var = "omp" ]];
	then
		USE_OMP="ON"
		BUILDDIR=$BUILDDIR"-parallel"
		echo "Using Open MP compile flags"

	elif [[ $var = "deployment" || $var = "deploy" ]];
	then
		DEPLOYMENT="ON"
		# a debian build keeps its own build directory suffix
		if [[ $DEBIAN_PACKAGE = "OFF" ]]; then
			DEPLOYMENT_SUFFIX="-deployment"
		fi
		echo "Enabling deployment mode"

	elif [[ $var = "debian" ]];
	then
		DEBIAN_PACKAGE="ON"
		DEPLOYMENT="ON"
		DEPLOYMENT_SUFFIX="-debian"
		echo "Building for Debian package (implies deployment mode)"

	elif [[ $var = "nightly" ]];
	then
		NIGHTLY="ON"
		echo "Nightly build mode enabled"

	elif [[ $var = "debug" ]];
	then
		CMAKE_BUILD_TYPE=" -DCMAKE_BUILD_TYPE:STRING=Debug"
		BUILD_TYPE_SUFFIX="debug"
		echo "Debug build..."

	elif [[ $var = "release" ]];
	then
		CMAKE_BUILD_TYPE=" -DCMAKE_BUILD_TYPE:STRING=Release"
		BUILD_TYPE_SUFFIX="release"
		echo "Release build..."

	elif [[ $var = "reldeb" ]];
	then
		CMAKE_BUILD_TYPE=" -DCMAKE_BUILD_TYPE:STRING=RelWithDebInfo"
		BUILD_TYPE_SUFFIX="reldeb"
		echo "RelWithDebInfo build..."

	elif [[ $var = "gprof" ]];
	then
		GPROF=1

	elif [[ $var = "verbose" ]];
	then
		VERBOSE_MAKEFILE="ON"

	elif [[ $var = "skip-test" ]];
	then
		SKIP_TESTS=1

	elif [[ $var = "no-gui" ]];
	then
		DISABLE_QT="ON"
		QT_SUFFIX=""
		echo "Building without GUI (MasterSimulatorUI and the Qt based libraries)"

	else
		echo "Unknown command line option '$var'" >&2
		echo "$USAGE" >&2
		exit 1
	fi

done


# setup Qt installed via aqtinstall, if available
if [[ $DISABLE_QT = "OFF" ]];
then
	if [ -d "$AQT_QT_PREFIX" ]; then
		echo "Using aqt Qt at $AQT_QT_PREFIX"
		export PATH="$AQT_QT_PREFIX/bin:$PATH"
		export CMAKE_PREFIX_PATH="$AQT_QT_PREFIX${CMAKE_PREFIX_PATH:+:$CMAKE_PREFIX_PATH}"
		# NOTE: the build RPATH generated by CMake already contains the Qt library directory,
		#       for installed binaries we add it explicitly so that they do not silently fall
		#       back to the system Qt
		CMAKE_OPTIONS="$CMAKE_OPTIONS -DCMAKE_INSTALL_RPATH=$AQT_QT_PREFIX/lib"
	else
		echo "NOTE: $AQT_QT_PREFIX not found, using Qt from CMAKE_PREFIX_PATH/system"
	fi
fi


# gprof builds get their own build directory, so that the -pg flags cannot leak
# into the cache of a regular build
if [[ $GPROF = 1 ]];
then
	BUILD_DIR_SUFFIX="gcc-gprof"
	CMAKE_COMPILER_OPTIONS="-DCMAKE_CXX_FLAGS=-pg -DCMAKE_EXE_LINKER_FLAGS=-pg"
	echo "Gprof build..."
fi

# all cached options are passed explicitly with ON/OFF, so that switching an
# option off again is not silently ignored by an already populated cmake cache
CMAKE_OPTIONS="$CMAKE_OPTIONS -DUSE_OMP:BOOL=$USE_OMP"
CMAKE_OPTIONS="$CMAKE_OPTIONS -DDEPLOYMENT:BOOL=$DEPLOYMENT"
CMAKE_OPTIONS="$CMAKE_OPTIONS -DBUILD_NIGHTLY:BOOL=$NIGHTLY"
CMAKE_OPTIONS="$CMAKE_OPTIONS -DIBK_BUILDING_DEBIAN_PACKAGE:BOOL=$DEBIAN_PACKAGE"
CMAKE_OPTIONS="$CMAKE_OPTIONS -DCMAKE_VERBOSE_MAKEFILE:BOOL=$VERBOSE_MAKEFILE"
CMAKE_OPTIONS="$CMAKE_OPTIONS -DDISABLE_QT:BOOL=$DISABLE_QT"


# create build dir if not exists
BUILDDIR=$BUILDDIR-$BUILD_DIR_SUFFIX$QT_SUFFIX-$BUILD_TYPE_SUFFIX$DEPLOYMENT_SUFFIX
if [ ! -d $BUILDDIR ]; then
	mkdir -p $BUILDDIR
fi &&

cd $BUILDDIR && "$CMAKE" $CMAKE_OPTIONS $CMAKE_BUILD_TYPE $CMAKE_COMPILER_OPTIONS $CMAKELISTSDIR && make -j$MAKE_CPUCOUNT &&
cd $SCRIPT_DIR &&
echo "*** Success ***" || exit 1

# The test suite runs the solver in bin/release, hence tests are only run for non-debug builds.
if [[ $SKIP_TESTS = 0 ]];
then
	if [[ $BUILD_TYPE_SUFFIX = "debug" ]];
	then
		echo "Debug build: skipping test suite (tests use bin/release/mastersim)"
	else
		./run_tests.sh
	fi
fi
