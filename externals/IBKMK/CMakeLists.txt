project( IBKMK )

# collect a list of all source files
file( GLOB SRCS src/*.cpp )
file( GLOB C_SRCS src/*.c )
SET( SRCS ${SRCS} ${C_SRCS} )

# add build target for library - static (only c++)
add_library( ${PROJECT_NAME} STATIC
	${SRCS}
)

target_include_directories(${PROJECT_NAME} PUBLIC 
	src
	src/CDT
)

# Link against depending library. For a static library
# this automatically imports the include paths needed
# to compile against the library.
target_link_libraries(${PROJECT_NAME} PUBLIC
	IBK
)
