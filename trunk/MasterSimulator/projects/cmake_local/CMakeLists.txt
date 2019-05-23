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
	set (LINK_Z_LIBS z dl)
endif( WIN32 )


add_executable( ${PROJECT_NAME}
	${APP_SRCS}
)

# link against the dependent libraries
target_link_libraries( ${PROJECT_NAME} 
	MasterSim
	IBKMK
	IBK
	TiCPP
	minizip
	${LINK_Z_LIBS}
)

