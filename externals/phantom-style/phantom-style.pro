# ----------------------------------------------------
# Project for PhantomStyle library (Qt widget style)
# ----------------------------------------------------

TARGET = PhantomStyle
TEMPLATE = lib

# common project configurations, source this file after TEMPLATE was specified
include( ../IBK/IBK.pri )

QT += core gui widgets

INCLUDEPATH = \
	src

HEADERS += \
	src/phantomcolor.h \
	src/phantomstyle.h \
	src/phantomtweak.h

SOURCES += \
	src/phantomcolor.cpp \
	src/phantomstyle.cpp
