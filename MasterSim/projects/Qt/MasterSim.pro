# -------------------------------------------------
# Project for MasterSim library
# -------------------------------------------------
TARGET = MasterSim
TEMPLATE = lib

OPTIONS += top_level_libs

# this pri must be sourced from all our applications
include( ../../../externals/IBK/projects/Qt/IBK.pri )

unix|mac {
	VER_MAJ = 0
	VER_MIN = 1
	VER_PAT = 0
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
}

QMAKE_CXXFLAGS +=  -std=c++11

INCLUDEPATH = \
	../../../externals/minizip/src \
	../../../externals/IBK/src \
	../../../externals/IBKMK/src \
	../../../externals/DataIO/src \
	../../../externals/TiCPP/src

LIBS += \
	-lDataIO \
	-lIBKMK \
	-lIBK \
	-lTiCPP \
	-lminizip \
	-lz

SOURCES += \
	../../src/MSIM_MasterSim.cpp \
	../../src/MSIM_Project.cpp \
	../../src/MSIM_ArgParser.cpp \
	../../src/MSIM_Constants.cpp \
	../../src/MSIM_FMUManager.cpp \
	../../src/MSIM_Slave.cpp \
	../../src/MSIM_FMU.cpp \
	../../src/MSIM_ModelDescription.cpp \
	../../src/MSIM_FMIVariable.cpp \
	../../src/MSIM_AlgorithmGaussJacobi.cpp \
	../../src/MSIM_AlgorithmGaussSeidel.cpp \
	../../src/MSIM_AlgorithmNewton.cpp \
	../../src/MSIM_FMIType.cpp \
	../../src/MSIM_OutputWriter.cpp \
	../../src/MSIM_ProgressFeedback.cpp

HEADERS += \
	../../src/MSIM_MasterSim.h \
	../../src/MSIM_Project.h \
	../../src/MSIM_ArgParser.h \
	../../src/MSIM_Constants.h \
	../../src/MSIM_FMUManager.h \
	../../src/MSIM_Slave.h \
	../../src/MSIM_FMU.h \
	../../src/MSIM_ModelDescription.h \
	../../src/MSIM_FMIVariable.h \
	../../src/fmi/fmi2Functions.h \
	../../src/fmi/fmi2FunctionTypes.h \
	../../src/fmi/fmi2TypesPlatform.h \
	../../src/fmi/fmiFunctions.h \
	../../src/fmi/fmiModelTypes.h \
	../../src/fmi/fmiPlatformTypes.h \
	../../src/MSIM_AlgorithmGaussJacobi.h \
	../../src/MSIM_AlgorithmGaussSeidel.h \
	../../src/MSIM_AbstractAlgorithm.h \
	../../src/MSIM_AlgorithmNewton.h \
	../../src/MSIM_FMIType.h \
	../../src/MSIM_OutputWriter.h \
	../../src/MSIM_ProgressFeedback.h
