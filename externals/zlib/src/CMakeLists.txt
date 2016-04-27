

cmake_minimum_required(VERSION 2.6)

project(zlib C)

include(CheckTypeSize)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckCSourceCompiles)
enable_testing()

check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(stdint.h    HAVE_STDINT_H)
check_include_file(stddef.h    HAVE_STDDEF_H)
#
# Check to see if we have large file support
#
set(CMAKE_REQUIRED_DEFINITIONS -D_LARGEFILE64_SOURCE=1)

#
# Check for fseeko
#
check_function_exists(fseeko HAVE_FSEEKO)
if(NOT HAVE_FSEEKO)
    add_definitions(-DNO_FSEEKO)
endif()

#
# Check for unistd.h
#
check_include_file(unistd.h Z_HAVE_UNISTD_H)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/zconf.h.cmakein
               ${CMAKE_CURRENT_BINARY_DIR}/zconf.h @ONLY)


include_directories( 
	${PROJECT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/contrib/minizip
)


#============================================================================
# zlib
#============================================================================

set(ZLIB_SRCS
    adler32.c
    compress.c
    crc32.c
    deflate.c
    gzclose.c
    gzlib.c
    gzread.c
    gzwrite.c
    inflate.c
    infback.c
    inftrees.c
    inffast.c
    trees.c
    uncompr.c
    zutil.c
    contrib/minizip/ioapi.c
    contrib/minizip/miniunz.c
    contrib/minizip/minizip.c
    contrib/minizip/mztools.c
    contrib/minizip/unzip.c
    contrib/minizip/zip.c
)

add_library( z STATIC 
  ${ZLIB_SRCS} 
#  ${ZLIB_PUBLIC_HDRS} 
#  ${ZLIB_PRIVATE_HDRS}
)

