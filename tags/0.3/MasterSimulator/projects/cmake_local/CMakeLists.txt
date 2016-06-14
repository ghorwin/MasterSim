# CMakeLists.txt file for MasterSimulator

project( MasterSimulator )

# add include directories
include_directories(
	${PROJECT_SOURCE_DIR}/../../../externals/IBK/src
	${PROJECT_SOURCE_DIR}/../../../externals/DataIO/src
	${PROJECT_SOURCE_DIR}/../../../MasterSim/src
)

# collect a list of all source files
file( GLOB APP_SRCS ${PROJECT_SOURCE_DIR}/../../src/*.cpp )

# set variable for dependent libraries
set( LINK_LIBS
	MasterSim
	DataIO
	IBKMK
	IBK
	TiCPP
	minizip
)

if( WIN32 )
	set (LINK_LIBS ${LINK_LIBS} zlibstatic)
else( WIN32 )
	set (LINK_LIBS ${LINK_LIBS} z dl)
endif( WIN32 )

add_executable( ${PROJECT_NAME}
	${APP_SRCS}
)

# link against the dependent libraries
target_link_libraries( ${PROJECT_NAME} 
	${LINK_LIBS} 
)

