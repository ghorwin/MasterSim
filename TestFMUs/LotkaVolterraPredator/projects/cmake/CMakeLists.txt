project( LotkaVolterraPredator )

cmake_minimum_required(VERSION 3.5)

# add include directories
include_directories(
	${PROJECT_SOURCE_DIR}/../../src
)

add_library( ${PROJECT_NAME} SHARED
	${PROJECT_SOURCE_DIR}/../../src/${PROJECT_NAME}.cpp
	${PROJECT_SOURCE_DIR}/../../src/fmi2common/fmi2Functions.cpp
	${PROJECT_SOURCE_DIR}/../../src/fmi2common/InstanceData.cpp
)

# link against the dependent libraries
target_link_libraries( ${PROJECT_NAME}
	${APPLE_FRAMEWORKS}
)

# FMU building command
# define a command that re-generates the FMU from the already compiled library
#add_custom_command(
#	DEPENDS LotkaVolterraPredator
#	OUTPUT	Predator.fmu
#	COMMAND  ${PROJECT_SOURCE_DIR}/generate_FMU.sh ${PROJECT_SOURCE_DIR}/../.. ${CMAKE_CURRENT_BINARY_DIR}
#	COMMENT "Generating Predator.fmu"
#)

#add_custom_target(ExampleFMUPredator ALL
#	DEPENDS	Predator.fmu
#)
