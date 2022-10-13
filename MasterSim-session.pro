# Project for MasterSim session and all its libraries

TEMPLATE=subdirs

# SUBDIRS lists all subprojects
SUBDIRS += MasterSim \
	MasterSimulator \
	MasterSimulatorUI \
	BlockMod \
	IBK \
	IBKMK \
	minizip \
	TiCPP \
	FMUTestMath003Part1 \
	FMUTestMath003Part2 \
	FMUTestMath003Part3 \
	FMUTestLotkaVolterraPrey \
	FMUTestLotkaVolterraPredator

win32|macx {
	SUBDIRS += zlib
	zlib.file = externals/zlib/zlib.pro
}

# where to find the sub projects
#MasterSimulator.file = MasterSimulator/MasterSimulator.pro
#MasterSim.file = MasterSim/MasterSim.pro
#MasterSimulatorUI.file = MasterSimulatorUI/MasterSimulatorUI.pro

BlockMod.file = externals/BlockMod/BlockMod.pro
IBK.file = externals/IBK/IBK.pro
IBKMK.file = externals/IBKMK/IBKMK.pro
minizip.file = externals/minizip/minizip.pro
TiCPP.file = externals/TiCPP/TiCPP.pro

FMUTestMath003Part1.file = TestFMUs/Math003Part1/Math003Part1.pro
FMUTestMath003Part2.file = TestFMUs/Math003Part2/Math003Part2.pro
FMUTestMath003Part3.file = TestFMUs/Math003Part3/Math003Part3.pro
FMUTestLotkaVolterraPrey.file = TestFMUs/LotkaVolterraPrey/LotkaVolterraPrey.pro
FMUTestLotkaVolterraPredator.file = TestFMUs/LotkaVolterraPredator/LotkaVolterraPredator.pro

# dependencies
MasterSim.depends = IBK IBKMK TiCPP minizip
win32|macx {
	MasterSim.depends += zlib
	minizip.depends += zlib
}

IBKMK.depends = IBK
BlockMod.depends = IBK
TiCPP.depends = IBK

MasterSimulator.depends = MasterSim IBK IBKMK TiCPP minizip
MasterSimulatorUI.depends = MasterSim BlockMod IBK IBKMK TiCPP minizip

FMUTestMath003Part1.depends = MasterSimulator
FMUTestMath003Part2.depends = MasterSimulator
FMUTestMath003Part3.depends = MasterSimulator
FMUTestLotkaVolterraPrey.depends = MasterSimulator
FMUTestLotkaVolterraPredator.depends = MasterSimulator

