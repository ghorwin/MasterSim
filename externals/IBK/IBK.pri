###########################################################################
#
# Common configuration settings for all derived libraries and applications.
#
###########################################################################

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
#   include( ../externals/IBK/IBK.pri )
#
# in dependent projects, but mind the directory structure.

# Build-Configurations are stored in CONFIG.pri
!include( CONFIG.pri ){
	message( "No custom build options specified" )
}


# general Qt CONFIG options

CONFIG			+= silent
greaterThan(QT_MAJOR_VERSION, 5) {
	# Qt requires c++17 itself
	CONFIG			+= c++17
}
else {
	# our code requites at least C++11
	CONFIG			+= c++11
}
CONFIG			+= warn_on
CONFIG			+= no_batch

# enable function/file info also in release mode
DEFINES += QT_MESSAGELOGCONTEXT

CONFIG(release, debug|release) {
#	message( "Setting NDEBUG define" )
	DEFINES += NDEBUG
}


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
# openmp settings added for gcc and min-gw
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
# Output directories for temporary files
# Note: for shadow builds, the generated files end up in configuration-specific directory trees anyway,
#       so that we wouldn't need to distinguish between release/debug subdirs. However, if used
#       for non-shadow-builds, we need to separate debug/release mode files.
#
CONFIG(debug, debug|release) {
	OBJECTS_DIR = debug/obj
	MOC_DIR = debug/moc
	RCC_DIR = debug/rcc
	UI_DIR = debug/ui
	contains( OPTIONS, binaries_to_shadow_dir ) {
		BINARYDESTDIR = ../bin/debug
	}
	else {
		BINARYDESTDIR = $$PWD/../../bin/debug
	}
}
else {
	OBJECTS_DIR = release/obj
	MOC_DIR = release/moc
	RCC_DIR = release/rcc
	UI_DIR = release/ui
	contains( OPTIONS, binaries_to_shadow_dir ) {
		BINARYDESTDIR = ../bin/release
	}
	else {
		BINARYDESTDIR = $$PWD/../../bin/release
	}
}


#
# *** Applications ***
#
# This section contains all application specific settings. It can be enabled
# by setting 'app' the TEMPLATE environment variable.
# It defines DESTDIR and library search paths.
equals(TEMPLATE,app) {
	DESTDIR = $$BINARYDESTDIR

	CONFIG(debug, debug|release) {
		QMAKE_LIBDIR += ../externals/lib/debug ../lib/debug
		LIBS += -L../lib/debug -L../externals/lib/debug
	}
	else {
		QMAKE_LIBDIR += ../externals/lib/release ../lib/release
		LIBS += -L../lib/release -L../externals/lib/release
	}

}


#
# *** Libraries ***
#
# This section contains all library specific settings. It can be enabled
# by setting 'lib' as qmake project TEMPLATE
equals(TEMPLATE,lib) {

#	message(Setting up ordinary library support.)
	QT -=	core gui

	# MIND: MS compiler needs explicit export statements in case of shared libs
	#       Shared libs should provide dll_import.pri files on Windows for libraries that use
	#       the DLLs and import symbols.
	win32-msvc* {
		# if you really need shared libs on Windows, manually override this setting after the pri-include
		CONFIG += static
	}
	else {
		# on Unix/MacOS we always build our libraries as dynamic libs
		CONFIG += shared
	}

	CONFIG(debug, debug|release) {
		DESTDIR = ../lib/debug
		# Note: when library is located top-level, ie. <repo-root>/Library/...
		#       we need to set the linker path such, that it finds other libraries in <repo-root>/externals/lib
		contains( OPTIONS, top_level_libs ) {
			LIBS += -L../externals/lib/debug
			QMAKE_LIBDIR += ../externals/lib/debug
			windows {
				# place windows DLLs into executable target directory
				DLLDESTDIR = $$BINARYDESTDIR
			}
		}
		else {
			QMAKE_LIBDIR += ../lib/debug
			LIBS += -L../lib/debug
			windows {
				# place windows DLLs into executable target directory (two levels up)
				DLLDESTDIR = ../$$BINARYDESTDIR
			}
		}

	}
	else {
		DESTDIR = ../lib/release
		contains( OPTIONS, top_level_libs ) {
			LIBS += -L../externals/lib/release
			QMAKE_LIBDIR += ../externals/lib/release
			windows {
				# place windows DLLs into executable target directory
				DLLDESTDIR = $$BINARYDESTDIR
			}
		}
		else {
			QMAKE_LIBDIR += ../lib/release
			LIBS += -L../lib/release
			windows {
				# place windows DLLs into executable target directory (two levels up)
				DLLDESTDIR = ../$$BINARYDESTDIR
			}
		}
	}


}
