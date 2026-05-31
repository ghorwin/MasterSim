# CMakeLists.txt file for BlockMod library

project( BlockMod )

# collect a list of all source files
file( GLOB LIB_SRCS ${PROJECT_SOURCE_DIR}/src/*.cpp )

# collect a list of all header files
file( GLOB LIB_MOC_SRCS ${PROJECT_SOURCE_DIR}/src/*.h )

# collect a list of all form files
file( GLOB LIB_UI_SRCS ${PROJECT_SOURCE_DIR}/src/*.ui )

# set resource files
set( LIB_QRCS ${PROJECT_SOURCE_DIR}/resources/BlockMod.qrc )

include_directories(
	${PROJECT_SOURCE_DIR}/src			# needed so that ui-generated header files find our own headers
	${PROJECT_SOURCE_DIR}/../IBK/src
	${Qt6Widgets_INCLUDE_DIRS}
	${Qt6PrintSupport_INCLUDE_DIRS}
	${Qt6Network_INCLUDE_DIRS}
	${Qt6Svg_INCLUDE_DIRS}
	${Qt6Xml_INCLUDE_DIRS}
)

# automatically generate .moc and moc_xxx.cpp files when needed
set( CMAKE_AUTOMOC ON )
set( CMAKE_AUTORCC ON )
set( CMAKE_AUTOUIC ON )

# add build target for library
add_library( ${PROJECT_NAME} STATIC
	${LIB_SRCS}
	${LIB_MOC_SRCS}
	${LIB_UI_SRCS}
	${LIB_QRCS}
)
