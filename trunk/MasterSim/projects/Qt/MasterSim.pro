# -------------------------------------------------
# Project for MasterSim library
# -------------------------------------------------
TARGET = MasterSim

# this is the central configuration file for all IBK dependent libraries
# we check if this file was created by our build tool helper python what ever
!include( ../../../externals/IBK/projects/Qt/CONFIG.pri ){
	message( "No custom build options specified" )
}

# this pri must be sourced from all our applications
include( ../../../externals/IBK/projects/Qt/IBK.pri )

# This MUST be done after pri is included
TEMPLATE = lib

unix|mac {
	VER_MAJ = 0
	VER_MIN = 1
	VER_PAT = 0
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
}

LIBS += -L../../../externals/lib \
	-lIBK

INCLUDEPATH = \
	../../../externals/IBK/src \
	../../../externals/DataIO/src

SOURCES += \
	../../src/MasterSim.cpp \
    ../../src/Project.cpp

HEADERS += \
	../../src/MasterSim.h \
    ../../src/Project.h
