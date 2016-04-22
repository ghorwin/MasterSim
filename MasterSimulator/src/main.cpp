#include <iostream>
#include <cstdlib>

#include <IBK_WaitOnExit.h>
#include <IBK_SolverArgsParser.h>
#include <IBK_Exception.h>

#include <MSIM_MasterSim.h>
#include <MSIM_Project.h>
#include <MSIM_ArgParser.h>

int main(int argc, char * argv[]) {

	// parse command line
	MASTER_SIM::ArgParser parser;
	parser.parse(argc, argv);

	// help and man-page
	if (parser.handleDefaultFlags(std::cout))
		return EXIT_SUCCESS;

	if (!parser.m_projectFile.isValid()) {
		std::cerr << "Missing project file argument. Use --help for syntax." << std::endl;
		return EXIT_FAILURE;
	}

	IBK::WaitOnExit wait(parser.flagEnabled('x'));


	try {

		// instantiate project
		MASTER_SIM::Project project;

		// read project file
		project.read(IBK::Path(parser.m_projectFile ));

		// create simulator
		MASTER_SIM::MasterSimulator masterSim;
		// initialize all FMUs (e.g. load dlls/shared libs, parse ModelDescription, do error checking
		masterSim.instantiateFMUs(project, parser.m_workingDir);

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

		double tEnd = project.m_tEnd; // override with command line argument
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


