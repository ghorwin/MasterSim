# CMakeLists.txt file for BlockMod Toolkit Library

cmake_minimum_required( VERSION 2.6 )

project( BlockMod )

include_directories(
	${PROJECT_SOURCE_DIR}/../../src
	${Qt5Widgets_INCLUDE_DIRS}
)

# collect a list of all source files
file( GLOB LIB_SRCS ${PROJECT_SOURCE_DIR}/../../src/*.cpp )

# collect a list of all header files (to be used in MOC compiler)
file( GLOB LIB_HDRS ${PROJECT_SOURCE_DIR}/../../src/*.h )

# collect a list of all form files
file( GLOB LIB_UIS ${PROJECT_SOURCE_DIR}/../../src/*.ui )

# The Qt resource file (*.qrc)
set( LIB_QRCS	${PROJECT_SOURCE_DIR}/../../resources/BlockMod.qrc )

# run uic and moc compilers
if (Qt5Widgets_INCLUDE_DIRS)

	qt5_add_resources( LIB_RC_SRCS ${LIB_QRCS} )
	qt5_wrap_ui( LIB_UI_SRCS ${LIB_UIS} )
	qt5_wrap_cpp( LIB_MOC_SRCS ${LIB_HDRS} )

else (Qt5Widgets_INCLUDE_DIRS)

	qt4_add_resources( LIB_RC_SRCS ${LIB_QRCS} )
	qt4_wrap_ui( LIB_UI_SRCS ${LIB_UIS} )
	qt4_wrap_cpp( LIB_MOC_SRCS ${LIB_HDRS} )

endif (Qt5Widgets_INCLUDE_DIRS)

# build application executable for the different platforms
add_library( ${PROJECT_NAME}
	${LIB_SRCS} ${LIB_MOC_SRCS} ${LIB_RC_SRCS} ${LIB_UI_SRCS} 
)

