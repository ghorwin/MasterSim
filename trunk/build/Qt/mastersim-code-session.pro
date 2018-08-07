# Project for MasterSim session and all its libraries

TEMPLATE=subdirs

# SUBDIRS lists all subprojects
SUBDIRS += MasterSim \
	MasterSimulator \
	MasterSimulatorUI \
	BlockMod \
	DataIO \
	IBK \
	IBKMK \
	minizip \
	TiCPP \
	zlib \
	FMUTestMath003Part1 \
	FMUTestMath003Part2 \
	FMUTestMath003Part3

# where to find the sub projects
MasterSimulator.file = ../../MasterSimulator/projects/Qt/MasterSimulator.pro
MasterSim.file = ../../MasterSim/projects/Qt/MasterSim.pro
MasterSimulatorUI.file = ../../MasterSimulatorUI/projects/Qt/MasterSimulatorUI.pro

BlockMod.file = ../../externals/BlockMod/projects/Qt/BlockMod.pro
DataIO.file = ../../externals/DataIO/projects/Qt/DataIO.pro
IBK.file = ../../externals/IBK/projects/Qt/IBK.pro
IBKMK.file = ../../externals/IBKMK/projects/Qt/IBKMK.pro
minizip.file = ../../externals/minizip/projects/Qt/minizip.pro
TiCPP.file = ../../externals/TiCPP/projects/Qt/TiCPP.pro
zlib.file = ../../externals/zlib/projects/Qt/zlib.pro

FMUTestMath003Part1.file = ../../TestFMUs/Math003Part1/projects/Qt/Math003Part1.pro
FMUTestMath003Part2.file = ../../TestFMUs/Math003Part2/projects/Qt/Math003Part2.pro
FMUTestMath003Part3.file = ../../TestFMUs/Math003Part3/projects/Qt/Math003Part3.pro

# dependencies
MasterSim.depends = IBK IBKMK TiCPP DataIO minizip zlib

DataIO.depends = IBK
IBKMK.depends = IBK
BlockMod.depends = IBK

MasterSimulator.depends = MasterSim
MasterSimulatorUI.depends = MasterSim BlockMod

FMUTestMath003Part1.depends = MasterSimulator
FMUTestMath003Part2.depends = MasterSimulator
FMUTestMath003Part3.depends = MasterSimulator

win32 {
#	SUBDIRS += EmfEngine
	# where to find the sub projects
#	EmfEngine.file = ../../externals/EmfEngine/projects/Qt/EmfEngine.pro
#	PostProcApp.depends += EmfEngine
}

