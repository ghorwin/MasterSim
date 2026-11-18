# CMakeLists.txt file for BlockMod library
#
# Note: This CMakeLists.txt file is not for stand-alone builds.
#       Requires toplevel CMakeLists.txt file.

project( BlockMod )

# collect a list of all source files of the library
file( GLOB SRCS src/*.cpp )

# collect a list of moc header-files in
file( GLOB MOC_HDRS src/*.h )

# collect a list of all form files (processed by AUTOUIC)
file( GLOB UIS src/*.ui )

# Qt-Ressourcen (AUTORCC handles these automatically)
set( RESOURCES
	resources/BlockMod.qrc
)

# add build target for library - Qt based, static
add_library( ${PROJECT_NAME} STATIC
	${SRCS}
	${MOC_HDRS}
	${UIS}
	${RESOURCES}
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
