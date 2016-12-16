# -------------------------------------------------
# Project for zlib library
# -------------------------------------------------

# first we define what we are
TARGET = z

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

#zlib has special folder structure thus we override
#DESTDIR = ../../../lib

# finally we setup our custom library specific things
# like version number etc., we also may reset all
unix|mac {
	VER_MAJ = 1
	VER_MIN = 2
	VER_PAT = 8
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_AT}
}

INCLUDEPATH = \
		../../src/

SOURCES += \
	../../src/adler32.c \
	../../src/compress.c \
	../../src/crc32.c \
	../../src/deflate.c \
	../../src/gzclose.c \
	../../src/gzlib.c \
	../../src/gzread.c \
	../../src/gzwrite.c \
	../../src/inflate.c \
	../../src/infback.c \
	../../src/inftrees.c \
	../../src/inffast.c \
	../../src/trees.c \
	../../src/uncompr.c \
	../../src/zutil.c

HEADERS += \
	../../src/crc32.h \
	../../src/deflate.h \
	../../src/gzguts.h \
	../../src/inffast.h \
	../../src/inffixed.h \
	../../src/inflate.h \
	../../src/inftrees.h \
	../../src/trees.h \
	../../src/zlib.h \
	../../src/zutil.h
