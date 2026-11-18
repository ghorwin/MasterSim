project( IBK )

# configuration file options
option( IBK_ENABLE_SAFE_MATH "Turns checking of function arguments to math routines on." ON )
option( IBK_ENABLE_UNIT_NAME "Adds a member variable with unit ID name to the IBK::Unit for better debugging." ON )
option( IBK_ENABLE_COLORED_CONSOLE "Enables colored outputs in the console window." ON )

configure_file(
	src/IBK_configuration.h.in
	src/IBK_configuration.h
)

# collect a list of all source files
file( GLOB SRCS src/*.cpp )

# add build target for library - static (only c++)
add_library( ${PROJECT_NAME} STATIC
	${SRCS}
)

target_include_directories(${PROJECT_NAME} PUBLIC 
	src
)

# Note: the precompiled header is a c++-only header, hence we must restrict its use
#       to c++ sources - otherwise c-sources in libraries linking against IBK (i.e. IBKMK)
#       would attempt to precompile it as c-header and fail.
target_precompile_headers(${PROJECT_NAME} PUBLIC 
	$<$<COMPILE_LANGUAGE:CXX>:${PROJECT_SOURCE_DIR}/src/IBK_headers.h>
)
