# CMakeLists.txt file for MasterSim library, applications and test FMUs

cmake_minimum_required( VERSION 3.16 )

# -------------------------------------------------------------
# macOS: universal binaries (Apple Silicon + Intel)
#
# MasterSim and the test FMUs are built for arm64 and x86_64. A process can only
# load FMU binaries of its own architecture, so an x86_64 slice of the solver is
# needed to run x86_64-only FMUs on Apple Silicon (via Rosetta 2: 'arch -x86_64 mastersim')
# and to support Intel Macs. FMI 1.0/2.0 has only one macOS platform directory
# (binaries/darwin64), hence the test FMUs contain universal dylibs as well.
#
# Must be set before project(). Use -DMSIM_UNIVERSAL_BINARIES=OFF to build only for
# the host architecture, or set CMAKE_OSX_ARCHITECTURES explicitly.
# All dependencies (Qt, zlib) must be universal as well; the official Qt binaries
# and the SDK zlib are, Homebrew libraries usually are not.
# -------------------------------------------------------------
if( APPLE )
	option( MSIM_UNIVERSAL_BINARIES "Build universal (arm64 + x86_64) binaries on macOS" ON )
	if( MSIM_UNIVERSAL_BINARIES AND NOT CMAKE_OSX_ARCHITECTURES )
		set( CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "macOS target architectures" FORCE )
	endif()
	if( NOT CMAKE_OSX_DEPLOYMENT_TARGET AND NOT DEFINED ENV{MACOSX_DEPLOYMENT_TARGET} )
		# Qt 6.9 requires macOS 12; the solver alone runs on macOS 11 (first version with arm64 support)
		if( DISABLE_QT )
			set( CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version" FORCE )
		else()
			set( CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum macOS version" FORCE )
		endif()
	endif()
endif()

project( MasterSimBundle )

# -------------------------------------------------------------
# Support for common cmake configuration
#
# Sets the C++ standard, the output directories (lib/ and
# bin/<config>), the rpath, the MSVC compiler flags and the
# common options (USE_OMP, DEPLOYMENT, DISABLE_QT) and enables
# the Qt code generators (AUTOMOC/AUTOUIC/AUTORCC).
# -------------------------------------------------------------
include( externals/IBK/build.cmake )


# -------------------------------------------------------------
# MasterSim specific options
# -------------------------------------------------------------

# Nightly build mode - shows startup warning dialog
option( BUILD_NIGHTLY "Build as nightly version with startup warning" OFF )
if( BUILD_NIGHTLY )
	add_compile_definitions( NIGHTLY_BUILD )
endif()

# The MasterSim sources check for IBK_DEPLOYMENT rather than DEPLOYMENT
if( DEPLOYMENT )
	add_compile_definitions( IBK_DEPLOYMENT )
endif()

# Disable fused multiply-add contraction (default of clang/gcc on arm64), so that results
# are identical on all CPU architectures and match the regression test reference results.
if( CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" )
	add_compile_options( -ffp-contract=off )
endif()

# enable this to test deb-deployment
# use in conjunction with `cmake -DCMAKE_INSTALL_PREFIX=/some/local/path`
# add_compile_definitions( IBK_BUILDING_DEBIAN_PACKAGE )

# -------------------------------------------------------------
# Packages
# -------------------------------------------------------------

if (DISABLE_QT)
	message("*** Building without Qt ***")
else (DISABLE_QT)

	find_package( Qt6 COMPONENTS Core REQUIRED )

	message("*** Building with Qt, Version ${Qt6Core_VERSION} ***")
	set(QT_VERSION_MAJOR 6)

endif (DISABLE_QT)

if( APPLE )
	find_library(IOKIT NAMES IOKit REQUIRED)
	find_library(COREFOUNDATION NAMES CoreFoundation REQUIRED)
	find_library(SECURITY_FRAMEWORK Security REQUIRED)
	find_library(SYSTEM_CONFIGURATION SystemConfiguration REQUIRED)

	set(APPLE_FRAMEWORKS
		${IOKIT}
		${SECURITY_FRAMEWORK}
		${COREFOUNDATION}
		${SYSTEM_CONFIGURATION}
	)
endif( APPLE )

# -------------------------------------------------------------
# subdirectories and subprojects
# -------------------------------------------------------------

# add subdirectories for all builds
add_subdirectory( externals/IBK )
add_subdirectory( externals/IBKMK )
add_subdirectory( externals/minizip )
add_subdirectory( externals/TiCPP )
add_subdirectory( MasterSim )

if (NOT DISABLE_QT)
	message("*** Building Qt-based libs ***")

	add_subdirectory( externals/BlockMod )
	add_subdirectory( externals/phantom-style )
endif (NOT DISABLE_QT)

# -------------------------------------------------------------
# applications/executables
# -------------------------------------------------------------

add_subdirectory( MasterSimulator )

if (NOT DISABLE_QT)
	add_subdirectory( MasterSimulatorUI )
endif (NOT DISABLE_QT)

# -------------------------------------------------------------
# Test FMUs
# -------------------------------------------------------------

add_subdirectory( TestFMUs/Math003Part1 )
add_subdirectory( TestFMUs/Math003Part2 )
add_subdirectory( TestFMUs/Math003Part3 )
add_subdirectory( TestFMUs/LotkaVolterraPrey )
add_subdirectory( TestFMUs/LotkaVolterraPredator )
add_subdirectory( TestFMUs/FourRealInputVars )
add_subdirectory( TestFMUs/Room )
add_subdirectory( TestFMUs/Radiator )

# -------------------------------------------------------------
# Generate FMUs (test cases and examples) on Linux and macOS
# -------------------------------------------------------------
if (UNIX)

	set( EXAMPLE_FMU_DIR ${PROJECT_SOURCE_DIR}/TestFMUs/fmus )
	set( EXAMPLE_FMUS
		${EXAMPLE_FMU_DIR}/Part1.fmu
		${EXAMPLE_FMU_DIR}/Part2.fmu
		${EXAMPLE_FMU_DIR}/Part3.fmu
		${EXAMPLE_FMU_DIR}/Predator.fmu
		${EXAMPLE_FMU_DIR}/Prey.fmu
		${EXAMPLE_FMU_DIR}/FourRealInputVars.fmu
		${EXAMPLE_FMU_DIR}/Room.fmu
		${EXAMPLE_FMU_DIR}/Radiator.fmu
	)

	# define a command that re-generates the FMUs from the already compiled libraries
	# it depends on the compiled shared libraries, which are placed in LIB_DIR (see build.cmake)
	add_custom_command(
		DEPENDS Math003Part1 Math003Part2 Math003Part3 LotkaVolterraPrey LotkaVolterraPredator FourRealInputVars Room Radiator
		OUTPUT  ${EXAMPLE_FMUS}
		COMMAND ${PROJECT_SOURCE_DIR}/TestFMUs/generate_FMUs.sh ${LIB_DIR}
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/TestFMUs
		COMMENT "Generating example FMUs"
	)

	# define a target that is always build - it checks the existence and time-stamp
	# of the example fmu files and if they are missing/out of date it
	# executes the command above to generate/update the FMUs
	add_custom_target( ExampleFMUs ALL
		DEPENDS	${EXAMPLE_FMUS}
	)

endif (UNIX)

# Support for 'make install' on Unix/Linux (not on MacOS!)
if (UNIX AND NOT APPLE)

	# -------------------------------------------------------------
	# Install example FMUs
	# -------------------------------------------------------------

	# installation targets for Unix systems
	include(GNUInstallDirs)

	# Math_003_control_loop

	set( EXAMPLE_SOURCE_DIR ${PROJECT_SOURCE_DIR}/data/examples/linux64/Math_003_control_loop )
	set( EXAMPLE_DESTINATION_DIR ${CMAKE_INSTALL_DATAROOTDIR}/mastersim/examples/Math_003_control_loop )
	install( FILES
			${EXAMPLE_SOURCE_DIR}/Math003_GaussSeidel_2iters_adaptive_Richardson.msim
			${EXAMPLE_SOURCE_DIR}/Math003_GaussSeidel_2iters_adaptive_Richardson.bm
			${EXAMPLE_SOURCE_DIR}/Math003_GaussSeidel_2iters_adaptive_Richardson.msim.png
			${EXAMPLE_SOURCE_DIR}/Math003_GaussSeidel_2iters_adaptive_Richardson.p2
			DESTINATION ${EXAMPLE_DESTINATION_DIR} )
	install( DIRECTORY ${EXAMPLE_SOURCE_DIR}/description
			DESTINATION ${EXAMPLE_DESTINATION_DIR} )
	# we install newly generated FMUs
	install( FILES
			${EXAMPLE_FMU_DIR}/Part1.fmu
			${EXAMPLE_FMU_DIR}/Part2.fmu
			${EXAMPLE_FMU_DIR}/Part3.fmu
			DESTINATION ${EXAMPLE_DESTINATION_DIR}/fmus/IBK )

	# Lotka_Volterra_System

	set( EXAMPLE_SOURCE_DIR ${PROJECT_SOURCE_DIR}/data/examples/linux64/Lotka_Volterra_System )
	set( EXAMPLE_DESTINATION_DIR ${CMAKE_INSTALL_DATAROOTDIR}/mastersim/examples/Lotka_Volterra_System )
	install( FILES
			${EXAMPLE_SOURCE_DIR}/LV_Richardson_noiter.msim
			${EXAMPLE_SOURCE_DIR}/LV_Richardson_noiter.bm
			${EXAMPLE_SOURCE_DIR}/LV_Richardson_noiter.msim.png
			${EXAMPLE_SOURCE_DIR}/LV_Richardson_noiter.p2
			DESTINATION ${EXAMPLE_DESTINATION_DIR} )
	install( DIRECTORY ${EXAMPLE_SOURCE_DIR}/description
			DESTINATION ${EXAMPLE_DESTINATION_DIR} )
	# we install newly generated FMUs
	install( FILES
			${EXAMPLE_FMU_DIR}/Prey.fmu
			${EXAMPLE_FMU_DIR}/Predator.fmu
			DESTINATION ${EXAMPLE_DESTINATION_DIR}/fmus/IBK )

endif (UNIX AND NOT APPLE)
