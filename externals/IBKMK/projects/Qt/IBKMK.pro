# -------------------------
# Project for IBKMK library
# -------------------------

# first we define what we are
TARGET = IBKMK

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

# finally we setup our custom library specfic things
# like version number etc., we also may reset all
#
unix|mac {
	VER_MAJ = 1
	VER_MIN = 0
	VER_PAT = 0
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
}


LIBS += -L../../../lib \
		-lIBK


INCLUDEPATH = \
	../../../IBK/src


SOURCES += \
	../../src/IBKMK_DenseMatrix.cpp \
	../../src/IBKMKC_dense_matrix.c


HEADERS += \
	../../src/IBKMK_DenseMatrix.h \
	../../src/IBKMKC_dense_matrix.h


