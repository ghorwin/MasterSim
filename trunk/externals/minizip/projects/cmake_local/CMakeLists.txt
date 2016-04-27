cmake_minimum_required(VERSION 2.8)

project(minizip C)

set(BUILD_SHARED_LIBS OFF)
set(SKIP_INSTALL_HEADERS ON)
set(SKIP_INSTALL_FILES ON)

set(MINIZP_SRC ${PROJECT_SOURCE_DIR}/../../src)

if(CMAKE_CL_64)
	set_target_properties(zlib PROPERTIES STATIC_LIBRARY_FLAGS "/machine:x64")
endif()

if(CMAKE_HOST_APPLE)
set(PLATFORM __APPLE__)
elseif(CMAKE_HOST_UNIX)
set(PLATFORM unix)
elseif(CMAKE_HOST_WIN32)
set(PLATFORM _WIN32)
else(CMAKE_HOST_APPLE)
message(FATAL_ERROR "Not supported Platform")
endif(CMAKE_HOST_APPLE)

add_definitions(-D${PLATFORM})

if(CMAKE_COMPILER_IS_GNUCC)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
endif()


include_directories( 
	${CMAKE_BINARY_DIR}/libz # add binary dir to include paths so that auto-generated zconf.h is found
	${PROJECT_SOURCE_DIR}/../../../zlib/src
) 

set(SOURCE
  ${MINIZP_SRC}/ioapi.c
  ${MINIZP_SRC}/miniunz.c
  ${MINIZP_SRC}/minizip.c
  ${MINIZP_SRC}/unzip.c
  ${MINIZP_SRC}/zip.c
)

if(WIN32)
    set(SOURCE ${SOURCE} ${MINIZP_SRC}/iowin32.c)
endif(WIN32)

add_library(minizip ${SOURCE})

target_link_libraries(minizip zlib)


