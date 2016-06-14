# CMakeLists.txt file for BlockMod Toolkit Library

cmake_minimum_required( VERSION 2.6 )

project( BlockMod )

# set( CMAKE_VERBOSE_MAKEFILE ON )

# Qt4 package is required
find_package( Qt4 REQUIRED )

set( QT_USE_QTXML TRUE )
set( QT_USE_QTSVG TRUE )

include( ${QT_USE_FILE} )

include_directories(
	${CMAKE_BINARY_DIR}
	${PROJECT_BINARY_DIR}
	${PROJECT_SOURCE_DIR}/../../src
)

# collect a list of all source files
file( GLOB LIB_SRCS ${PROJECT_SOURCE_DIR}/../../src/*.cpp )

# collect a list of all header files (to be used in MOC compiler)
file( GLOB LIB_HDRS ${PROJECT_SOURCE_DIR}/../../src/*.h )

# collect a list of all form files
file( GLOB LIB_UIS ${PROJECT_SOURCE_DIR}/../../src/*.ui )

# The Qt resource file (*.qrc)
set( LIB_QRCS	${PROJECT_SOURCE_DIR}/../../resources/BlockMod.qrc )

if ( UNIX )
	add_definitions( -Wall )
endif( UNIX )

# this command will generate rules that will run rcc on all qrc files
qt4_add_resources( LIB_RC_SRCS ${LIB_QRCS} )

# this will run uic on .ui files:
qt4_wrap_ui( LIB_UI_SRCS ${LIB_UIS} )

# and finally this will run moc:
qt4_wrap_cpp( LIB_MOC_SRCS ${LIB_HDRS} )

# build application executable for the different platforms
add_library( ${PROJECT_NAME}
	${LIB_SRCS} ${LIB_MOC_SRCS} ${LIB_RC_SRCS} ${LIB_UI_SRCS} 
)
