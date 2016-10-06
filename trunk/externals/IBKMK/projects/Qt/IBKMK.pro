# -----------------------
# Project for IBK library
# -----------------------

# first we define what we are
TARGET = IBKMK
TEMPLATE = lib

# this is the central configuration file for all IBK dependent libraries
# we check if this file was created by our build tool helper python what ever
!include( ../../../IBK/projects/Qt/CONFIG.pri ){
	message( "No custom build options specified" )
}

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../../../IBK/projects/Qt/IBK.pri )

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
		-lsundials \
		-lIBK


INCLUDEPATH = \
	../../../IBK/src


SOURCES += \
	../../src/IBKMK_BandMatrix.cpp \
	../../src/IBKMK_BlockBandMatrix.cpp \
	../../src/IBKMK_BlockSparseMatrix.cpp \
	../../src/IBKMK_BlockTridiagMatrix.cpp \
	../../src/IBKMKC_band_matrix.c \
	../../src/IBKMKC_dense_matrix.c \
	../../src/IBKMK_CRSpline.cpp \
	../../src/IBKMKC_sparse_matrix.c \
	../../src/IBKMKC_tridiag_matrix.c \
	../../src/IBKMK_CuthillMcKee.cpp \
	../../src/IBKMKC_vector_operations.c \
	../../src/IBKMK_DenseMatrix.cpp \
	../../src/IBKMK_IndexGenerator.cpp \
	../../src/IBKMK_numerics.cpp \
	../../src/IBKMK_SparseMatrixCSR.cpp \
	../../src/IBKMK_SparseMatrixEID.cpp \
	../../src/IBKMK_SparseMatrixPattern.cpp \
	../../src/IBKMK_TridiagMatrix.cpp


HEADERS += \
	../../src/IBKMK_BandMatrix.h \
	../../src/IBKMK_BlockBandMatrix.h \
	../../src/IBKMK_BlockSparseMatrix.h \
	../../src/IBKMK_BlockTridiagMatrix.h \
	../../src/IBKMK_BlockVector.h \
	../../src/IBKMKC_band_matrix.h \
	../../src/IBKMKC_dense_matrix.h \
	../../src/IBKMK_common_defines.h \
	../../src/IBKMK_CRSpline.h \
	../../src/IBKMKC_sparse_matrix.h \
	../../src/IBKMKC_tridiag_matrix.h \
	../../src/IBKMK_CuthillMcKee.h \
	../../src/IBKMKC_vector_operations.h \
	../../src/IBKMK_DenseMatrix.h \
	../../src/IBKMK.h \
	../../src/IBKMK_IndexGenerator.h \
	../../src/IBKMK_minimization.h \
	../../src/IBKMK_numerics.h \
	../../src/IBKMK_random.h \
	../../src/IBKMK_rational_number.h \
	../../src/IBKMK_SparseMatrixCSR.h \
	../../src/IBKMK_SparseMatrixEID.h \
	../../src/IBKMK_SparseMatrix.h \
	../../src/IBKMK_SparseMatrixPattern.h \
	../../src/IBKMK_TridiagMatrix.h \
	../../src/IBKMK_Vector3D.h


OTHER_FILES +=

DISTFILES += \
	../../doc/IBKMK_mainpage

