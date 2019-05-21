# CMakeLists.txt file for QtPropertyBrowser library

project( QtPropertyBrowser )

# collect a list of all source files to compile
file( GLOB LIB_SRCS ${PROJECT_SOURCE_DIR}/../../src/*.cpp )

# collect a list of all form files
file( GLOB LIB_UIS ${PROJECT_SOURCE_DIR}/../../src/*.ui )

include_directories(
	${Qt5Widgets_INCLUDE_DIRS}
	${PROJECT_SOURCE_DIR}/../../src			# needed so that ui-generated header files find our own headers
)


# automatically generate .moc and moc_xxx.cpp files when needed
set(CMAKE_AUTOMOC ON)

qt5_wrap_ui( LIB_UI_SRCS ${LIB_UIS} )

# add build target for library
add_library( ${PROJECT_NAME} STATIC 
	${LIB_SRCS}
	${LIB_UI_SRCS}
)


