# -------------------------------------------------
# Project for zlib library
# -------------------------------------------------

# first we define what we are
win32 {
	TARGET = zlib
}
else {
	TARGET = z
}

TEMPLATE = lib

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../IBK/projects/Qt/IBK.pri )

# finally we setup our custom library specfic things
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
