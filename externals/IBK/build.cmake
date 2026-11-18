# Common build configuration, included by all library and application CMake files.
#
# Defines the common output directories and, for applications, the search path
# needed to link against the shared libraries built in this repository.
#
# Usage (in a CMakeLists.txt):    
#   include( ${CMAKE_CURRENT_LIST_DIR}/<relative path>/libs/UitCore/build.cmake )
#
# CAUTION: The files IBK/build.cmake and UitCore/build.cmake are identical.
#          Use either file as include in your top-level CMakeLists.txt.

# The output directory variables below have directory scope and are read when a
# target is created, so this file has to be included by every CMakeLists.txt
# that creates a target - guard per directory, not globally.
#include_guard(DIRECTORY)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Libraries are placed in the 'lib' subdirectory of the build directory, so
# that all generated files of a build configuration stay together and the
# source tree remains clean. With one build directory per configuration, e.g.
# build/Desktop-Debug, the libraries end up in build/Desktop-Debug/lib.
# This way different build configurations (debug/release) can coexist
# without overwriting each other's libraries.
#
# Executables are placed in bin/debug or bin/release.

# repository root; this file lives in <root>/libs/UitCore, so its own directory 
# is the anchor, it does not depend on which CMakeLists.txt includes this file
get_filename_component(SOURCE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

# directories holding all shared libraries and all executables built in this
# repository; the generator expression selects the executable directory by
# configuration and, as a side effect, suppresses the per-configuration
# subdirectory that multi-config generators would append
set(LIB_DIR "${CMAKE_BINARY_DIR}/lib")
set(BIN_DIR "${SOURCE_ROOT}/bin/$<IF:$<CONFIG:Debug>,debug,release>")

# ARCHIVE covers static libraries and the Windows import libraries, LIBRARY the
# shared libraries on unix. RUNTIME covers the executables and, on Windows, the
# DLLs - which is what DLLDESTDIR achieves in build.pri.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${LIB_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${LIB_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BIN_DIR}")

# Set deployment directory
set(DEPLOYMENT_DIR "${CMAKE_BINARY_DIR}/deployment")

# detect if we are using a multi-config generator
get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

# tell the dynamic linker where to find the shared libraries at runtime
# this way the binary can be run without having to add the library dest dir
# to LD_LIBRARY_PATH
if (UNIX)
	if (IS_MULTI_CONFIG)
		# multi-config generators append the configuration name to the plain
		# (generator expression free) library output directory
		set(CMAKE_BUILD_RPATH "${LIB_DIR}/$<CONFIG>")
	else()
		set(CMAKE_BUILD_RPATH "${LIB_DIR}")
	endif()
endif()


# **** Our speciel settings ****

# bildet das qmake-Standardverhalten nach: ohne explizit angeforderte
# Konfiguration wird ein Release-Build erzeugt
if (NOT IS_MULTI_CONFIG AND NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Build config (Debug or Release)" FORCE)
endif()

# Release-Konfiguration
if( CMAKE_BUILD_TYPE STREQUAL "Release" )
	add_compile_definitions(QT_FORCE_ASSERTS)
	# NOTE: DEPLOYMENT wird via CMake-Kommandozeile gesetzt, nicht hier:
	# cmake -DCMAKE_BUILD_TYPE=Release -DDEPLOYMENT=ON ...
endif()


# -------------------------------------------------------------
# Options
# -------------------------------------------------------------

option( USE_OMP "Turns on OpenMP parallelization." OFF )
option( DEPLOYMENT "If enabled, the paths to database are generated differently based on binary file location." OFF )
option( DISABLE_QT "Disables building of Qt based libraries and UI (command line solver only)." OFF )

if( NOT DISABLE_QT )
	# run the Qt code generators automatically for all targets, this replaces the
	# handling that qmake does implicitly for SOURCES/HEADERS/RESOURCES/FORMS
	# NOTE: we guard this behind the DISABLE_QT to avoid error messages because of missing Qt components
	set(CMAKE_AUTOMOC ON)
	set(CMAKE_AUTORCC ON)
	set(CMAKE_AUTOUIC ON)
endif()

# if we build in deployment mode, we set out define DEPLOYMENT
# just like if we would write #define DEPLOYMENT at the head of each .h/.cpp file
if( DEPLOYMENT )
	message("*** DEPLOYMENT MODE ***")
	add_compile_definitions( DEPLOYMENT )
endif( DEPLOYMENT )

# Support for OpenMP
if( USE_OMP )
	find_package( OpenMP REQUIRED)
	set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif( USE_OMP )


# -------------------------------------------------------------
# Compiler/tool chain specific stuff
# -------------------------------------------------------------

# Enable Position-Independent Code for shared libraries
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Visual Studio Compiler specific options
if( MSVC )

	# add VC compiler-specific flags:
	# - NONMINMAX define -> prevent compile errors when using std::min or std::max
	# - /Zc:__cplusplus  -> let VC report the correct C++ version in macro __cplusplus 
	# - /wd4996          -> disable "unsafe" CRT warnings
	# - /permissive-     -> disable Microsoft C++ extension (needed in addition to 'set(CMAKE_CXX_EXTENSIONS OFF)')
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D\"NOMINMAX\" /wd4996 /Zc:__cplusplus  /utf-8 /permissive-" )

endif( MSVC )

if (WIN32)
	# on Windows we dont want the lib prefix
	set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif()


# on Unix we want really detailed warnings
if (UNIX)
	add_compile_options( -Wall )
endif (UNIX)
