# -------------------------------------------------
# Project for TICPP library
# -------------------------------------------------
TARGET = TiCPP
TEMPLATE = lib

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../IBK/IBK.pri )

unix:VERSION = 1.1.0

LIBS += -lIBK

INCLUDEPATH += \
	../IBK/src


SOURCES += \
	src/tinystr.cpp \
	src/tinyxml.cpp \
	src/tinyxmlerror.cpp \
	src/tinyxmlparser.cpp

HEADERS += \
	src/ticppIBKconfig.h \
	src/tinystr.h \
	src/tinyxml.h
