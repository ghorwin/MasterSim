# CMakeLists.pri - Shared CMake configuration for VICUS project
#
# This file contains common settings for all libraries and applications.
# Include this file in each CMakeLists.txt to ensure consistent configuration.

# Prevent multiple inclusions
if(VICUS_CMAKE_COMMON_INCLUDED)
    return()
endif()
set(VICUS_CMAKE_COMMON_INCLUDED TRUE)

# -------------------------------------------------------------
# Qt Version Detection and Configuration
# -------------------------------------------------------------

# User-configurable Qt version selection:
#   -DVICUS_QT_VERSION=5    Force Qt5 (fails if not found)
#   -DVICUS_QT_VERSION=6    Force Qt6 (fails if not found)
#   (not set)               Auto-detect: try Qt6 first, then Qt5
#
# The selected Qt version is stored in VICUS_QT_VERSION_DETECTED after detection.

# Macro to find Qt packages (works with both Qt5 and Qt6)
# Usage: vicus_find_qt_packages(Widgets Xml Svg ...)
macro(vicus_find_qt_packages)
    set(_qt_packages ${ARGN})

    # Determine which Qt version(s) to try based on user input
    set(_try_qt6 FALSE)
    set(_try_qt5 FALSE)
    set(_require_specific FALSE)

    if(DEFINED VICUS_QT_VERSION)
        # User explicitly specified a Qt version
        if(VICUS_QT_VERSION EQUAL 6)
            set(_try_qt6 TRUE)
            set(_require_specific TRUE)
            message(STATUS "Qt version explicitly set to Qt6")
        elseif(VICUS_QT_VERSION EQUAL 5)
            set(_try_qt5 TRUE)
            set(_require_specific TRUE)
            message(STATUS "Qt version explicitly set to Qt5")
        else()
            message(FATAL_ERROR "Invalid VICUS_QT_VERSION=${VICUS_QT_VERSION}. Use 5 or 6, or leave unset for auto-detection.")
        endif()
    else()
        # Auto-detect: try Qt6 first, then Qt5
        set(_try_qt6 TRUE)
        set(_try_qt5 TRUE)
        message(STATUS "Qt version not specified, auto-detecting (trying Qt6 first)")
    endif()

    set(_qt_found FALSE)

    # Try Qt6 if requested
    if(_try_qt6 AND NOT _qt_found)
		# search first only for component Core
        find_package(Qt6 QUIET COMPONENTS Core)
        if(Qt6_FOUND)
			find_package(Qt6 QUIET COMPONENTS ${_qt_packages})
            # Verify all requested components are available
            set(_missing_components "")
            foreach(_pkg ${_qt_packages})
                if(NOT TARGET Qt6::${_pkg})
                    list(APPEND _missing_components ${_pkg})
                endif()
            endforeach()

            if(_missing_components)
                if(_require_specific)
                    message(FATAL_ERROR "Qt6 found but missing required components: ${_missing_components}")
                else()
                    message(STATUS "Qt6 found but missing components: ${_missing_components}, trying Qt5")
                endif()
            else()
                set(_qt_found TRUE)
                set(VICUS_QT_VERSION 6 CACHE INTERNAL "Qt version in use")
                set(VICUS_QT_PREFIX "Qt6" CACHE INTERNAL "Qt package prefix")
                message(STATUS "Found Qt6, Version ${Qt6_VERSION}")

                # Set compatibility variables for include directories
                foreach(_pkg ${_qt_packages})
                    if(TARGET Qt6::${_pkg})
                        get_target_property(_inc Qt6::${_pkg} INTERFACE_INCLUDE_DIRECTORIES)
                        set(Qt6${_pkg}_INCLUDE_DIRS ${_inc} CACHE INTERNAL "Qt6 ${_pkg} include dirs")
                    endif()
                endforeach()
                # Also set Core and Gui includes (needed for basic Qt headers)
                if(TARGET Qt6::Core)
                    get_target_property(_inc Qt6::Core INTERFACE_INCLUDE_DIRECTORIES)
                    set(Qt6Core_INCLUDE_DIRS ${_inc} CACHE INTERNAL "Qt6 Core include dirs")
                endif()
                if(TARGET Qt6::Gui)
                    get_target_property(_inc Qt6::Gui INTERFACE_INCLUDE_DIRECTORIES)
                    set(Qt6Gui_INCLUDE_DIRS ${_inc} CACHE INTERNAL "Qt6 Gui include dirs")
                endif()
            endif()
        else()
            if(_require_specific)
                message(FATAL_ERROR "Qt6 not found. Please install Qt6 development packages or use -DVICUS_QT_VERSION=5 for Qt5.")
            else()
                message(STATUS "Qt6 not found, trying Qt5")
            endif()
        endif()
    endif()

    # Try Qt5 if requested and Qt6 wasn't found/used
    if(_try_qt5 AND NOT _qt_found)
        find_package(Qt5 QUIET COMPONENTS Core)
        if(Qt5_FOUND)
			find_package(Qt5 QUIET COMPONENTS ${_qt_packages})
            # Verify all requested components are available
            set(_missing_components "")
            foreach(_pkg ${_qt_packages})
                if(NOT TARGET Qt5::${_pkg})
                    list(APPEND _missing_components ${_pkg})
                endif()
            endforeach()

            if(_missing_components)
                if(_require_specific)
                    message(FATAL_ERROR "Qt5 found but missing required components: ${_missing_components}")
                else()
                    message(FATAL_ERROR "Qt5 found but missing components: ${_missing_components}. Neither Qt5 nor Qt6 have all required modules.")
                endif()
            else()
                set(_qt_found TRUE)
                set(VICUS_QT_VERSION 5 CACHE INTERNAL "Qt version in use")
                set(VICUS_QT_PREFIX "Qt5" CACHE INTERNAL "Qt package prefix")
                message(STATUS "Found Qt5, Version ${Qt5_VERSION}")
            endif()
        else()
            if(_require_specific)
                message(FATAL_ERROR "Qt5 not found. Please install Qt5 development packages or use -DVICUS_QT_VERSION=6 for Qt6.")
            endif()
        endif()
    endif()

    # Final check
    if(NOT _qt_found)
        message(FATAL_ERROR "Neither Qt5 nor Qt6 found with all required components (${_qt_packages}). Please install Qt development packages.")
    endif()
endmacro()

# Macro to add Qt resources (Qt5/Qt6 compatible)
macro(vicus_add_qt_resources OUT_VAR)
    if(VICUS_QT_VERSION EQUAL 6)
        qt6_add_resources(${OUT_VAR} ${ARGN})
    else()
        qt5_add_resources(${OUT_VAR} ${ARGN})
    endif()
endmacro()

# Macro to wrap UI files (Qt5/Qt6 compatible)
macro(vicus_wrap_ui OUT_VAR)
    if(VICUS_QT_VERSION EQUAL 6)
        qt6_wrap_ui(${OUT_VAR} ${ARGN})
    else()
        qt5_wrap_ui(${OUT_VAR} ${ARGN})
    endif()
endmacro()

# Macro to link Qt libraries (Qt5/Qt6 compatible)
macro(vicus_target_link_qt TARGET_NAME)
    foreach(_component ${ARGN})
        if(VICUS_QT_VERSION EQUAL 6)
            target_link_libraries(${TARGET_NAME} PUBLIC Qt6::${_component})
        else()
            target_link_libraries(${TARGET_NAME} PUBLIC Qt5::${_component})
        endif()
    endforeach()
endmacro()

# Macro to get Qt include directories
macro(vicus_get_qt_include_dirs OUT_VAR)
    set(${OUT_VAR} "")
    foreach(_component ${ARGN})
        if(VICUS_QT_VERSION EQUAL 6)
            if(TARGET Qt6::${_component})
                get_target_property(_inc Qt6::${_component} INTERFACE_INCLUDE_DIRECTORIES)
                list(APPEND ${OUT_VAR} ${_inc})
            endif()
        else()
            list(APPEND ${OUT_VAR} ${Qt5${_component}_INCLUDE_DIRS})
        endif()
    endforeach()
endmacro()

# -------------------------------------------------------------
# Platform-Specific Compiler Configuration
# -------------------------------------------------------------

# on Unix and Apple we want really detailed warnings
if (UNIX)
	# C++17 standard for Linux builds
	set(CMAKE_CXX_STANDARD 17)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
	set(CMAKE_CXX_EXTENSIONS OFF)
	# all warnings
	ADD_DEFINITIONS( -Wall )
	# our code does not check for errno values, so we can skip this feature
	ADD_DEFINITIONS( -fno-math-errno )
	# create position independent code (needed for dynamic libs)
	ADD_DEFINITIONS( -fPIC )
endif (UNIX)


#
# combined thread checker, openmp, icc and gcc support for linux
#
IF ( UNIX )

	IF ( NOT APPLE )

		# *** Linux ***

		IF ( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )

			IF ( USE_OMP )

				find_package(OpenMP REQUIRED)
				SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
				SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

				#ADD_DEFINITIONS( -fopenmp -fPIC )
			ELSE ( USE_OMP )

				MESSAGE( STATUS "No OpenMP support - ignoring unknown pragmas")
				ADD_DEFINITIONS( -Wno-unknown-pragmas )

			ENDIF ( USE_OMP )

		ELSE ( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )

			MESSAGE( STATUS "ICC FOUND" )

			IF ( USE_THREAD_CHECKER )

				# thread checker replaces complete cxx and c flags
				MESSAGE( STATUS "THREADCHECKER AND OPENMP SET" )
				find_package(OpenMP REQUIRED)
				SET( CMAKE_CXX_FLAGS "-g -O0" )
				SET( CMAKE_C_FLAGS "-g -O0" )
				SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
				SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

			ELSEIF ( USE_THREAD_CHECKER )

				find_package(OpenMP REQUIRED)
				SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
				SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

				#		  ADD_DEFINITIONS( -openmp )
			ENDIF ( USE_THREAD_CHECKER )

		ENDIF ( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )

	ELSE ( NOT APPLE )

		# *** Mac ***

		IF ( USE_OMP )
			set(OPENMP_LIBRARIES "/usr/local/Cellar/gcc/8.1.0/lib/gcc/8")
			set(OPENMP_INCLUDES "/usr/local/Cellar/gcc/8.1.0/include/c++/8.1.0")

			set(OpenMP_C "${CMAKE_C_COMPILER}")
			set(OpenMP_C_FLAGS "-fopenmp")
			set(OpenMP_C_LIB_NAMES "libgomp")

			set(OpenMP_CXX "${CMAKE_CXX_COMPILER}")
			set(OpenMP_CXX_FLAGS "-fopenmp")
			set(OpenMP_CXX_LIB_NAMES "libgomp")

			set(OpenMP_libgomp_LIBRARY libgomp)

			OPTION (USE_OpenMP "Use OpenMP to enamble <omp.h>" ON)

			MESSAGE( STATUS "Checking for OpenMP support on Mac")
			MESSAGE( STATUS "OpenMP_C_FLAGS = ${OpenMP_C_FLAGS}" )
			MESSAGE( STATUS "OpenMP_CXX_FLAGS = ${OpenMP_CXX_FLAGS}" )
			find_package(OpenMP REQUIRED)

			SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
			SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

		ELSE ( USE_OMP )

			MESSAGE( STATUS "No OpenMP support - ignoring unknown pragmas")
			ADD_DEFINITIONS( -Wno-unknown-pragmas )

		ENDIF ( USE_OMP )


	ENDIF ( NOT APPLE )

ELSE ( UNIX  )

	# *** Windows ***

	IF ( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )
		# MinGW compiler
		MESSAGE( STATUS "GCC FOUND")

		IF ( USE_OMP )

			find_package(OpenMP REQUIRED)
			SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
			SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

			#ADD_DEFINITIONS( -fopenmp )
		ELSE ( USE_OMP )

			ADD_DEFINITIONS( -Wno-unknown-pragmas )

		ENDIF ( USE_OMP )

	ENDIF ( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )

	IF (MSVC)
		# VisualStudio compiler
		IF ( USE_OMP )
			find_package(OpenMP REQUIRED)
			SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
			SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

		ELSE ( USE_OMP )

			MESSAGE( STATUS "No OpenMP support - ignoring unknown pragmas")
			#ADD_DEFINITIONS( -Wno-unknown-pragmas )

		ENDIF ( USE_OMP )

	ENDIF (MSVC)
ENDIF( UNIX )
