# Project file for MasterSimulator
# remember to set DYLD_FALLBACK_LIBRARY_PATH on MacOSX
# set LD_LIBRARY_PATH on Linux

TARGET = MasterSimulator
TEMPLATE = app

# this pri must be sourced from all our applications
include( ../externals/IBK/IBK.pri )

QT -= core gui
CONFIG += console
CONFIG -= app_bundle

LIBS += \
	-lMasterSim \
	-lminizip \
	-lIBKMK \
	-lTiCPP \
	-lIBK

unix {
	LIBS += \
		-lz \
		-ldl
}

win32 {
	LIBS += -lzlib
}

INCLUDEPATH = \
	../MasterSim/src \
	../externals/IBK/src

DEPENDPATH = $${INCLUDEPATH}

SOURCES += \
	src/main.cpp

