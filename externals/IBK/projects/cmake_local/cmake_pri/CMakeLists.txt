

# on Unix and Apple we want really detailed warnings
if (UNIX)
	ADD_DEFINITIONS( -Wall )
endif (UNIX)


#
# combined thread checker, openmp, icc and gcc support for linux
#
IF ( UNIX AND NOT APPLE )

	IF ( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )

		IF ( USE_OMP )

			find_package(OpenMP REQUIRED)
			SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
			SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

		#                        ADD_DEFINITIONS( -fopenmp -fPIC )
		ELSE ( USE_OMP )

			MESSAGE( STATUS "No OpenMP support - ignoring unknown pragmas")
			ADD_DEFINITIONS( -Wunknown-pragmas )

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

ELSE ( UNIX AND NOT APPLE )

	IF ( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )
		# MinGW compiler
		MESSAGE( STATUS "GCC FOUND")

		IF ( USE_OMP )

			find_package(OpenMP REQUIRED)
			SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
			SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

			#ADD_DEFINITIONS( -fopenmp )
		ELSE ( USE_OMP )

			ADD_DEFINITIONS( -Wunknown-pragmas )

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
			#ADD_DEFINITIONS( -Wunknown-pragmas )

		ENDIF ( USE_OMP )
		
	ENDIF (MSVC)
ENDIF( UNIX AND NOT APPLE )



