#include <iostream>
#include <cstdlib>

#include <IBK_WaitOnExit.h>
#include <IBK_SolverArgsParser.h>
#include <IBK_Exception.h>

#include <MasterSim.h>
#include <Project.h>

int main(int argc, char * argv[]) {
	IBK::WaitOnExit wait(true);


	// parse command line

	IBK::SolverArgsParser parser;
	parser.addOption(0, "working-dir", "Working directory for master.", "working-directory", "Parent path of project file.");
	parser.parse(argc, argv);

	// help and man-page
	if (parser.handleDefaultFlags(std::cout))
		return EXIT_SUCCESS;

	if (parser.args().size() < 2) {
		std::cerr << "Missing project file argument. Use --help for syntax." << std::endl;
		return EXIT_FAILURE;
	}

	// get project file
	IBK::Path projectFile = parser.args()[1];

	// get working directory (defaults to project file path)
	IBK::Path workingDir = projectFile.parentPath();

	// if invalid working dir (project file path is relative) use current directory
	if (!workingDir.isValid())
		workingDir = IBK::Path::current();
	// override with command line option
	if (parser.hasOption("working-dir"))
		workingDir = IBK::Path(parser.option("working-dir"));

	// get states subdirectory
	IBK::Path stateDir = workingDir / "states";

	try {

		// instantiate project

		MASTER_SIM::Project project;

		// read project file
		project.read(IBK::Path(parser.args()[1]));


		MASTER_SIM::MasterSimulator masterSim;
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


