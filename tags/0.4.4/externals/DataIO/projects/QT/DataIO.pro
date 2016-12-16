# -------------------------------------------------
# Project for DataIO library
# -------------------------------------------------
TARGET = DataIO
TEMPLATE = lib


# this is the central configuration file for all IBK dependent libraries
# we check if this file was created by our build tool helper python what ever
!include( ../../../IBK/projects/Qt/CONFIG.pri ){
	message( "No custom build options specified" )
}

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../IBK/projects/Qt/IBK.pri )

unix|mac {
	VER_MAJ = 7
	VER_MIN = 1
	VER_PAT = 0
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
}

LIBS += -L../../../lib \
	-lIBK

INCLUDEPATH += \
	../../../IBK/src
SOURCES += ../../src/DATAIO_DataIO.cpp \
	../../src/DATAIO_Utils.cpp \
	../../src/DATAIO_GeoFile.cpp \
	../../src/DATAIO_Constants.cpp \
	../../src/DATAIO_TextNotificationHandler.cpp

HEADERS += \
	../../src/DATAIO_DataIO.h \
	../../src/DATAIO_Utils.h \
	../../src/DATAIO_GeoFile.h \
	../../src/DATAIO_Constants.h \
	../../src/DATAIO_TextNotificationHandler.h \
	../../src/DataIO

OTHER_FILES += \
	../../doc/footer.html \
	../../doc/header.html \
	../../doc/IBK-doxygen.css
