# CMakeLists.txt file for MasterSim library and applications
# Author: Andreas Nicolai (andreas.nicolai -at- tu-dresden.de)

# The project name
project( MasterSimBundle )

# Require a fairly recent cmake version
cmake_minimum_required( VERSION 2.8.12 )

# enable @rpath on MacOS
cmake_policy(SET CMP0042 NEW)

# enable this to test deb-deployment
# use in conjunction with `cmake -DCMAKE_INSTALL_PREFIX=/some/local/path`
# add_definitions( -DIBK_BUILDING_DEBIAN_PACKAGE )

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
endif (UNIX)

# add VC compiler-specific flags (NONMINMAX define and disable "unsafe" warnings)
if (MSVC)
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D\"NOMINMAX\" /wd4996 /GS-" )
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
	find_package(Qt5PrintSupport REQUIRED)

	# when building with gcc/icc add compile flag 
	if (NOT MSVC)
		if (APPLE)
			set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++" )
		else (APPLE)
			set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -std=gnu++11" )
		endif (APPLE)
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
if (NOT UNIX)
    add_subdirectory( ../../externals/zlib/src libz)
endif (NOT UNIX)
add_subdirectory( ../../externals/minizip/projects/cmake_local minizip)
add_subdirectory( ../../externals/IBK/projects/cmake_local IBK )
add_subdirectory( ../../externals/IBKMK/projects/cmake_local IBKMK )
add_subdirectory( ../../externals/BlockMod/projects/cmake BlockMod )
add_subdirectory( ../../externals/TiCPP/projects/cmake_local TiCPP )
add_subdirectory( ../../MasterSim/projects/cmake_local MasterSim)

# -------------------------------------------------------------
# Executables/applications
# -------------------------------------------------------------

add_subdirectory( ../../MasterSimulator/projects/cmake_local MasterSimulator)

if (DEFINED Qt5Widgets_INCLUDE_DIRS OR DEFINED QT4_FOUND )
	add_subdirectory( ../../MasterSimulatorUI/projects/cmake_local MasterSimulatorUI)
endif (DEFINED Qt5Widgets_INCLUDE_DIRS OR DEFINED QT4_FOUND )

# -------------------------------------------------------------
# Test FMUs Libraries
# -------------------------------------------------------------

add_subdirectory( ../../TestFMUs/Math003Part1/projects/cmake Math003Part1)
add_subdirectory( ../../TestFMUs/Math003Part2/projects/cmake Math003Part2)
add_subdirectory( ../../TestFMUs/Math003Part3/projects/cmake Math003Part3)
add_subdirectory( ../../TestFMUs/LotkaVolterraPrey/projects/cmake LotkaVolterraPrey)
add_subdirectory( ../../TestFMUs/LotkaVolterraPredator/projects/cmake LotkaVolterraPredator)
add_subdirectory( ../../TestFMUs/FourRealInputVars/projects/cmake FourRealInputVars)

# Support for 'make install' on Unix/Linux (not on MacOS!)
if (UNIX AND NOT APPLE)

	# -------------------------------------------------------------
	# Generate FMUs to be shipped with examples
	# -------------------------------------------------------------

	# define a command that re-generates the FMUs from the already compiled libraries
	# it depends on the compiled shared libraries
	add_custom_command(
		DEPENDS Math003Part1 Math003Part2 Math003Part3 LotkaVolterraPrey LotkaVolterraPredator
		OUTPUT	${PROJECT_SOURCE_DIR}/../../TestFMUs/fmus/Part1.fmu 
				${PROJECT_SOURCE_DIR}/../../TestFMUs/fmus/Part2.fmu 
				${PROJECT_SOURCE_DIR}/../../TestFMUs/fmus/Part3.fmu
				${PROJECT_SOURCE_DIR}/../../TestFMUs/fmus/Predator.fmu
				${PROJECT_SOURCE_DIR}/../../TestFMUs/fmus/Prey.fmu
				${PROJECT_SOURCE_DIR}/../../TestFMUs/fmus/FourRealInputVars.fmu
		COMMAND  ${PROJECT_SOURCE_DIR}/../../TestFMUs/generate_FMUs.sh ${CMAKE_CURRENT_BINARY_DIR}
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/../../TestFMUs
		COMMENT "Generating example FMUs"
	)
	
	# define a target that is always build - it checks the existence and time-stamp
	# of the example fmu files and if they are missing/out of date it
	# executes the command above to generate/update the FMUs
	add_custom_target(ExampleFMUs ALL
		DEPENDS	${PROJECT_SOURCE_DIR}/../../TestFMUs/fmus/Part1.fmu 
				${PROJECT_SOURCE_DIR}/../../TestFMUs/fmus/Part2.fmu 
				${PROJECT_SOURCE_DIR}/../../TestFMUs/fmus/Part3.fmu
				${PROJECT_SOURCE_DIR}/../../TestFMUs/fmus/Predator.fmu
				${PROJECT_SOURCE_DIR}/../../TestFMUs/fmus/Prey.fmu
				${PROJECT_SOURCE_DIR}/../../TestFMUs/fmus/FourRealInputVars.fmu
	)

	# -------------------------------------------------------------
	# Install example FMUs
	# -------------------------------------------------------------

	# installation targets for Unix systems
	include(GNUInstallDirs)

	# Math_003_control_loop

	set(EXAMPLE_DESTINATION_DIR ${CMAKE_INSTALL_DATAROOTDIR}/mastersim/examples/Math_003_control_loop)
	install(FILES ${PROJECT_SOURCE_DIR}/../../data/examples/linux64/Math_003_control_loop/Math003_GaussSeidel_2iters_adaptive_Richardson.msim
			DESTINATION ${EXAMPLE_DESTINATION_DIR})
	install(FILES ${PROJECT_SOURCE_DIR}/../../data/examples/linux64/Math_003_control_loop/Math003_GaussSeidel_2iters_adaptive_Richardson.bm
			DESTINATION ${EXAMPLE_DESTINATION_DIR})
	install(FILES ${PROJECT_SOURCE_DIR}/../../data/examples/linux64/Math_003_control_loop/Math003_GaussSeidel_2iters_adaptive_Richardson.msim.png
			DESTINATION ${EXAMPLE_DESTINATION_DIR})
	install(FILES ${PROJECT_SOURCE_DIR}/../../data/examples/linux64/Math_003_control_loop/Math003_GaussSeidel_2iters_adaptive_Richardson.p2
			DESTINATION ${EXAMPLE_DESTINATION_DIR})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/../../data/examples/linux64/Math_003_control_loop/description
			DESTINATION ${EXAMPLE_DESTINATION_DIR})
	# we install newly generated FMUs
	install(FILES ${PROJECT_SOURCE_DIR}/../../TestFMUs/fmus/Part1.fmu 
			DESTINATION ${EXAMPLE_DESTINATION_DIR}/fmus/IBK)
	install(FILES ${PROJECT_SOURCE_DIR}/../../TestFMUs/fmus/Part2.fmu 
			DESTINATION ${EXAMPLE_DESTINATION_DIR}/fmus/IBK)
	install(FILES ${PROJECT_SOURCE_DIR}/../../TestFMUs/fmus/Part3.fmu 
			DESTINATION ${EXAMPLE_DESTINATION_DIR}/fmus/IBK)


	# Lotka_Volterra_System

	set(EXAMPLE_DESTINATION_DIR ${CMAKE_INSTALL_DATAROOTDIR}/mastersim/examples/Lotka_Volterra_System)
	install(FILES ${PROJECT_SOURCE_DIR}/../../data/examples/linux64/Lotka_Volterra_System/LV_Richardson_noiter.msim
			DESTINATION ${EXAMPLE_DESTINATION_DIR})
	install(FILES ${PROJECT_SOURCE_DIR}/../../data/examples/linux64/Lotka_Volterra_System/LV_Richardson_noiter.bm
			DESTINATION ${EXAMPLE_DESTINATION_DIR})
	install(FILES ${PROJECT_SOURCE_DIR}/../../data/examples/linux64/Lotka_Volterra_System/LV_Richardson_noiter.msim.png
			DESTINATION ${EXAMPLE_DESTINATION_DIR})
	install(FILES ${PROJECT_SOURCE_DIR}/../../data/examples/linux64/Lotka_Volterra_System/LV_Richardson_noiter.p2
			DESTINATION ${EXAMPLE_DESTINATION_DIR})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/../../data/examples/linux64/Lotka_Volterra_System/description
			DESTINATION ${EXAMPLE_DESTINATION_DIR})

	# we install newly generated FMUs
	install(FILES ${PROJECT_SOURCE_DIR}/../../TestFMUs/fmus/Prey.fmu 
			DESTINATION ${EXAMPLE_DESTINATION_DIR}/fmus/IBK)
	install(FILES ${PROJECT_SOURCE_DIR}/../../TestFMUs/fmus/Predator.fmu 
			DESTINATION ${EXAMPLE_DESTINATION_DIR}/fmus/IBK)

endif (UNIX AND NOT APPLE)

