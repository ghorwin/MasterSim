#############################################
#
# Actual evironment configuration routines (kept in repository)
#
#############################################

# This section contains all mpi specific settings. It can be enabled
# by adding the option 'mpi' to the CONFIG environment variable.
# It defines USE_MPICH which can be utilized to create MPI code inside
# your ordinary serial code.
#
# creates an environment for mpi compilation
# mpicc must be found via systems executable path
# linux install dir is set to /opt/mpich
# windows must be added
#
contains( OPTIONS, mpi ) {

	message(Setting up MPICH support.)

	#setup compiler
	QMAKE_CXX = mpicxx
	QMAKE_CXX_RELEASE = $$QMAKE_CXX
	QMAKE_CXX_DEBUG = $$QMAKE_CXX
	QMAKE_LINK = $$QMAKE_CXX
	QMAKE_CC = mpicc

	# setup linker
	QMAKE_LFLAGS += -FPIC $$system(mpicxx --showme:link)
	QMAKE_CFLAGS += $$system(mpicc --showme:compile)
	QMAKE_CXXFLAGS += $$system(mpicxx --showme:compile) -DMPICH_IGNORE_CXX_SEEK
	QMAKE_CXXFLAGS_RELEASE += $$system(mpicxx --showme:compile) -DMPICH_IGNORE_CXX_SEEK

	# setup define for actual source code
	 DEFINES += USE_MPICH


	unix {
		INCLUDEPATH += "/opt/mpich/include"
	}

	win32 {
		INCLUDEPATH += "/opt/mpich/include"
	}
}


#
# This option eables explicit mpi + icc support
#
contains( OPTIONS, mpiICC ) {

	message(Setting up MPICH+ICC support.)

	#setup compiler
	QMAKE_CXX = /opt/mpich_icc/bin/mpicxx
	QMAKE_CXX_RELEASE = $$QMAKE_CXX
	QMAKE_CXX_DEBUG = $$QMAKE_CXX
	QMAKE_LINK = $$QMAKE_CXX
	QMAKE_CC = /opt/mpich_icc/bin/mpicc

	# setup linker
	QMAKE_LFLAGS += -FPIC $$system(/opt/mpich_icc/bin/mpicxx --showme:link)
	QMAKE_CFLAGS += $$system(/opt/mpich_icc/bin/mpicc --showme:compile)
	QMAKE_CXXFLAGS += $$system(/opt/mpich_icc/bin/mpicxx --showme:compile) -DMPICH_IGNORE_CXX_SEEK
	QMAKE_CXXFLAGS_RELEASE += $$system(/opt/mpich_icc/bin/mpicxx --showme:compile) -DMPICH_IGNORE_CXX_SEEK

	# setup define for actual source code
	 DEFINES += USE_MPICH

	unix {
		INCLUDEPATH += "/opt/mpich_icc/include"
	}

	win32 {
		INCLUDEPATH += "/opt/mpich_icc/include"
	}

}



#
# This option eables explicit mpi + gcc support
#
contains( OPTIONS, mpiGCC ) {

	message(Setting up MPICH+GCC support.)

	#setup compiler
	QMAKE_CXX = /opt/mpich_gcc/bin/mpicxx
	QMAKE_CXX_RELEASE = $$QMAKE_CXX
	QMAKE_CXX_DEBUG = $$QMAKE_CXX
	QMAKE_LINK = $$QMAKE_CXX
	QMAKE_CC = /opt/mpich_gcc/bin/mpicc

	# setup linker
	QMAKE_LFLAGS += -FPIC $$system(/opt/mpich_gcc/bin/mpicxx --showme:link)
	QMAKE_CFLAGS += $$system(/opt/mpich_gcc/bin/mpicc --showme:compile)
	QMAKE_CXXFLAGS += $$system(/opt/mpich_gcc/bin/mpicxx --showme:compile) -DMPICH_IGNORE_CXX_SEEK
	QMAKE_CXXFLAGS_RELEASE += $$system(/opt/mpich_gcc/bin/mpicxx --showme:compile) -DMPICH_IGNORE_CXX_SEEK

	# setup define for actual source code
	 DEFINES += USE_MPICH

	unix {
		INCLUDEPATH += "/opt/mpich_gcc/include"
	}

	win32 {
		INCLUDEPATH += "/opt/mpich_gcc/include"
	}

}


#
# openmp settings added for gcc and min-gw
#
contains( OPTIONS, openmp ) {

	message(Setting up OpenMP support)

	#setup linker and compiler flags
	CONFIG(debug, debug|release) {
		QMAKE_CFLAGS = -fopenmp -fPIC -march=core-avx-i -mtune=core-avx-i
	} else {
		QMAKE_CFLAGS = -fopenmp -fPIC -O3 -march=core-avx-i -mtune=core-avx-i #-Ofast
	}
	QMAKE_LFLAGS = $$QMAKE_CFLAGS
	QMAKE_CXXFLAGS = $$QMAKE_CFLAGS
	QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS

	# all OpenMP implementations define a _OPENMP symbol
}
else {

	message(No OpenMP support)
	unix {
		message(OpenMP warnings disabled)
		QMAKE_CFLAGS += -Wno-unknown-pragmas
		QMAKE_CXXFLAGS += -Wno-unknown-pragmas
	}

}


#
# This option eables explicit vampire trace + mpi + icc support
#
contains( OPTIONS, vapireTraceICC ) {

	message(Setting up TRACE+ICC support.)

	#setup compiler
	QMAKE_CXX = /opt/vampirTrace_icc/bin/vtcxx
	QMAKE_CXX_RELEASE = $$QMAKE_CXX
	QMAKE_CXX_DEBUG = $$QMAKE_CXX
	QMAKE_LINK = $$QMAKE_CXX
	QMAKE_CC = /opt/vampirTrace_icc/bin/vtcc

	QMAKE_CFLAGS += -DVTRACE
	QMAKE_LFLAGS += $$QMAKE_CFLAGS
	QMAKE_CXXFLAGS += $$QMAKE_CFLAGS
	QMAKE_CXXFLAGS_RELEASE += $$QMAKE_CFLAGS

	# setup define for actual source code (manual instrumentation)
	DEFINES += USE_VAMPIRE

	unix {
		INCLUDEPATH += "/opt/vampirTrace_icc/include /opt/vampirTrace_icc/include/vampirtrace"
#		LIBS += -L/opt/vampirTrace_icc/lib \
#				-l \
	}

	win32 {
		INCLUDEPATH += "/opt/vampirTrace_icc/include  /opt/vampirTrace_icc/include/vampirtrace"
#		LIBS += -L/opt/vampirTrace_icc/lib \
#				-l \
	}

}



#
# This option eables explicit vampire trace + mpi + gcc support
#
contains( OPTIONS, vapireTraceGCC ) {

	message(Setting up TRACE+GCC support.)

	#setup compiler
	QMAKE_CXX = /opt/vampirTrace_gcc/bin/vtcxx
	QMAKE_CXX_RELEASE = $$QMAKE_CXX
	QMAKE_CXX_DEBUG = $$QMAKE_CXX
	QMAKE_LINK = $$QMAKE_CXX
	QMAKE_CC = /opt/vampirTrace_gcc/bin/vtcc

	QMAKE_CFLAGS += -DVTRACE
	QMAKE_LFLAGS += $$QMAKE_CFLAGS
	QMAKE_CXXFLAGS += $$QMAKE_CFLAGS
	QMAKE_CXXFLAGS_RELEASE += $$QMAKE_CFLAGS


	# setup define for actual source code (manual instrumentation)
	DEFINES += USE_VAMPIRE

	unix {
		INCLUDEPATH += "/opt/vampirTrace_gcc/include /opt/vampirTrace_gcc/include/vampirtrace"
#		LIBS += -L/opt/vampirTrace_gcc/lib \
#				-l \
	}

	win32 {
		INCLUDEPATH += "/opt/vampirTrace_gcc/include  /opt/vampirTrace_gcc/include/vampirtrace"
#		LIBS += -L/opt/vampirTrace_gcc/lib \
#				-l \
	}

}


contains( OPTIONS, openmpVapireTraceICC ){

	message(Setting up OPENMP+TRACE+ICC support.)

	#setup compiler
	QMAKE_CXX = /opt/vampirTrace_icc/bin/vtcxx
	QMAKE_CXX_RELEASE = $$QMAKE_CXX
	QMAKE_CXX_DEBUG = $$QMAKE_CXX
	QMAKE_LINK = $$QMAKE_CXX
	QMAKE_CC = /opt/vampirTrace_icc/bin/vtcc

	QMAKE_CFLAGS += -fopenmp -fPIC -DVTRACE
	QMAKE_LFLAGS += $$QMAKE_CFLAGS
	QMAKE_CXXFLAGS += $$QMAKE_CFLAGS
	QMAKE_CXXFLAGS_RELEASE += $$QMAKE_CFLAGS

	# setup define for actual source code (manual instrumentation)
	DEFINES += USE_VAMPIRE

	unix {
		INCLUDEPATH += "/opt/vampirTrace_icc/include /opt/vampirTrace_icc/include/vampirtrace"
#		LIBS += -L/opt/vampirTrace_icc/lib \
#				-l \
	}

	win32 {
		INCLUDEPATH += "/opt/vampirTrace_icc/include  /opt/vampirTrace_icc/include/vampirtrace"
#		LIBS += -L/opt/vampirTrace_icc/lib \
#				-l \
	}
}

contains( OPTIONS, openmpVapireTraceGCC ){

	message(Setting up OPENMP+TRACE+GCC support.)

	#setup compiler
	QMAKE_CXX = /opt/vampirTrace_gcc/bin/vtcxx
	QMAKE_CXX_RELEASE = $$QMAKE_CXX
	QMAKE_CXX_DEBUG = $$QMAKE_CXX
	QMAKE_LINK = $$QMAKE_CXX
	QMAKE_CC = /opt/vampirTrace_gcc/bin/vtcc

	QMAKE_CFLAGS += -fopenmp -fPIC -O2
	QMAKE_LFLAGS += $$QMAKE_CFLAGS
	QMAKE_CXXFLAGS += $$QMAKE_CFLAGS
	QMAKE_CXXFLAGS_RELEASE += $$QMAKE_CFLAGS

	# setup define for actual source code (manual instrumentation)
	DEFINES += USE_VAMPIRE

	unix {
		INCLUDEPATH += "/opt/vampirTrace_gcc/include /opt/vampirTrace_gcc/include/vampirtrace"
#		LIBS += -L/opt/vampirTrace_gcc/lib \
#				-l \
	}

	win32 {
		INCLUDEPATH += "/opt/vampirTrace_gcc/include  /opt/vampirTrace_gcc/include/vampirtrace"
#		LIBS += -L/opt/vampirTrace_gcc/lib \
#				-l \
	}

}

#
# openmp settings added for gcc and min-gw
#
contains( OPTIONS, gprof ) {

	message(Setting up gprof support.)

	#setup linker and compiler flags
	QMAKE_CFLAGS += -pg
	QMAKE_LFLAGS += $$QMAKE_CFLAGS
	QMAKE_CXXFLAGS += $$QMAKE_CFLAGS
	QMAKE_CXXFLAGS_RELEASE += $$QMAKE_CFLAGS

}

# This section contains all library specific settings. It can be enabled
# by setting 'lib' the TEMPLATE environment variable.
# It defines DESTDIR, OBJECTS_DIR, DLLDESTDIR and sets shared in CONFIG
# variable to all libraries. Reset locally in when required.
equals(TEMPLATE,lib) {

	message(Setting up ordinary library support.)
	QT -=	core gui
# using of shared libs only for non MC compiler
# MS compiler needs explicite export statements in case of shared libs
	win32-msvc* {
		CONFIG += static
		DEFINES += NOMINMAX
	}
	else {
		CONFIG += shared
	}
# disable warning for unsafe functions if using MS compiler
	win32-msvc* {
		QMAKE_CXXFLAGS += /wd4996
	}
	else {
		QMAKE_CXXFLAGS += -std=c++11
	}

	DESTDIR = ../../../lib
	CONFIG(debug, debug|release) {
		OBJECTS_DIR = debug
		windows {
			contains( OPTIONS, top_level_libs ) {
				DLLDESTDIR = ../../../bin/debug
			}
			else {
				DLLDESTDIR = ../../../../bin/debug
			}
		}
		win32-msvc* {
			QMAKE_CXXFLAGS += /GS /RTC1
		}
	}
	else {
		#QMAKE_CFLAGS += -O3 #-fcx-limited-range -fno-math-errno
		OBJECTS_DIR = release
		windows {
			contains( OPTIONS, top_level_libs ) {
				DLLDESTDIR = ../../../bin/release
			}
			else {
				DLLDESTDIR = ../../../../bin/release
			}
		}
	}

	win32:LIBS += -lshell32
	win32:LIBS += -liphlpapi

}
