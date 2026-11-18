# CMakeLists.txt file for MasterSimulator application (command line solver)
#
# Note: This CMakeLists.txt file is not for stand-alone builds.
#       Requires toplevel CMakeLists.txt file.

project( MasterSimulator )

# collect a list of all source files of the application
file( GLOB APP_SRCS src/*.cpp )

add_executable( ${PROJECT_NAME}
	${APP_SRCS}
)

target_include_directories( ${PROJECT_NAME} PRIVATE
	src
)

# The console solver is distributed as a stand-alone binary and is run on
# machines without a matching version of the GCC runtime, hence we link the
# runtime statically. On MacOS clang/libc++ is used and the flags do not apply.
if( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT APPLE )
	target_link_options( ${PROJECT_NAME} PRIVATE
		-static-libgcc
		-static-libstdc++
	)
endif()

# Link against depending libraries. The include paths of these libraries are
# imported automatically.
#
# Note: MasterSim pulls in the remaining libraries (IBKMK, TiCPP, minizip, zlib, IBK)
#       as public dependencies.
target_link_libraries( ${PROJECT_NAME}
	MasterSim
	IBK
)

# we want the binary to be named 'mastersim'
set_target_properties( ${PROJECT_NAME} PROPERTIES OUTPUT_NAME "mastersim" )

# Support for 'make install' on Unix/Linux (not on MacOS!)
if( UNIX AND NOT APPLE )

	# installation targets for Unix systems
	include( GNUInstallDirs )

	# MasterSimulator -> /usr/bin/mastersim
	install( TARGETS ${PROJECT_NAME} RUNTIME
		DESTINATION ${CMAKE_INSTALL_BINDIR} )

	# Man-page -> /usr/share/man/man1/mastersim.1.gz
	install( FILES ${PROJECT_SOURCE_DIR}/doc/mastersim.1
		DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 )

endif()
