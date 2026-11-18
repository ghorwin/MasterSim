###########################################################################
#
# Common configuration settings for all libraries and applications of this
# session/repository.
#
###########################################################################
#
# This is the central configuration file for all IBK libraries/applications.
# First define the target type:
#
#   TEMPLATE = app
#
# or
#
#   TEMPLATE = lib
#
# then
#
#   include( <relative-path>/externals/IBK/IBK.pri )
#
# in dependent projects. The include must appear *after* the TEMPLATE line,
# because output paths and library search paths are selected based on the
# template type.
#
# Directory layout created by this file:
#
#   <repo-root>/bin/<debug|release>     all executables and (on Windows) all DLLs
#   <shadow-build-root>/lib             all libraries and Windows import libraries
#
# The library directory lives inside the shadow build directory (as created by
# QtCreator by default, e.g. build/Desktop-Debug), so that libraries of
# different build configurations and toolchains never overwrite each other.
# The binary directory, in contrast, is a fixed path within the source tree, so
# that the relative path from an executable to the development resources is
# always the same, no matter where the shadow build directory is located.
#

# Build-Configurations are stored in CONFIG.pri
!include( CONFIG.pri ){
	message( "No custom build options specified" )
}


# general Qt CONFIG options

CONFIG			+= silent
# Qt 6 requires c++17
CONFIG			+= c++17

CONFIG			+= warn_on
CONFIG			+= no_batch

# enable function/file info also in release mode
DEFINES += QT_MESSAGELOGCONTEXT
# enable asserts also in release mode
DEFINES += QT_FORCE_ASSERTS

CONFIG(release, debug|release) {
#	message( "Setting NDEBUG define" )
	DEFINES += NDEBUG
}

win32 {

	# windows-VC specific options
	win32-msvc* {
		# disable warning for unsafe functions if using MS compiler
		QMAKE_CXXFLAGS += /wd4996
		QMAKE_CFLAGS += /wd4996
		DEFINES += NOMINMAX
		DEFINES += _CRT_SECURE_NO_WARNINGS

		# In Debug mode add warnings for access to uninitialized variables
		# and out-of-bounds access for static arrays (and vectors)
		CONFIG(debug, debug|release) {
			QMAKE_CXXFLAGS += /GS /RTC1
		}
	}
	else {
		#message("Added MinGW Big Files flag")
		QMAKE_CXXFLAGS += -Wa,-mbig-obj
	}
}

win32:LIBS += -liphlpapi
win32:LIBS += -lshell32

# Linux/Mac/GCC specific options
linux-g++ | linux-g++-64 | macx {

	# our code doesn't check errno after calling math functions
	# so it is perfectly safe to disable it in favor of better performance
	# use *= to uniquely assign option
	QMAKE_CXXFLAGS   *= -fno-math-errno

	# create "Position Independent Code"
	QMAKE_CXXFLAGS   *= -fPIC
}


#
# code sanitizer checks
#
contains( OPTIONS, sanitize_checks ) {

	CONFIG(debug, debug|release) {
		CONFIG += sanitizer
		CONFIG += sanitize_address
		CONFIG += sanitize_undefined
	}

	linux-g++ | linux-g++-64 | macx {
		QMAKE_CXXFLAGS_DEBUG   *= -fsanitize=address -fno-omit-frame-pointer
	}
}


#
# openmp settings added for gcc and min-gw
#
contains( OPTIONS, openmp ) {

	message(Setting up OpenMP support)

	#setup linker and compiler flags
	CONFIG(debug, debug|release) {
		QMAKE_CFLAGS = -fopenmp -fPIC -march=core-avx-i -mtune=core-avx-i
	} else {
		QMAKE_CFLAGS = -fopenmp -fPIC -O3 -march=core-avx-i -mtune=core-avx-i #-Ofast
	}
	QMAKE_LFLAGS = $$QMAKE_CFLAGS
	QMAKE_CXXFLAGS = $$QMAKE_CFLAGS
	QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS

	# all OpenMP implementations define a _OPENMP symbol
}
else {

#	message(No OpenMP support)
	unix {
#		message(OpenMP warnings disabled)
		QMAKE_CFLAGS += -Wno-unknown-pragmas
		QMAKE_CXXFLAGS += -Wno-unknown-pragmas
	}

}


#
# gprof settings added for gcc and min-gw
#
contains( OPTIONS, gprof ) {

	message(Setting up gprof support.)

	#setup linker and compiler flags
	QMAKE_CFLAGS += -pg
	QMAKE_LFLAGS += $$QMAKE_CFLAGS
	QMAKE_CXXFLAGS += $$QMAKE_CFLAGS
	QMAKE_CXXFLAGS_RELEASE += $$QMAKE_CFLAGS

}


#
# Output directories
#

# Top-level build directory, i.e. the shadow build directory that corresponds to the
# root directory of the session, for example <repo-root>/build/Desktop-Debug.
# Note: $$PWD is always <repo-root>/externals/IBK, no matter which pro-file includes
#       this file, hence $$PWD/../.. is always the root directory of the session.
#       For non-shadow-builds $$shadowed() simply returns the source directory itself.
BUILD_ROOT = $$shadowed($$PWD/../..)

# Directory that collects the libraries of all sub-projects. Since it lives inside the
# shadow build directory, debug/release (and different toolchains) are separated
# automatically without any further subdirectories.
LIB_DIR = $$BUILD_ROOT/lib

# Directory that collects the executables of all sub-projects, and on Windows also all
# DLLs. In contrast to LIB_DIR this is a fixed location in the source tree, so binaries
# built by different toolchains overwrite each other - this is intentional, see the
# comment at the top of this file.
CONFIG(debug, debug|release) {
	contains( OPTIONS, binaries_to_shadow_dir ) {
		BINARYDESTDIR = $$BUILD_ROOT/bin/debug
	}
	else {
		BINARYDESTDIR = $$PWD/../../bin/debug
	}
}
else {
	contains( OPTIONS, binaries_to_shadow_dir ) {
		BINARYDESTDIR = $$BUILD_ROOT/bin/release
	}
	else {
		BINARYDESTDIR = $$PWD/../../bin/release
	}
}

# Relative subdirectories for the generated intermediate files. Since these paths are
# relative, they are resolved within the shadow build directory of the individual
# sub-project, so generated files of different sub-projects never collide.
OBJECTS_DIR = obj
MOC_DIR = moc
RCC_DIR = rcc
UI_DIR = ui


#
# *** Libraries ***
#
# This section contains all library specific settings. It is enabled
# by setting 'lib' as qmake project TEMPLATE.
equals( TEMPLATE, lib ) {

#	message(Setting up ordinary library support.)
	QT -=	core gui

	# MIND: MS compiler needs explicit export statements in case of shared libs, hence such
	#       libraries must also set the defines that enable their XXX_EXPORT macros, and
	#       all projects using the DLL must set the corresponding import define.

	win32 {
		# On Windows, we build static by default. 
		# If you really need shared libs on Windows, manually override this setting after 
		# the pri-include with:
		#  CONFIG -= static
		#  CONFIG += shared
		# 
		CONFIG += static
	}
	else {
		# on Unix/MacOS we always build our libraries as dynamic libs
		# we override this in libraries that are meant for static building
		CONFIG += shared
	}

	DESTDIR = $$LIB_DIR

	# on Windows the DLLs are placed next to the executables that use them
	# (ignored by qmake on all other platforms)
	DLLDESTDIR = $$BINARYDESTDIR

	# our libraries link against each other, so they need the library search path, too
	LIBS += -L$$LIB_DIR

	# tells the dynamic linker where to find the dependent libraries at runtime;
	# RUNPATH is not searched transitively, hence each library needs its own entry
	unix {
		QMAKE_RPATHDIR += $$LIB_DIR
	}

	#message("DESTDIR    =" $$DESTDIR)
	#message("DLLDESTDIR =" $$DLLDESTDIR)
}


#
# *** Applications ***
#
# This section contains all application specific settings. It is enabled
# by setting 'app' as qmake project TEMPLATE.
# It defines DESTDIR and the library search paths.
else:equals( TEMPLATE, app ) {

	DESTDIR = $$BINARYDESTDIR

	# library search path used when linking against the libraries of this session
	LIBS += -L$$LIB_DIR

	# tells the dynamic linker where to find the shared libraries at runtime, so that
	# the executable can be started without adding the library directory to
	# LD_LIBRARY_PATH (or DYLD_FALLBACK_LIBRARY_PATH on MacOS)
	unix {
		QMAKE_RPATHDIR += $$LIB_DIR
	}

	#message("DESTDIR    =" $$DESTDIR)
}
