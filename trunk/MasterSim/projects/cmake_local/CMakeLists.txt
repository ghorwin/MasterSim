# CMakeLists.txt file for MasterSim
# Does not work as stand-alone CMakeLists.txt file, must be included in CMakeLists.txt master file.

project( MasterSim )

# add include directories
include_directories(
	${PROJECT_SOURCE_DIR}/../../../externals/IBK/src
	${PROJECT_SOURCE_DIR}/../../../externals/zlib/src
	${PROJECT_SOURCE_DIR}/../../../externals/minizip/src
	${PROJECT_SOURCE_DIR}/../../../externals/DataIO/src
	${PROJECT_SOURCE_DIR}/../../../externals/TiCPP/src
)

# collect a list of all source files of the library
file( GLOB LIB_SRCS ${PROJECT_SOURCE_DIR}/../../src/*.cpp )

# build the library
add_library( ${PROJECT_NAME}
	${LIB_SRCS}
)


