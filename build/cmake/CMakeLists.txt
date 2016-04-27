# CMakeLists.txt file
# Copyright Andreas Nicolai (andreas.nicolai -at- tu-dresden.de)

# The project name
project( MasterSim )

# -------------------------------------------------------------
# Initial commands
# -------------------------------------------------------------

# Require a fairly recent cmake version
cmake_minimum_required( VERSION 2.4.3 )

# set the policy to the new variant
if (COMMAND cmake_policy)
    cmake_policy(VERSION 2.6)
endif (COMMAND cmake_policy)

# Uncomment this to enable detailed make output
# set( CMAKE_VERBOSE_MAKEFILE ON )


# Hide some more cache variables to keep things tidy
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)
MARK_AS_ADVANCED(EXECUTABLE_OUTPUT_PATH LIBRARY_OUTPUT_PATH)

# Set release mode as default, except for the Borland Compiler
if (NOT CMAKE_BUILD_TYPE)
	set( CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
		"Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif (NOT CMAKE_BUILD_TYPE)

# on Unix we want really detailed warnings
if (UNIX)
	ADD_DEFINITIONS( -Wall )
endif (UNIX)


# -------------------------------------------------------------
# Required packages
# -------------------------------------------------------------

# need Qt4 for user interface
FIND_PACKAGE( Qt4 )

if( QT4_FOUND )

	set( QT_USE_QTXML TRUE )
	include( ${QT_USE_FILE} )

endif( QT4_FOUND )

find_package(Boost 1.54.0 COMPONENTS)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS}) 
endif(Boost_FOUND)

# -------------------------------------------------------------
# Support for build flags
# -------------------------------------------------------------
include( ../../externals/IBK/projects/cmake_local/cmake_pri/CMakeLists.txt )

# -------------------------------------------------------------
# Subdirectories and subprojects
# -------------------------------------------------------------

# add subdirectories for all builds
add_subdirectory( ../../externals/zlib/src libz)
add_subdirectory( ../../externals/minizip/projects/cmake_local minizip)
add_subdirectory( ../../externals/IBK/projects/cmake_local IBK )
add_subdirectory( ../../externals/DataIO/projects/cmake_local DataIO)
add_subdirectory( ../../MasterSim/projects/cmake_local MasterSim)

# -------------------------------------------------------------
# Executables/applications
# -------------------------------------------------------------

add_subdirectory( ../../MasterSimulator/projects/cmake_local MasterSimulator)

if( QT4_FOUND )
	add_subdirectory( ../../MasterSimulatorUI/projects/cmake_local MasterSimulatorUI)
endif( QT4_FOUND )

