# ----------------------------------
# Project for FMU Test cases
# ----------------------------------

TARGET = Math003Part3
TEMPLATE = lib

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../externals/IBK/IBK.pri )

QT -= core gui

CONFIG(debug, debug|release) {
	windows {
                DLLDESTDIR = ../../bin/debug
	}
	else {
                DESTDIR = ../../bin/debug
	}
}
else {
	windows {
                DLLDESTDIR = ../../bin/release
	}
	else {
                DESTDIR = ../../bin/release
	}
}

#DEFINES += FMI2_FUNCTION_PREFIX=Math003Part3_

unix|mac {
	VER_MAJ = 2
	VER_MIN = 0
	VER_PAT = 0
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
}

INCLUDEPATH = src

SOURCES += \
	src/fmi2common/fmi2Functions.cpp \
	src/fmi2common/InstanceData.cpp \
	src/Math003Part3.cpp

HEADERS += \
	src/fmi2common/fmi2Functions.h \
	src/fmi2common/fmi2Functions_complete.h \
	src/fmi2common/fmi2FunctionTypes.h \
	src/fmi2common/fmi2TypesPlatform.h \
	src/fmi2common/InstanceData.h \
	src/Math003Part3.h


