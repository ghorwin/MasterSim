# CMakeLists.txt file for MasterSimulatorUI

project( MasterSimulatorUI )

# add include directories
include_directories(
	${CMAKE_BINARY_DIR}						# needed to find ui-header files
	${PROJECT_BINARY_DIR}					# needed so that ui-generated header files find our own headers
	${PROJECT_SOURCE_DIR}/../../src			# needed so that ui-generated header files find our own headers
	${PROJECT_SOURCE_DIR}/../../../externals/IBK/src
	${PROJECT_SOURCE_DIR}/../../../externals/DataIO/src
	${PROJECT_SOURCE_DIR}/../../../MasterSim/src
)

# collect a list of all source files
file( GLOB APP_SRCS ${PROJECT_SOURCE_DIR}/../../src/*.cpp )

# collect a list of all header files (to be used in MOC compiler)
file( GLOB APP_HDRS ${PROJECT_SOURCE_DIR}/../../src/*.h )

# collect a list of all form files
file( GLOB APP_UIS ${PROJECT_SOURCE_DIR}/../../src/*.ui )

# set variable for dependent libraries
set( LINK_LIBS
	MasterSim
	DataIO
	IBKMK
	IBK
	TiCPP
	minizip
)

if( WIN32 )
	set (LINK_LIBS ${LINK_LIBS} zlibstatic)
else( WIN32 )
	set (LINK_LIBS ${LINK_LIBS} z dl)
endif( WIN32 )

# this command will generate rules that will run rcc on all qrc files
qt4_add_resources( APP_RC_SRCS ${APP_QRCS} )

# this will run uic on .ui files:
qt4_wrap_ui( APP_UI_SRCS ${APP_UIS} )

# and finally this will run moc:
qt4_wrap_cpp( APP_MOC_SRCS ${APP_HDRS} )

# build application executable for the different platforms
if( WIN32 )
	add_executable( ${PROJECT_NAME} WIN32 
					 ${APP_SRCS} ${APP_MOC_SRCS} ${APP_RC_SRCS} ${APP_UI_SRCS} 
	)
endif( WIN32 ) 

if( UNIX )
	if( APPLE )
		add_executable( ${PROJECT_NAME} 
						 ${APP_SRCS} ${APP_MOC_SRCS} ${APP_RC_SRCS} ${APP_UI_SRCS} 
						 ${PROJECT_SOURCE_DIR}/../../resources/MasterSimulatorUI.icns 
		)
	else( APPLE )
		add_executable( ${PROJECT_NAME} 
						 ${APP_SRCS} ${APP_MOC_SRCS} ${APP_RC_SRCS} ${APP_UI_SRCS} 
		)
	endif( APPLE ) 
endif( UNIX )


if( APPLE )

	# link against the dependent libraries
	target_link_libraries( ${PROJECT_NAME} 
		${LINK_LIBS}
		${QT_LIBRARIES}
		"-framework Security"
	)

else( APPLE )

	# link against the dependent libraries
	target_link_libraries( ${PROJECT_NAME} 
		${LINK_LIBS}
		${QT_LIBRARIES}
		${QT_QTMAIN_LIBRARY} # to avoid "error LNK2019: unresolved external symbol _WinMain@16 referenced in function ___tmainCRTStartup"
	)

endif( APPLE )





