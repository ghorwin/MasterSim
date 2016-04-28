# -------------------------------------------------
# Project for TICPP library
# -------------------------------------------------
TARGET = TiCPP

# this is the central configuration file for all IBK dependent libraries
# we check if this file was created by our build tool helper python what ever
!include( ../../../IBK/projects/Qt/CONFIG.pri ){
	message( "No custom build options specified" )
}

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../IBK/projects/Qt/IBK.pri )

# This MUST be done after pri is included
TEMPLATE = lib

unix|mac {
	VER_MAJ = 1
	VER_MIN = 0
	VER_PAT = 0
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
}

SOURCES += \
	../../src/tinyxmlparser.cpp \
	../../src/tinyxmlerror.cpp \
	../../src/tinyxml.cpp \
	../../src/tinystr.cpp
HEADERS += \
	../../src/tinyxml.h \
	../../src/tinystr.h \
	../../src/ticppIBKconfig.h
