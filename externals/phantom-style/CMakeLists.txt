# CMakeLists.txt file for PhantomStyle library (Qt widget style)
#
# Note: This CMakeLists.txt file is not for stand-alone builds.
#       Requires toplevel CMakeLists.txt file.

project( PhantomStyle )

# collect a list of all source files of the library
file( GLOB SRCS src/*.cpp )

# collect a list of moc header-files in
file( GLOB MOC_HDRS src/*.h )

# add build target for library - Qt based, static
add_library( ${PROJECT_NAME} STATIC
	${SRCS}
	${MOC_HDRS}
)

target_include_directories( ${PROJECT_NAME} PUBLIC
	src
)

find_package( Qt6 COMPONENTS Core Gui Widgets REQUIRED )

# Link against depending libraries. For a static library
# this automatically imports the include paths needed
# to compile against the library.
target_link_libraries( ${PROJECT_NAME} PUBLIC
	Qt6::Core
	Qt6::Gui
	Qt6::Widgets
)
