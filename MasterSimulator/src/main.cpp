#include <iostream>
#include <cstdlib>

#include <IBK_WaitOnExit.h>
#include <IBK_ArgParser.h>
#include <IBK_Exception.h>

#include <MasterSim.h>

int main(int argc, char * argv[]) {
	IBK::WaitOnExit wait(true);


	// parse command line

	IBK::ArgParser parser;
	parser.addOption(0, "working-dir", "Working directory for master.", "working-directory", "/var/tmp");
	parser.parse(argc, argv);

	IBK::Path workingDirRoot = IBK::Path(parser.option("working-dir"));

	IBK::Path stateDir = workingDirRoot / "states";

	// help and man-page
	if (parser.handleDefaultFlags(std::cout))
		return EXIT_SUCCESS;

	if (parser.args().size() < 2) {
		std::cerr << "Syntax: MasterSim <project file>" << std::endl;
		return EXIT_FAILURE;
	}

	try {

		// instantiate sim project
		MASTER_SIM::MasterSimulator masterSim;

		// read project file
		masterSim.readProjectFile(IBK::Path(parser.args()[1]));

		// initialize all FMUs (e.g. load dlls/shared libs, parse ModelDescription, do error checking
		masterSim.instantiateFMUs( workingDirRoot );

#if HAVE_SERIALIZATION_CODE
		// set master and all FMUs to start time point
		double tStart = masterSim.tStart(); // override with command line argument
		masterSim.restoreState(tStart, stateDir);
		double t = masterSim.currentTime();
#else
		// run master for entire simulation
		double t = 0;
		masterSim.initialize();
		masterSim.writeOutputs();
#endif

		double tEnd = masterSim.tEnd(); // override with command line argument
		while (t < tEnd) {
			// ask master to do an internal step
			masterSim.doStep();
			t = masterSim.tCurrent();

			// handle outputs (filtering implemented inside writeOutputs()).
			masterSim.writeOutputs();
		}

	}
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}


