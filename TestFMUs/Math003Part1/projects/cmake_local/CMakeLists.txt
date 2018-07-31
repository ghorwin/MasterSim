# CMakeLists.txt file for Therakles FMU v2

project( FMITestCases )

# add include directories
include_directories(
	${PROJECT_SOURCE_DIR}/../../src
)

add_library( Math_003_Part1 SHARED
	${PROJECT_SOURCE_DIR}/../../src/InstanceData.cpp
	${PROJECT_SOURCE_DIR}/../../src/fmi2common/fmi2Functions.cpp
	${PROJECT_SOURCE_DIR}/../../src/fmi2common/InstanceDataCommon.cpp
)

# link against the dependent libraries
target_link_libraries( Math_003_Part1
	${APPLE_FRAMEWORKS}
)

