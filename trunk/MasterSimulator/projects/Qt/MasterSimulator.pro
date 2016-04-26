# Project file for MasterSimulator
# remember to set DYLD_FALLBACK_LIBRARY_PATH on MacOSX
# set LD_LIBRARY_PATH on Linux

TARGET = MasterSimulator

# this is the central configuration file for all IBK dependent libraries
# we check if this file was created by our build tool helper python what ever
!include( ../../../externals/IBK/projects/Qt/CONFIG.pri ){
	message( "No custom build options specified" )
}

# this pri must be sourced from all our applications
include( ../../../externals/IBK/projects/Qt/IBK.pri )

TEMPLATE = app
QT -= core gui
CONFIG += console
CONFIG -= app_bundle

CONFIG(debug, debug|release) {
	OBJECTS_DIR = debug
	DESTDIR = ../../../bin/debug
}
else {
	OBJECTS_DIR = release
	DESTDIR = ../../../bin/release
}

unix {
#	LIBS += -L../../../externals/FMILibrary/lib_linux64
}

win32 {
#	LIBS += -L../../../externals/FMILibrary/lib_win32
}

LIBS += -L../../../externals/lib \
	-L../../../lib \
	-lMasterSim \
	-lFMILibrary \
	-lminizip \
	-lexpat \
	-lz \
	-ldl \
	-lDataIO \
	-lIBK

INCLUDEPATH = \
	../../../MasterSim/src \
	../../../externals/DataIO/src \
	../../../externals/IBK/src

SOURCES += \
	../../src/main.cpp

HEADERS +=

