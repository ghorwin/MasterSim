# CMakeLists.txt file for MasterSim library and applications
# Author: Andreas Nicolai (andreas.nicolai -at- tu-dresden.de)

# The project name
project( MasterSimBundle )

# Require a fairly recent cmake version
cmake_minimum_required( VERSION 2.8.12 )

# -------------------------------------------------------------
# MACRO definitions
# -------------------------------------------------------------

# Macros to hide/show cached variables.
# These two macros can be used to "hide" or "show" in the
# list of cached variables various variables and/or options 
# that depend on other options.
# Note that once a variable is modified, it will preserve its
# value (hidding it merely makes it internal)

MACRO(HIDE_VARIABLE var)
  IF(DEFINED ${var})
    SET(${var} "${${var}}" CACHE INTERNAL "")
  ENDIF(DEFINED ${var})
ENDMACRO(HIDE_VARIABLE)

MACRO(SHOW_VARIABLE var type doc default)
  IF(DEFINED ${var})
    SET(${var} "${${var}}" CACHE "${type}" "${doc}" FORCE)
  ELSE(DEFINED ${var})
    SET(${var} "${default}" CACHE "${type}" "${doc}")
  ENDIF(DEFINED ${var})
ENDMACRO(SHOW_VARIABLE)

# -------------------------------------------------------------
# Initial commands
# -------------------------------------------------------------

# Uncomment this to enable detailed make output
#set( CMAKE_VERBOSE_MAKEFILE ON )

# Hide some more cache variables to keep things tidy
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)
MARK_AS_ADVANCED(EXECUTABLE_OUTPUT_PATH LIBRARY_OUTPUT_PATH)

# Set default build type
if (NOT CMAKE_BUILD_TYPE)
	set( CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
		"Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif (NOT CMAKE_BUILD_TYPE)

# on Unix we want really detailed warnings
if (UNIX)
	ADD_DEFINITIONS( -Wall -fPIC )
	if (CMAKE_COMPILER_IS_GNUCC)
		ADD_DEFINITIONS( -static-libgcc -static-libstdc++ )
	else (CMAKE_COMPILER_IS_GNUCC)
		ADD_DEFINITIONS( -static )
	endif (CMAKE_COMPILER_IS_GNUCC)
endif (UNIX)

# add VC compiler-specific flags (NONMINMAX define and disable "unsafe" warnings)
if (MSVC)
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D\"NOMINMAX\" /wd4996" )
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

	# when building with gcc/icc add compile flag
	if (NOT MSVC)
		set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )
	endif (NOT MSVC)

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

