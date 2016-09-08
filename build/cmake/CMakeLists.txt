# CMakeLists.txt file for MasterSim library and applications
# Author: Andreas Nicolai (andreas.nicolai -at- tu-dresden.de)

cmake_minimum_required( VERSION 2.6 )

# The project name
project( MasterSimBundle )

# -------------------------------------------------------------
# Initial commands
# -------------------------------------------------------------

# Uncomment this to enable detailed make output
# set( CMAKE_VERBOSE_MAKEFILE ON )

# Set release mode as default, except for the Borland Compiler
if (NOT CMAKE_BUILD_TYPE)
	set( CMAKE_BUILD_TYPE Release CACHE STRING
		"Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif (NOT CMAKE_BUILD_TYPE)

# on Unix we want really detailed warnings
if (UNIX)
	ADD_DEFINITIONS( -Wall -fPIC)
endif (UNIX)

# for VC compiler, add the flag to disable wchar_t as native type
if (MSVC)
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:wchar_t-" )

	set( CMAKE_PREFIX_PATH "C:\\Qt\\Qt5.7.0_VC10\\msvc2010\\")
endif (MSVC)

# -------------------------------------------------------------
# Required packages
# -------------------------------------------------------------

# automatically add CMAKE_CURRENT_SOURCE_DIR and CMAKE_CURRENT_BINARY_DIR to the include directories in every processed CMakeLists.txt
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Test for Qt5
find_package(Qt5Widgets)

if (Qt5Widgets_INCLUDE_DIRS)
	message("*** Building with Qt5 ***")

	find_package(Qt5Xml REQUIRED)
	find_package(Qt5Svg REQUIRED)
	find_package(Qt5Network REQUIRED)

else (Qt5Widgets_INCLUDE_DIRS)

	# not found, fall back to Qt4
	message("*** Building with Qt4 ***")

	# We need Qt for building
	find_package( Qt4  REQUIRED )

	set( QT_USE_QTXML TRUE )
	set( QT_USE_QTSVG TRUE )
	set( QT_USE_QTNETWORK TRUE )
	include( ${QT_USE_FILE} )

endif (Qt5Widgets_INCLUDE_DIRS)

if( APPLE )
	find_library(IOKIT NAMES IOKit REQUIRED)
	find_library(SYSTEM_CONFIGURATION SystemConfiguration REQUIRED)
	find_library(COREFOUNDATION NAMES CoreFoundation REQUIRED)
	find_library(SECURITY_FRAMEWORK Security REQUIRED)

	set(APPLE_FRAMEWORKS 
		${IOKIT}
		${SECURITY_FRAMEWORK}
		${COREFOUNDATION}
		${SYSTEM_CONFIGURATION}
	)
endif( APPLE )

# -------------------------------------------------------------
# Support for build flags
# -------------------------------------------------------------
include( ../../externals/IBK/projects/cmake_local/cmake_pri/CMakeLists.txt )

if (MINGW)
	# link statically to MinGW libraries
	#set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE} " -static-libgcc -static-libstdc++" )
	add_definitions(-static-libgcc -static-libstdc++)
endif (MINGW)

if (MSVC)
	add_definitions(/wd"4996")

endif (MSVC)

# -------------------------------------------------------------
# Subdirectories and subprojects
# -------------------------------------------------------------

# add subdirectories for all builds
add_subdirectory( ../../externals/zlib/src libz)
add_subdirectory( ../../externals/minizip/projects/cmake_local minizip)
add_subdirectory( ../../externals/IBK/projects/cmake_local IBK )
add_subdirectory( ../../externals/IBKMK/projects/cmake_local IBKMK )
add_subdirectory( ../../externals/BlockMod/projects/cmake BlockMod )
add_subdirectory( ../../externals/DataIO/projects/cmake_local DataIO)
add_subdirectory( ../../externals/TiCPP/projects/cmake_local TiCPP )
add_subdirectory( ../../MasterSim/projects/cmake_local MasterSim)

# -------------------------------------------------------------
# Executables/applications
# -------------------------------------------------------------

add_subdirectory( ../../MasterSimulator/projects/cmake_local MasterSimulator)

if (DEFINED Qt5Widgets_INCLUDE_DIRS OR DEFINED QT4_FOUND )
	add_subdirectory( ../../MasterSimulatorUI/projects/cmake_local MasterSimulatorUI)
endif (DEFINED Qt5Widgets_INCLUDE_DIRS OR DEFINED QT4_FOUND )

