cmake_minimum_required(VERSION 3.5...3.13)

project( Math003Part3 )

# add include directories
include_directories(
	${PROJECT_SOURCE_DIR}/src
)

add_library( ${PROJECT_NAME} SHARED
	${PROJECT_SOURCE_DIR}/src/${PROJECT_NAME}.cpp
	${PROJECT_SOURCE_DIR}/src/fmi2common/fmi2Functions.cpp
	${PROJECT_SOURCE_DIR}/src/fmi2common/InstanceData.cpp
)

# link against the dependent libraries
target_link_libraries( ${PROJECT_NAME}
	${APPLE_FRAMEWORKS}
)

