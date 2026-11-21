# CMakeLists.txt file for test FMU Radiator
#
# Note: This CMakeLists.txt file is not for stand-alone builds.
#       Requires toplevel CMakeLists.txt file.

project( Radiator )

# add build target for library - shared library, packaged as FMU afterwards
add_library( ${PROJECT_NAME} SHARED
	src/${PROJECT_NAME}.cpp
	src/fmi2common/fmi2Functions.cpp
	src/fmi2common/InstanceData.cpp
)

target_include_directories( ${PROJECT_NAME} PRIVATE
	src
)

# link against the dependent libraries
target_link_libraries( ${PROJECT_NAME}
	${APPLE_FRAMEWORKS}
)
