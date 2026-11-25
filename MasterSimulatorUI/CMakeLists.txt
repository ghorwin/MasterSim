# CMakeLists.txt file for MasterSimulatorUI application
#
# Note: This CMakeLists.txt file is not for stand-alone builds.
#       Requires toplevel CMakeLists.txt file.

project( MasterSimulatorUI )

# collect a list of all source files
file( GLOB APP_SRCS CONFIGURE_DEPENDS src/*.cpp )

# collect a list of moc header-files in
file( GLOB APP_HDRS CONFIGURE_DEPENDS src/*.h )

# collect a list of all form files (processed by AUTOUIC)
file( GLOB APP_UIS CONFIGURE_DEPENDS src/*.ui )

# Qt-Ressourcen (AUTORCC handles these automatically)
set( APP_RESOURCES
	resources/MasterSimulator.qrc
)

# resource script with the application icon, only used on Windows
set( APP_WIN_RC
	resources/MasterSimulator.rc
)

# build application executable for the different platforms
if( WIN32 )

	add_executable( ${PROJECT_NAME} WIN32
		${APP_SRCS}
		${APP_HDRS}
		${APP_UIS}
		${APP_RESOURCES}
		${APP_WIN_RC}
	)

elseif( APPLE )

	# application icon of the bundle
	set( APP_ICON resources/MasterSimulatorUI.icns )
	set_source_files_properties( ${APP_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources" )

	add_executable( ${PROJECT_NAME} MACOSX_BUNDLE
		${APP_SRCS}
		${APP_HDRS}
		${APP_UIS}
		${APP_RESOURCES}
		${APP_ICON}
	)

	set_target_properties( ${PROJECT_NAME} PROPERTIES
		MACOSX_BUNDLE_ICON_FILE MasterSimulatorUI.icns
		# application name shown in the macOS application menu (instead of executable name)
		MACOSX_BUNDLE_BUNDLE_NAME "MasterSim"
		# version shown in Finder/"About" (MSIM_VERSION is read from MSIM_Version.h in the top-level CMakeLists.txt)
		MACOSX_BUNDLE_SHORT_VERSION_STRING "${MSIM_VERSION}"
		MACOSX_BUNDLE_BUNDLE_VERSION "${MSIM_VERSION}"
	)

else()

	add_executable( ${PROJECT_NAME}
		${APP_SRCS}
		${APP_HDRS}
		${APP_UIS}
		${APP_RESOURCES}
	)

endif()

target_include_directories( ${PROJECT_NAME} PRIVATE
	# needed so that the ui-generated header files find our own headers
	src
)

find_package( Qt6 COMPONENTS Core Gui Network PrintSupport Svg Widgets REQUIRED )

# Link against the dependent libraries. The libraries below export their
# include paths and their own dependencies, so for example linking against
# MasterSim also pulls in IBKMK, TiCPP, minizip and zlib.
target_link_libraries( ${PROJECT_NAME}
	MasterSim
	BlockMod
	PhantomStyle
	IBK
	Qt6::Core
	Qt6::Gui
	Qt6::Network
	Qt6::PrintSupport
	Qt6::Svg
	Qt6::Widgets
	${APPLE_FRAMEWORKS}
)

if( WIN32 )
	target_link_libraries( ${PROJECT_NAME} iphlpapi )
endif( WIN32 )

# we want the binary to be named 'mastersim-gui'
set_target_properties( ${PROJECT_NAME} PROPERTIES OUTPUT_NAME "mastersim-gui" )

# Support for 'make install' on Unix/Linux (not on MacOS!)
if( UNIX AND NOT APPLE )

	# installation targets for Unix systems
	include( GNUInstallDirs )

	# MasterSimulatorUI -> /usr/bin/mastersim-gui
	install( TARGETS ${PROJECT_NAME} RUNTIME
			DESTINATION ${CMAKE_INSTALL_BINDIR} )

	# Man-page -> /usr/share/man/man1/mastersim-gui.1.gz
	install( FILES ${PROJECT_SOURCE_DIR}/doc/mastersim-gui.1
			DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 )

	# Translation files
	install( FILES ${PROJECT_SOURCE_DIR}/resources/translations/mastersim-gui_de.qm
			DESTINATION ${CMAKE_INSTALL_LOCALEDIR}/de/LC_MESSAGES/
			RENAME mastersim-gui.qm )

	# Desktop file
	install( FILES ${PROJECT_SOURCE_DIR}/resources/mastersim.desktop
			DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications )

	# Mime type
	install( FILES ${PROJECT_SOURCE_DIR}/resources/mastersim.xml
			DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/mime/packages )

	# Icons
	set( ICON_ROOT_DIR ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor )

	foreach( ICON_SIZE 512 256 64 48 32 16 )
		install( FILES ${PROJECT_SOURCE_DIR}/resources/gfx/logo/Icon_${ICON_SIZE}.png
				DESTINATION ${ICON_ROOT_DIR}/${ICON_SIZE}x${ICON_SIZE}/apps
				RENAME mastersim.png )

		install( FILES ${PROJECT_SOURCE_DIR}/resources/gfx/logo/Icon_${ICON_SIZE}.png
				DESTINATION ${ICON_ROOT_DIR}/${ICON_SIZE}x${ICON_SIZE}/mimetypes
				RENAME application-mastersim.png )
	endforeach()

endif()
