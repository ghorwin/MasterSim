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

LIBS += -L../lib \ # for our top-level MasterSim lib
	-lMasterSim \
	-lminizip \
	-lIBKMK \
	-lIBK \
	-lTiCPP

unix {
	LIBS += \
		-lz \
		-ldl
}

DEPENDPATH += $${INCLUDEPATH}

win32 {
	LIBS += -lzlib
}

INCLUDEPATH = \
	../MasterSim/src \
	../externals/IBK/src

SOURCES += \
	src/main.cpp

