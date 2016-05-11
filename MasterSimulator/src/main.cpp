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

void setupLogFile(const MASTER_SIM::ArgParser & parser);

int main(int argc, char * argv[]) {
	const char * const FUNC_ID = "[main]";

	try {
		// parse command line
		MASTER_SIM::ArgParser parser;
		parser.parse(argc, argv);

		IBK::WaitOnExit wait(!parser.flagEnabled('x'));

		// help and man-page
		if (parser.handleDefaultFlags(std::cout))
			return EXIT_SUCCESS;


		if (parser.flagEnabled('v')) {
			std::cout << "MasterSimulator, version " << MASTER_SIM::LONG_VERSION << std::endl;
			return EXIT_SUCCESS;
		}

		if (!parser.m_projectFile.isValid()) {
			std::cerr << "Missing project file argument. Use --help for syntax." << std::endl;
			return EXIT_FAILURE;
		}

		// setup directory structure and initialize log file, in debug mode set console log level to detailed
#if !defined(NDEBUG)
//		parser.m_verbosityLevel = IBK::VL_DETAILED;
#endif
		setupLogFile(parser);

		// instantiate project
		MASTER_SIM::Project project;

		// read project file
		IBK::IBK_Message(IBK::FormatString("Reading project '%1'\n").arg(parser.m_projectFile), IBK::MSG_PROGRESS, FUNC_ID);
		project.read(IBK::Path(parser.m_projectFile ));

		// create simulator
		MASTER_SIM::MasterSim masterSim;
		// initialize all FMUs (e.g. load dlls/shared libs, parse ModelDescription, do error checking
		masterSim.instantiateFMUs(parser, project);

#if HAVE_SERIALIZATION_CODE
		// set master and all FMUs to start time point
		double tStart = masterSim.tStart(); // override with command line argument
		masterSim.restoreState(tStart, stateDir);
		// (re-)open output file
		masterSim.openOutputFile(true);
#else
		// run master for entire simulation
		masterSim.initialize();
		// (re-)create output file
		masterSim.openOutputFiles(false);
		// and write initial conditions
		masterSim.appendOutputs();
#endif

		if (parser.flagEnabled("test-init")) {
			IBK::IBK_Message("Stopping after successful initialization.\n", IBK::MSG_PROGRESS, FUNC_ID);
			return EXIT_SUCCESS;
		}

		// adjust log-file message handler to log only standard level outputs (unless user specified higher level)
		IBK::MessageHandlerRegistry::instance().messageHandler()->setLogfileVerbosityLevel( std::max<unsigned int>(IBK::VL_STANDARD, parser.m_verbosityLevel));

		// let master run the simulation until end
		masterSim.simulate();

		// print final statistics
		masterSim.writeMetrics();

	}
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		IBK::IBK_Message("Try running with --verbosity-level=4 for more detailed outputs to track down errors.", IBK::MSG_ERROR, FUNC_ID);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}


void setupLogFile(const MASTER_SIM::ArgParser & parser) {
	const char * const FUNC_ID = "[setupLogFile]";
	// setup directory structure
	if (!parser.m_workingDir.exists() && !IBK::Path::makePath(parser.m_workingDir))
		throw IBK::Exception(IBK::FormatString("Error creating working directory: '%1'").arg(parser.m_workingDir), FUNC_ID);

	IBK::Path logPath = parser.m_workingDir / "log";
	if (!logPath.exists() && !IBK::Path::makePath(logPath))
		throw IBK::Exception(IBK::FormatString("Error creating log directory: '%1'").arg(logPath), FUNC_ID);

	// and initialize log file
	IBK::MessageHandler::setupUtf8Console();
	IBK::MessageHandler * msgHandler = IBK::MessageHandlerRegistry::instance().messageHandler();
	msgHandler->setConsoleVerbosityLevel(parser.m_verbosityLevel);
	msgHandler->setLogfileVerbosityLevel( std::max<unsigned int>(IBK::VL_DETAILED, parser.m_verbosityLevel));
	IBK::Path logFile = logPath / "screenlog.txt";
	std::string errmsg;
	if (!msgHandler->openLogFile(logFile.str(), false, errmsg))
		std::cerr << "Cannot write log file '" << logFile.str() << "'." << std::endl;
}
