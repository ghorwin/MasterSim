# remember to set DYLD_FALLBACK_LIBRARY_PATH on MacOSX
# set LD_LIBRARY_PATH on Linux

TARGET = MasterSimulatorUI
# this is the central configuration file for all IBK dependent libraries
# we check if this file was created by our build tool helper python what ever
!include( ../../../externals/IBK/projects/Qt/CONFIG.pri ){
	message( "No custom build options specified" )
}

# this pri must be sourced from all our applications
include( ../../../externals/IBK/projects/Qt/IBK.pri )

TEMPLATE = app
QT += xml core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG(debug, debug|release) {
	OBJECTS_DIR = debug
	DESTDIR = ../../../bin/debug
}
else {
	OBJECTS_DIR = release
	DESTDIR = ../../../bin/release
}

LIBS += -L../../../externals/lib \
	-L../../../lib \
	-lMasterSim \
	-lminizip \
	-lz \
	-lBlockMod \
	-lDataIO \
	-lIBKMK \
	-lIBK \
	-lTiCPP

INCLUDEPATH = \
	../../src \
	../../../externals/IBK/src \
	../../../externals/DataIO/src \
	../../../externals/BlockMod/src \
	../../../MasterSim/src


SOURCES += ../../src/main.cpp \
	../../src/MSIMUIConstants.cpp \
	../../src/MSIMDirectories.cpp \
	../../src/MSIMMainWindow.cpp \
	../../src/MSIMProjectHandler.cpp \
	../../src/MSIMSettings.cpp \
	../../src/MSIMConversion.cpp \
	../../src/MSIMLanguageHandler.cpp \
	../../src/MSIMMessageHandler.cpp \
	../../src/MSIMDebugApplication.cpp \
	../../src/MSIMWelcomeScreen.cpp \
    ../../src/MSIMLogWidget.cpp \
    ../../src/MSIMPreferencesDialog.cpp \
    ../../src/MSIMPreferencesPageGeneral.cpp \
    ../../src/MSIMUndoCommandBase.cpp \
    ../../src/MSIMUndoProject.cpp \
    ../../src/MSIMBrowseFilenameWidget.cpp \
    ../../src/MSIMViewSlaves.cpp \
    ../../src/MSIMViewConnections.cpp \
    ../../src/MSIMSlaveItemDelegate.cpp \
    ../../src/MSIMConnectionItemDelegate.cpp \
    ../../src/MSIMViewSimulation.cpp

HEADERS  += \
	../../src/MSIMUIConstants.h \
	../../src/MSIMDirectories.h \
	../../src/MSIMMainWindow.h \
	../../src/MSIMProjectHandler.h \
	../../src/MSIMSettings.h \
	../../src/MSIMConversion.h \
	../../src/MSIMLanguageHandler.h \
	../../src/MSIMMessageHandler.h \
	../../src/MSIMDebugApplication.h \
	../../src/MSIMWelcomeScreen.h \
    ../../src/MSIMLogWidget.h \
    ../../src/MSIMPreferencesDialog.h \
    ../../src/MSIMPreferencesPageGeneral.h \
    ../../src/MSIMUndoCommandBase.h \
    ../../src/MSIMUndoProject.h \
    ../../src/MSIMBrowseFilenameWidget.h \
    ../../src/MSIMViewSlaves.h \
    ../../src/MSIMViewConnections.h \
    ../../src/MSIMSlaveItemDelegate.h \
    ../../src/MSIMConnectionItemDelegate.h \
    ../../src/MSIMViewSimulation.h

FORMS    += \
	../../src/MSIMMainWindow.ui \
	../../src/MSIMWelcomeScreen.ui \
    ../../src/MSIMPreferencesDialog.ui \
    ../../src/MSIMPreferencesPageGeneral.ui \
    ../../src/MSIMViewSlaves.ui \
    ../../src/MSIMViewConnections.ui \
    ../../src/MSIMViewSimulation.ui

TRANSLATIONS += ../../resources/translations/MasterSimulatorUI_de.ts
CODECFORSRC = UTF-8

RESOURCES += \
	../../resources/MasterSimulator.qrc

