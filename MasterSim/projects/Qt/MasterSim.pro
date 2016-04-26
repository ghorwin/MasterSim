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

INCLUDEPATH = \
	../../../externals/minizip/src \
	../../../externals/IBK/src \
	../../../externals/DataIO/src

LIBS += -L../../../externals/lib

LIBS += \
	-lDataIO \
	-lIBK \
	-lminizip \
	-lz

SOURCES += \
	../../src/MSIM_MasterSim.cpp \
	../../src/MSIM_Project.cpp \
	../../src/MSIM_Simulator.cpp \
	../../src/MSIM_ArgParser.cpp \
	../../src/MSIM_Constants.cpp \
	../../src/MSIM_FMUManager.cpp \
	../../src/MSIM_FMUSlave.cpp \
    ../../src/MSIM_FMU.cpp

HEADERS += \
	../../src/MSIM_MasterSim.h \
	../../src/MSIM_Project.h \
	../../src/MSIM_Simulator.h \
	../../src/MSIM_ArgParser.h \
	../../src/MSIM_Constants.h \
	../../src/MSIM_FMUManager.h \
	../../src/MSIM_FMUSlave.h \
    ../../src/MSIM_FMU.h
