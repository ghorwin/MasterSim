# CMakeLists.txt file for minizip library
#
# Note: This CMakeLists.txt file is not for stand-alone builds.
#       Requires toplevel CMakeLists.txt file.

project( minizip C )

# collect a list of all source files of the library
set( SRCS
	src/ioapi.c
	src/miniunz.c
	src/minizip.c
	src/unzip.c
	src/zip.c
)

if( WIN32 )
	list( APPEND SRCS src/iowin32.c )
endif( WIN32 )

# add build target for library - static (only c)
add_library( ${PROJECT_NAME} STATIC
	${SRCS}
)

target_include_directories( ${PROJECT_NAME} PUBLIC
	src
)

# platform define used within the minizip sources
if( APPLE )
	target_compile_definitions( ${PROJECT_NAME} PRIVATE __APPLE__ )
elseif( UNIX )
	target_compile_definitions( ${PROJECT_NAME} PRIVATE unix )
elseif( WIN32 )
	target_compile_definitions( ${PROJECT_NAME} PRIVATE _WIN32 )
endif()

set_target_properties( ${PROJECT_NAME} PROPERTIES
	C_STANDARD 99
)

# We use the system zlib on Linux/MacOS, on Windows zlib is provided via vcpkg
# (see build/setup_zlib_vcpkg.bat).
find_package( ZLIB REQUIRED )

# Link against depending library. zlib is linked publicly, because the public
# headers zip.h and unzip.h include <zlib.h>.
target_link_libraries( ${PROJECT_NAME} PUBLIC
	ZLIB::ZLIB
)
