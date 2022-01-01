# CMakeLists.txt file for MasterSimulator

project( MasterSimulator )

# add include directories
include_directories(
	${PROJECT_SOURCE_DIR}/../../../externals/IBK/src
	${PROJECT_SOURCE_DIR}/../../../MasterSim/src
)

# collect a list of all source files
file( GLOB APP_SRCS ${PROJECT_SOURCE_DIR}/../../src/*.cpp )

if( WIN32 )
	set (LINK_Z_LIBS zlibstatic)
else( WIN32 )
	# unix Linux/Unix/Darwin we use the system libz package
	set (LINK_Z_LIBS z dl)
endif( WIN32 )

if (UNIX AND NOT APPLE)
	# unix Linux/Unix we use the system tinyxml package
	set (LINK_TINYXML_LIB tinyxml )
else (UNIX AND NOT APPLE)
	set (LINK_TINYXML_LIB TiCPP )
endif (UNIX AND NOT APPLE)

add_executable( ${PROJECT_NAME}
	${APP_SRCS}
)

# link against the dependent libraries
target_link_libraries( ${PROJECT_NAME} 
	MasterSim
	IBKMK
	IBK
	${LINK_TINYXML_LIB}
	minizip
	${LINK_Z_LIBS}
)


# installation targets for Unix systems
install(TARGETS ${PROJECT_NAME})
