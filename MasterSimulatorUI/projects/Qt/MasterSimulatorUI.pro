# remember to set DYLD_FALLBACK_LIBRARY_PATH on MacOSX
# set LD_LIBRARY_PATH on Linux

TARGET = MasterSimulatorUI
TEMPLATE = app

# this pri must be sourced from all our applications
include( ../../../externals/IBK/projects/Qt/IBK.pri )

QT += xml core gui network printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

LIBS += -L../../../lib$${DIR_PREFIX} \
	-lMasterSim \
	-lminizip \
	-lBlockMod \
	-lIBKMKmini \
	-lIBK \
	-lTiCPP

win32 {
	LIBS += -lzlib -luser32
}

INCLUDEPATH = \
	../../src \
	../../../externals/zlib/src \
	../../../externals/minizip/src \
	../../../externals/IBK/src \
	../../../externals/BlockMod/src \
	../../../externals/TiCPP/src \
	../../../MasterSim/src


SOURCES += \
	../../src/main.cpp \
	../../src/MSIMAboutDialog.cpp \
	../../src/MSIMBlockEditorDialog.cpp \
	../../src/MSIMBrowseFilenameWidget.cpp \
	../../src/MSIMButtonBar.cpp \
	../../src/MSIMConnectionItemDelegate.cpp \
	../../src/MSIMConnectionPropertiesEditDialog.cpp \
	../../src/MSIMConversion.cpp \
	../../src/MSIMDebugApplication.cpp \
	../../src/MSIMDirectories.cpp \
	../../src/MSIMGUIMessageHandler.cpp \
	../../src/MSIMLanguageHandler.cpp \
	../../src/MSIMLogFileDialog.cpp \
	../../src/MSIMLogWidget.cpp \
	../../src/MSIMMainWindow.cpp \
	../../src/MSIMMessageHandler.cpp \
	../../src/MSIMPostProcBindings.cpp \
	../../src/MSIMPreferencesDialog.cpp \
	../../src/MSIMPreferencesPageGeneral.cpp \
	../../src/MSIMProjectHandler.cpp \
	../../src/MSIMSceneManager.cpp \
	../../src/MSIMSettings.cpp \
	../../src/MSIMSimulationMonitorWidget.cpp \
	../../src/MSIMSlaveBlock.cpp \
	../../src/MSIMSlaveItemDelegate.cpp \
	../../src/MSIMSlaveTableWidget.cpp \
	../../src/MSIMUIConstants.cpp \
	../../src/MSIMUndoCommandBase.cpp \
	../../src/MSIMUndoConnectionModified.cpp \
	../../src/MSIMUndoConnections.cpp \
	../../src/MSIMUndoNetworkGeometry.cpp \
	../../src/MSIMUndoProject.cpp \
	../../src/MSIMUndoSimulationSettings.cpp \
	../../src/MSIMUndoSlaveParameters.cpp \
	../../src/MSIMUndoSlaves.cpp \
	../../src/MSIMViewConnections.cpp \
	../../src/MSIMViewSimulation.cpp \
	../../src/MSIMViewSlaves.cpp \
	../../src/MSIMWelcomeScreen.cpp

HEADERS  += \
	../../src/MSIMAboutDialog.h \
	../../src/MSIMBlockEditorDialog.h \
	../../src/MSIMBrowseFilenameWidget.h \
	../../src/MSIMButtonBar.h \
	../../src/MSIMConnectionItemDelegate.h \
	../../src/MSIMConnectionPropertiesEditDialog.h \
	../../src/MSIMConversion.h \
	../../src/MSIMDebugApplication.h \
	../../src/MSIMDirectories.h \
	../../src/MSIMGUIMessageHandler.h \
	../../src/MSIMLanguageHandler.h \
	../../src/MSIMLogFileDialog.h \
	../../src/MSIMLogWidget.h \
	../../src/MSIMMainWindow.h \
	../../src/MSIMMessageHandler.h \
	../../src/MSIMPostProcBindings.h \
	../../src/MSIMPreferencesDialog.h \
	../../src/MSIMPreferencesPageGeneral.h \
	../../src/MSIMProjectHandler.h \
	../../src/MSIMSceneManager.h \
	../../src/MSIMSettings.h \
	../../src/MSIMSimulationMonitorWidget.h \
	../../src/MSIMSlaveBlock.h \
	../../src/MSIMSlaveItemDelegate.h \
	../../src/MSIMSlaveTableWidget.h \
	../../src/MSIMUIConstants.h \
	../../src/MSIMUndoCommandBase.h \
	../../src/MSIMUndoConnectionModified.h \
	../../src/MSIMUndoConnections.h \
	../../src/MSIMUndoNetworkGeometry.h \
	../../src/MSIMUndoProject.h \
	../../src/MSIMUndoSimulationSettings.h \
	../../src/MSIMUndoSlaveParameters.h \
	../../src/MSIMUndoSlaves.h \
	../../src/MSIMViewConnections.h \
	../../src/MSIMViewSimulation.h \
	../../src/MSIMViewSlaves.h \
	../../src/MSIMWelcomeScreen.h

FORMS    += \
	../../src/MSIMAboutDialog.ui \
	../../src/MSIMBlockEditorDialog.ui \
	../../src/MSIMConnectionPropertiesEditDialog.ui \
	../../src/MSIMLogFileDialog.ui \
	../../src/MSIMMainWindow.ui \
	../../src/MSIMPreferencesDialog.ui \
	../../src/MSIMPreferencesPageGeneral.ui \
	../../src/MSIMSimulationMonitorWidget.ui \
	../../src/MSIMViewConnections.ui \
	../../src/MSIMViewSimulation.ui \
	../../src/MSIMViewSlaves.ui \
	../../src/MSIMWelcomeScreen.ui

TRANSLATIONS += ../../resources/translations/MasterSimulatorUI_de.ts
CODECFORSRC = UTF-8

RESOURCES += \
	../../resources/MasterSimulator.qrc

