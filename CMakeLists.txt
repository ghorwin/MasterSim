# CMakeLists.txt file for MasterSim library and applications
# Author: Andreas Nicolai (andreas.nicolai -at- gmx.net)

# Require a fairly recent cmake version
cmake_minimum_required( VERSION 3.16...3.31 )

# The project name
project( MasterSimBundle )

# enable @rpath on MacOS
cmake_policy(SET CMP0042 NEW)

cmake_policy(SET CMP0072 NEW)

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

# Nightly build mode - shows startup warning dialog
option(BUILD_NIGHTLY "Build as nightly version with startup warning" OFF)
if(BUILD_NIGHTLY)
	add_definitions(-DNIGHTLY_BUILD)
endif()

# Deployment build mode
option(IBK_DEPLOYMENT "Build in deployment mode" OFF)
if(IBK_DEPLOYMENT)
	add_definitions(-DIBK_DEPLOYMENT)
endif()

# -----------------------
# Support for build flags
# -----------------------
include( externals/IBK/CMakeLists.pri )

# -------------------------------------------------------------
# Required packages
# -------------------------------------------------------------

# automatically add CMAKE_CURRENT_SOURCE_DIR and CMAKE_CURRENT_BINARY_DIR to the include directories in every processed CMakeLists.txt
set(CMAKE_INCLUDE_CURRENT_DIR ON)


if (DISABLE_QT)
	message("*** Building without Qt ***")
else (DISABLE_QT)

	message("*** Building with Qt ***")

	# C++17 needed for Qt6
	set( CMAKE_CXX_STANDARD 17 )
	set( CMAKE_CXX_STANDARD_REQUIRED ON )
	find_package(Qt6 REQUIRED COMPONENTS Core Widgets Xml Svg Network PrintSupport)

	set( CMAKE_AUTOMOC ON )
	set( CMAKE_AUTORCC ON )
	set( CMAKE_AUTOUIC ON )

endif (DISABLE_QT)

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
    add_subdirectory( externals/zlib/src libz)
endif (NOT UNIX)
add_subdirectory( externals/minizip minizip)
add_subdirectory( externals/IBK IBK )
add_subdirectory( externals/IBKMK IBKMK )
add_subdirectory( externals/TiCPP TiCPP )
add_subdirectory( MasterSim )

if (NOT DISABLE_QT)
	message("*** Building Qt-based libs ***")
	add_subdirectory( externals/BlockMod BlockMod )
endif (NOT DISABLE_QT)

# -------------------------------------------------------------
# Executables/applications
# -------------------------------------------------------------

add_subdirectory( MasterSimulator )

if (NOT DISABLE_QT)
	add_subdirectory( MasterSimulatorUI )
endif (NOT DISABLE_QT)

# -------------------------------------------------------------
# Test FMUs Libraries
# -------------------------------------------------------------

add_subdirectory( TestFMUs/Math003Part1 )
add_subdirectory( TestFMUs/Math003Part2 )
add_subdirectory( TestFMUs/Math003Part3 )
add_subdirectory( TestFMUs/LotkaVolterraPrey )
add_subdirectory( TestFMUs/LotkaVolterraPredator )
add_subdirectory( TestFMUs/FourRealInputVars )

# Support for 'make install' on Unix/Linux (not on MacOS!)
if (UNIX AND NOT APPLE)

	# -------------------------------------------------------------
	# Generate FMUs to be shipped with examples
	# -------------------------------------------------------------

	# define a command that re-generates the FMUs from the already compiled libraries
	# it depends on the compiled shared libraries
	add_custom_command(
		DEPENDS Math003Part1 Math003Part2 Math003Part3 LotkaVolterraPrey LotkaVolterraPredator FourRealInputVars
		OUTPUT	${PROJECT_SOURCE_DIR}/TestFMUs/fmus/Part1.fmu 
				${PROJECT_SOURCE_DIR}/TestFMUs/fmus/Part2.fmu 
				${PROJECT_SOURCE_DIR}/TestFMUs/fmus/Part3.fmu
				${PROJECT_SOURCE_DIR}/TestFMUs/fmus/Predator.fmu
				${PROJECT_SOURCE_DIR}/TestFMUs/fmus/Prey.fmu
				${PROJECT_SOURCE_DIR}/TestFMUs/fmus/FourRealInputVars.fmu
		COMMAND  ${PROJECT_SOURCE_DIR}/TestFMUs/generate_FMUs.sh ${CMAKE_CURRENT_BINARY_DIR}
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/TestFMUs
		COMMENT "Generating example FMUs"
	)
	
	# define a target that is always build - it checks the existence and time-stamp
	# of the example fmu files and if they are missing/out of date it
	# executes the command above to generate/update the FMUs
	add_custom_target(ExampleFMUs ALL
		DEPENDS	${PROJECT_SOURCE_DIR}/TestFMUs/fmus/Part1.fmu 
				${PROJECT_SOURCE_DIR}/TestFMUs/fmus/Part2.fmu 
				${PROJECT_SOURCE_DIR}/TestFMUs/fmus/Part3.fmu
				${PROJECT_SOURCE_DIR}/TestFMUs/fmus/Predator.fmu
				${PROJECT_SOURCE_DIR}/TestFMUs/fmus/Prey.fmu
				${PROJECT_SOURCE_DIR}/TestFMUs/fmus/FourRealInputVars.fmu
	)

	# -------------------------------------------------------------
	# Install example FMUs
	# -------------------------------------------------------------

	# installation targets for Unix systems
	include(GNUInstallDirs)

	# Math_003_control_loop

	set(EXAMPLE_DESTINATION_DIR ${CMAKE_INSTALL_DATAROOTDIR}/mastersim/examples/Math_003_control_loop)
	install(FILES ${PROJECT_SOURCE_DIR}/data/examples/linux64/Math_003_control_loop/Math003_GaussSeidel_2iters_adaptive_Richardson.msim
			DESTINATION ${EXAMPLE_DESTINATION_DIR})
	install(FILES ${PROJECT_SOURCE_DIR}/data/examples/linux64/Math_003_control_loop/Math003_GaussSeidel_2iters_adaptive_Richardson.bm
			DESTINATION ${EXAMPLE_DESTINATION_DIR})
	install(FILES ${PROJECT_SOURCE_DIR}/data/examples/linux64/Math_003_control_loop/Math003_GaussSeidel_2iters_adaptive_Richardson.msim.png
			DESTINATION ${EXAMPLE_DESTINATION_DIR})
	install(FILES ${PROJECT_SOURCE_DIR}/data/examples/linux64/Math_003_control_loop/Math003_GaussSeidel_2iters_adaptive_Richardson.p2
			DESTINATION ${EXAMPLE_DESTINATION_DIR})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/data/examples/linux64/Math_003_control_loop/description
			DESTINATION ${EXAMPLE_DESTINATION_DIR})
	# we install newly generated FMUs
	install(FILES ${PROJECT_SOURCE_DIR}/TestFMUs/fmus/Part1.fmu 
			DESTINATION ${EXAMPLE_DESTINATION_DIR}/fmus/IBK)
	install(FILES ${PROJECT_SOURCE_DIR}/TestFMUs/fmus/Part2.fmu 
			DESTINATION ${EXAMPLE_DESTINATION_DIR}/fmus/IBK)
	install(FILES ${PROJECT_SOURCE_DIR}/TestFMUs/fmus/Part3.fmu 
			DESTINATION ${EXAMPLE_DESTINATION_DIR}/fmus/IBK)


	# Lotka_Volterra_System

	set(EXAMPLE_DESTINATION_DIR ${CMAKE_INSTALL_DATAROOTDIR}/mastersim/examples/Lotka_Volterra_System)
	install(FILES ${PROJECT_SOURCE_DIR}/data/examples/linux64/Lotka_Volterra_System/LV_Richardson_noiter.msim
			DESTINATION ${EXAMPLE_DESTINATION_DIR})
	install(FILES ${PROJECT_SOURCE_DIR}/data/examples/linux64/Lotka_Volterra_System/LV_Richardson_noiter.bm
			DESTINATION ${EXAMPLE_DESTINATION_DIR})
	install(FILES ${PROJECT_SOURCE_DIR}/data/examples/linux64/Lotka_Volterra_System/LV_Richardson_noiter.msim.png
			DESTINATION ${EXAMPLE_DESTINATION_DIR})
	install(FILES ${PROJECT_SOURCE_DIR}/data/examples/linux64/Lotka_Volterra_System/LV_Richardson_noiter.p2
			DESTINATION ${EXAMPLE_DESTINATION_DIR})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/data/examples/linux64/Lotka_Volterra_System/description
			DESTINATION ${EXAMPLE_DESTINATION_DIR})

	# we install newly generated FMUs
	install(FILES ${PROJECT_SOURCE_DIR}/TestFMUs/fmus/Prey.fmu 
			DESTINATION ${EXAMPLE_DESTINATION_DIR}/fmus/IBK)
	install(FILES ${PROJECT_SOURCE_DIR}/TestFMUs/fmus/Predator.fmu 
			DESTINATION ${EXAMPLE_DESTINATION_DIR}/fmus/IBK)

endif (UNIX AND NOT APPLE)

