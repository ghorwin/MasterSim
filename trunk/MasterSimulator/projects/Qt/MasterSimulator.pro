# Project file for MasterSimulator
# remember to set DYLD_FALLBACK_LIBRARY_PATH on MacOSX
# set LD_LIBRARY_PATH on Linux

TARGET = MasterSimulator
TEMPLATE = app

# this pri must be sourced from all our applications
include( ../../../externals/IBK/projects/Qt/IBK.pri )

LIBS += -L../../../lib$${DIR_PREFIX} \
	-L../../../externals/lib$${DIR_PREFIX} \
	-lMasterSim \
	-lminizip \
	-lz \
	-lIBKMKmini \
	-lIBK \
	-lTiCPP

unix {
	LIBS += -ldl
}

INCLUDEPATH = \
	../../../MasterSim/src \
	../../../externals/IBK/src

SOURCES += \
	../../src/main.cpp

