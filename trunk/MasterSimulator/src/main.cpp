#include <iostream>
#include <cstdlib>

#include <IBK_WaitOnExit.h>
#include <IBK_SolverArgsParser.h>
#include <IBK_Exception.h>
#include <IBK_messages.h>

#include <MSIM_MasterSim.h>
#include <MSIM_Project.h>
#include <MSIM_ArgParser.h>
#include <MSIM_Constants.h>

int main(int argc, char * argv[]) {

	// parse command line
	MASTER_SIM::ArgParser parser;
	parser.parse(argc, argv);

	// help and man-page
	if (parser.handleDefaultFlags(std::cout))
		return EXIT_SUCCESS;

	IBK::WaitOnExit wait(parser.flagEnabled('x'));

	if (parser.flagEnabled('v')) {
		std::cout << "MasterSimulator, version " << MASTER_SIM::LONG_VERSION << std::endl;
		return EXIT_SUCCESS;
	}

	if (!parser.m_projectFile.isValid()) {
		std::cerr << "Missing project file argument. Use --help for syntax." << std::endl;
		return EXIT_FAILURE;
	}

	// setup directory structure and initialize log file
	IBK::MessageHandler::setupUtf8Console();
	IBK::MessageHandler * msgHandler = IBK::MessageHandlerRegistry::instance().messageHandler();
	msgHandler->setConsoleVerbosityLevel(parser.m_verbosityLevel);
	IBK::Path logFile = parser.m_projectFile.withoutExtension();
	logFile.addExtension("log");
	std::string errmsg;
	if (!msgHandler->openLogFile(logFile.str(), false, errmsg))
		std::cerr << "Cannot write log file '" << logFile.str() << "'." << std::endl;

	try {

		// instantiate project
		MASTER_SIM::Project project;

		// read project file
		IBK::IBK_Message(IBK::FormatString("Reading project '%1'\n").arg(parser.m_projectFile), IBK::MSG_PROGRESS);
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

		if (parser.flagEnabled("test-init")) {
			IBK::IBK_Message("Stopping after successful initialization.\n", IBK::MSG_PROGRESS);
			return EXIT_SUCCESS;
		}

		double tEnd = project.m_tEnd; // override with command line argument
		while (t < tEnd) {
			// ask master to do an internal step
			masterSim.doStep();
			t = masterSim.tCurrent();

			// handle outputs (filtering/scheduling is implemented inside writeOutputs()).
			masterSim.writeOutputs();
		}

	}
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}


