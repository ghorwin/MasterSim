

# on Unix and Apple we want really detailed warnings
if (UNIX)
	ADD_DEFINITIONS( -Wall )
endif (UNIX)


#
# combined thread checker, openmp, icc and gcc support for linux
#
IF ( UNIX AND NOT APPLE )

	IF ( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )

		MESSAGE( STATUS "GCC FOUND")

		IF ( USE_OMP )

			find_package(OpenMP)
			IF (OPENMP_FOUND)
				SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
				SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
			ELSE( OPENMP_FOUND )
				message(STATUS "Disabling OpenMP support, could not determine compiler flags")
			ENDIF(OPENMP_FOUND)

		#                        ADD_DEFINITIONS( -fopenmp -fPIC )
		ELSE ( USE_OMP )

			ADD_DEFINITIONS( -Wunknown-pragmas )

		ENDIF ( USE_OMP )

	ELSE ( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )

		MESSAGE( STATUS "ICC FOUND" )

		IF ( USE_THREAD_CHECKER )

			# thread checker replaces complete cxx and c flags
			MESSAGE( STATUS "THREADCHECKER AND OPENMP SET" )
			find_package(OpenMP)
			IF (OPENMP_FOUND)
				SET( CMAKE_CXX_FLAGS "-g -O0" )
				SET( CMAKE_C_FLAGS "-g -O0" )
				SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
				SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
			ENDIF(OPENMP_FOUND)

		ELSEIF ( USE_THREAD_CHECKER )

			find_package(OpenMP)
			IF (OPENMP_FOUND)
				SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
				SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
			ENDIF(OPENMP_FOUND)

			#		  ADD_DEFINITIONS( -openmp )
		ENDIF ( USE_THREAD_CHECKER )

	ENDIF ( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )

ELSE ( UNIX AND NOT APPLE )

	# On Windows, openmp related stuff only for mingw compiler
	IF ( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )
		MESSAGE( STATUS "GCC FOUND")

		IF ( USE_OMP )

			find_package(OpenMP)
			IF (OPENMP_FOUND)
				SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
				SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
			ENDIF(OPENMP_FOUND)

		#                        ADD_DEFINITIONS( -fopenmp -fPIC )
		ELSE ( USE_OMP )

			ADD_DEFINITIONS( -Wunknown-pragmas )

		ENDIF ( USE_OMP )

	ENDIF ( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )

ENDIF( UNIX AND NOT APPLE )



