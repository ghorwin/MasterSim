# CMakeLists.txt file for MasterSim library
#
# Note: This CMakeLists.txt file is not for stand-alone builds.
#       Requires toplevel CMakeLists.txt file.

project( MasterSim )

# collect a list of all source files of the library
file( GLOB SRCS src/*.cpp )

# add build target for library - static (only c++)
add_library( ${PROJECT_NAME} STATIC
	${SRCS}
)

target_include_directories( ${PROJECT_NAME} PUBLIC
	src
)

# Link against depending libraries. For a static library
# this automatically imports the include paths needed
# to compile against the library.
#
# Note: CMAKE_DL_LIBS is the library providing dlopen() and friends (empty on Windows).
target_link_libraries( ${PROJECT_NAME} PUBLIC
	IBKMK
	TiCPP
	minizip
	IBK
	${CMAKE_DL_LIBS}
)
