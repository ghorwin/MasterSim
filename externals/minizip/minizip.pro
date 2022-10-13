# -------------------------------------------------
# Project for minizip library
# -------------------------------------------------
TARGET = minizip
TEMPLATE = lib

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../IBK/projects/Qt/IBK.pri )

# finally we setup our custom library specific things
# like version number etc., we also may reset all
unix|mac {
	VER_MAJ = 1
	VER_MIN = 2
	VER_PAT = 5
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
}

macx {
	LIBS += -lz
}

win32 {
	LIBS += lzlib
}

INCLUDEPATH += \
	../../../zlib/src

SOURCES += \
  ../../src/ioapi.c \
  ../../src/miniunz.c \
  ../../src/minizip.c \
  ../../src/unzip.c \
  ../../src/zip.c

win32 {
	SOURCES += ../../src/iowin32.c
}

HEADERS += \
	../../src/crypt.h \
	../../src/ioapi.h \
	../../src/miniunz.h \
	../../src/minizip.h \
	../../src/mztools.h \
	../../src/unzip.h \
	../../src/zip.h


