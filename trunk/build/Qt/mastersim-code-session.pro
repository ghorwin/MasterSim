# Project for MasterSim session and all its libraries

TEMPLATE=subdirs

# SUBDIRS lists all subprojects
SUBDIRS += MasterSim \
	MasterSimulator \
	MasterSimulatorUI \
	BlockMod \
	QtPropertyBrowser \
	DataIO \
	IBK \
	IBKMK \
	minizip \
	TiCPP \
	FMUTestMath003Part1 \
	FMUTestMath003Part2 \
	FMUTestMath003Part3 \
	FMUTestLotkaVolterraPrey \
	FMUTestLotkaVolterraPredator

WIN32 {
	SUBDIRS += zlib
	zlib.file = ../../externals/zlib/projects/Qt/zlib.pro
}

# where to find the sub projects
MasterSimulator.file = ../../MasterSimulator/projects/Qt/MasterSimulator.pro
MasterSim.file = ../../MasterSim/projects/Qt/MasterSim.pro
MasterSimulatorUI.file = ../../MasterSimulatorUI/projects/Qt/MasterSimulatorUI.pro

BlockMod.file = ../../externals/BlockMod/projects/Qt/BlockMod.pro
QtPropertyBrowser.file = ../../externals/QtPropertyBrowser/projects/Qt/QtPropertyBrowser.pro
DataIO.file = ../../externals/DataIO/projects/Qt/DataIO.pro
IBK.file = ../../externals/IBK/projects/Qt/IBK.pro
IBKMK.file = ../../externals/IBKMK/projects/Qt/IBKMK.pro
minizip.file = ../../externals/minizip/projects/Qt/minizip.pro
TiCPP.file = ../../externals/TiCPP/projects/Qt/TiCPP.pro

FMUTestMath003Part1.file = ../../TestFMUs/Math003Part1/projects/Qt/Math003Part1.pro
FMUTestMath003Part2.file = ../../TestFMUs/Math003Part2/projects/Qt/Math003Part2.pro
FMUTestMath003Part3.file = ../../TestFMUs/Math003Part3/projects/Qt/Math003Part3.pro
FMUTestLotkaVolterraPrey.file = ../../TestFMUs/LotkaVolterraPrey/projects/Qt/LotkaVolterraPrey.pro
FMUTestLotkaVolterraPredator.file = ../../TestFMUs/LotkaVolterraPredator/projects/Qt/LotkaVolterraPredator.pro

# dependencies
MasterSim.depends = IBK IBKMK TiCPP DataIO minizip
WIN32 {
	MasterSim.depends += zlib
}

DataIO.depends = IBK
IBKMK.depends = IBK
BlockMod.depends = IBK

MasterSimulator.depends = MasterSim IBK IBKMK TiCPP DataIO minizip
MasterSimulatorUI.depends = MasterSim BlockMod QtPropertyBrowser IBK IBKMK TiCPP DataIO minizip

FMUTestMath003Part1.depends = MasterSimulator
FMUTestMath003Part2.depends = MasterSimulator
FMUTestMath003Part3.depends = MasterSimulator
FMUTestLotkaVolterraPrey.depends = MasterSimulator
FMUTestLotkaVolterraPredator.depends = MasterSimulator


