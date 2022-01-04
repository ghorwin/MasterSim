# CMakeLists.txt file for MasterSimulatorUI

project( MasterSimulatorUI )

# add include directories
include_directories(
	${PROJECT_SOURCE_DIR}/../../src			# needed so that ui-generated header files find our own headers
	${PROJECT_SOURCE_DIR}/../../../externals/IBK/src
	${PROJECT_SOURCE_DIR}/../../../externals/BlockMod/src
	${PROJECT_SOURCE_DIR}/../../../MasterSim/src
	${PROJECT_SOURCE_DIR}/../../../externals/zlib/src
	${PROJECT_SOURCE_DIR}/../../../externals/minizip/src
	${PROJECT_SOURCE_DIR}/../../../externals/TiCPP/src
	${Qt5Widgets_INCLUDE_DIRS}
	${Qt5Network_INCLUDE_DIRS}
	${Qt5Svg_INCLUDE_DIRS}
	${Qt5PrintSupport_INCLUDE_DIRS}
)

# collect a list of all source files
file( GLOB APP_SRCS ${PROJECT_SOURCE_DIR}/../../src/*.cpp )

# collect a list of all header files (to be used in MOC compiler)
file( GLOB APP_HDRS ${PROJECT_SOURCE_DIR}/../../src/*.h )

# collect a list of all form files
file( GLOB APP_UIS ${PROJECT_SOURCE_DIR}/../../src/*.ui )

# The Qt resource file (*.qrc)
set( APP_QRCS	${PROJECT_SOURCE_DIR}/../../resources/MasterSimulator.qrc )

# set rc file
set ( APP_WIN_RC
	${PROJECT_SOURCE_DIR}/../../resources/MasterSimulator.rc
)

# run uic and moc compilers, add resources
qt5_add_resources( APP_RC_SRCS ${APP_QRCS} )
qt5_wrap_ui( APP_UI_SRCS ${APP_UIS} )
qt5_wrap_cpp( APP_MOC_SRCS ${APP_HDRS} )

# generate list of all required QT_LIBRARIES
set ( QT_LIBRARIES 
	Qt5::Widgets
	Qt5::Svg
	Qt5::Network
	Qt5::PrintSupport
)

if( WIN32 )
	set (LINK_Z_LIBS zlibstatic)
else( WIN32 )
	# unix Linux/Unix/Darwin we use the system libz package
	set (LINK_Z_LIBS z dl)
endif( WIN32 )

# build application executable for the different platforms
if( WIN32 )
	add_executable( ${PROJECT_NAME} WIN32 
					 ${APP_SRCS} ${APP_MOC_SRCS} ${APP_RC_SRCS} ${APP_UI_SRCS} ${APP_WIN_RC}
	)

	set(WIN_FRAMEWORKS iphlpapi)

endif( WIN32 ) 

if( UNIX )
	if( APPLE )
		add_executable( ${PROJECT_NAME} MACOSX_BUNDLE 
						 ${APP_SRCS} ${APP_MOC_SRCS} ${APP_RC_SRCS} ${APP_UI_SRCS} 
		)
	else( APPLE )
		add_executable( ${PROJECT_NAME} 
						${APP_SRCS} ${APP_MOC_SRCS} ${APP_RC_SRCS} ${APP_UI_SRCS} 
						${PROJECT_SOURCE_DIR}/../../resources/MasterSimulatorUI.icns
		)
	endif( APPLE ) 

endif( UNIX )


# link against the dependent libraries
target_link_libraries( ${PROJECT_NAME} 
	MasterSim
	BlockMod
	IBKMK
	IBK
	TiCPP
	minizip
	${LINK_Z_LIBS}
	${QT_LIBRARIES}
	${APPLE_FRAMEWORKS}
	${WIN_FRAMEWORKS}
)

# Support for 'make install' on Unix/Linux (not on MacOS!)
if (UNIX AND NOT APPLE)

	# installation targets for Unix systems
	include(GNUInstallDirs)

	# MasterSimulator -> /usr/bin
	install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin )

	# Man-page
	install(FILES ${PROJECT_SOURCE_DIR}/../../doc/${PROJECT_NAME}.1 
			DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 )

	# Translation files
	install(FILES ${PROJECT_SOURCE_DIR}/../../resources/translations/${PROJECT_NAME}_de.qm 
			DESTINATION ${CMAKE_INSTALL_LOCALEDIR}/de/LC_MESSAGES/
			RENAME ${PROJECT_NAME}.qm)

	# Desktop file
	install(FILES ${PROJECT_SOURCE_DIR}/../../resources/mastersim.desktop 
			DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications )

	# Mime type
	install(FILES ${PROJECT_SOURCE_DIR}/../../resources/mastersim.xml 
			DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/mime/packages )

	# Icons
	set(ICON_ROOT_DIR ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor)

	install(FILES ${PROJECT_SOURCE_DIR}/../../resources/gfx/logo/Icon_512.png DESTINATION ${ICON_ROOT_DIR}/512x512/apps RENAME mastersim.png)
	install(FILES ${PROJECT_SOURCE_DIR}/../../resources/gfx/logo/Icon_256.png DESTINATION ${ICON_ROOT_DIR}/256x256/apps RENAME mastersim.png)
	install(FILES ${PROJECT_SOURCE_DIR}/../../resources/gfx/logo/Icon_128.png DESTINATION ${ICON_ROOT_DIR}/128x128/apps RENAME mastersim.png)
	install(FILES ${PROJECT_SOURCE_DIR}/../../resources/gfx/logo/Icon_64.png DESTINATION ${ICON_ROOT_DIR}/64x64/apps RENAME mastersim.png)
	install(FILES ${PROJECT_SOURCE_DIR}/../../resources/gfx/logo/Icon_48.png DESTINATION ${ICON_ROOT_DIR}/48x48/apps RENAME mastersim.png)
	install(FILES ${PROJECT_SOURCE_DIR}/../../resources/gfx/logo/Icon_32.png DESTINATION ${ICON_ROOT_DIR}/32x32/apps RENAME mastersim.png)
	install(FILES ${PROJECT_SOURCE_DIR}/../../resources/gfx/logo/Icon_16.png DESTINATION ${ICON_ROOT_DIR}/16x16/apps RENAME mastersim.png)

	install(FILES ${PROJECT_SOURCE_DIR}/../../resources/gfx/logo/Icon_512.png DESTINATION ${ICON_ROOT_DIR}/512x512/mimetypes RENAME application-mastersim.png)
	install(FILES ${PROJECT_SOURCE_DIR}/../../resources/gfx/logo/Icon_256.png DESTINATION ${ICON_ROOT_DIR}/256x256/mimetypes RENAME application-mastersim.png)
	install(FILES ${PROJECT_SOURCE_DIR}/../../resources/gfx/logo/Icon_128.png DESTINATION ${ICON_ROOT_DIR}/128x128/mimetypes RENAME application-mastersim.png)
	install(FILES ${PROJECT_SOURCE_DIR}/../../resources/gfx/logo/Icon_64.png DESTINATION ${ICON_ROOT_DIR}/64x64/mimetypes RENAME application-mastersim.png)
	install(FILES ${PROJECT_SOURCE_DIR}/../../resources/gfx/logo/Icon_48.png DESTINATION ${ICON_ROOT_DIR}/48x48/mimetypes RENAME application-mastersim.png)
	install(FILES ${PROJECT_SOURCE_DIR}/../../resources/gfx/logo/Icon_32.png DESTINATION ${ICON_ROOT_DIR}/32x32/mimetypes RENAME application-mastersim.png)
	install(FILES ${PROJECT_SOURCE_DIR}/../../resources/gfx/logo/Icon_16.png DESTINATION ${ICON_ROOT_DIR}/16x16/mimetypes RENAME application-mastersim.png)

endif (UNIX AND NOT APPLE)
