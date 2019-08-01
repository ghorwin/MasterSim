#include "MSIMSimulationMonitorWidget.h"
#include "ui_MSIMSimulationMonitorWidget.h"

#include "MSIMGUIMessageHandler.h"
#include <IBK_messages.h>
#include <IBK_MessageHandler.h>

#include <MSIM_Project.h>

MSIMSimulationMonitorWidget::MSIMSimulationMonitorWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::MSIMSimulationMonitorWidget)
{
	setWindowModality(Qt::ApplicationModal);
	ui->setupUi(this);
}


MSIMSimulationMonitorWidget::~MSIMSimulationMonitorWidget() {
	delete ui;
}


void MSIMSimulationMonitorWidget::onNewMessageReceived(const QString & message) {

}


void MSIMSimulationMonitorWidget::lastLineOfMessage(const QString & lastLineOfMessage) {

}


void MSIMSimulationMonitorWidget::run(const IBK::Path & projectFile, int verbosityLevel) {
	const char * const FUNC_ID = "[MSIMSimulationMonitorWidget::run]";
#if 0
	try {
		// setup directory structure
		IBK::Path workingDir = projectFile.withoutExtension();

		IBK::Path logPath = workingDir / "log";
		if (!logPath.exists() && !IBK::Path::makePath(logPath))
			throw IBK::Exception(IBK::FormatString("Error creating log directory: '%1'").arg(logPath), FUNC_ID);

		// and initialize log file
		IBK::MessageHandler * msgHandler = IBK::MessageHandlerRegistry::instance().messageHandler();
		msgHandler->setConsoleVerbosityLevel(parser.m_verbosityLevel);
		msgHandler->setLogfileVerbosityLevel( std::max<unsigned int>(IBK::VL_DETAILED, parser.m_verbosityLevel));
		IBK::Path logFile = logPath / "screenlog.txt";
		std::string errmsg;
		if (!msgHandler->openLogFile(logFile.str(), false, errmsg))
			std::cerr << "Cannot write log file '" << logFile.str() << "'." << std::endl;

		MASTER_SIM::MasterSim::writeVersionInfo();

		// instantiate project
		MASTER_SIM::Project project;

		// read project file
		if (!parser.m_projectFile.exists())
			throw IBK::Exception(IBK::FormatString("Project file '%1' doesn't exist.").arg(parser.m_projectFile), FUNC_ID);
		IBK::IBK_Message(IBK::FormatString("Reading project '%1'\n").arg(parser.m_projectFile), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		project.read(IBK::Path(parser.m_projectFile), false);

		// create simulator
		MASTER_SIM::MasterSim masterSim;
		// initialize all FMUs (e.g. load dlls/shared libs, parse ModelDescription, do error checking
		masterSim.importFMUs(parser, project);

#ifdef HAVE_SERIALIZATION_CODE
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
			IBK::IBK_Message("Stopping after successful initialization.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			// free FMU slaves
			masterSim.freeSlaves();
			return EXIT_SUCCESS;
		}

		// adjust log-file message handler to log only standard level outputs (unless user specified higher level)
		IBK::MessageHandlerRegistry::instance().messageHandler()->setConsoleVerbosityLevel( std::max<int>(IBK::VL_STANDARD, (int)parser.m_verbosityLevel));
		IBK::MessageHandlerRegistry::instance().messageHandler()->setLogfileVerbosityLevel( std::max<int>(IBK::VL_STANDARD, (int)parser.m_verbosityLevel));

		// let master run the simulation until end
		masterSim.simulate();

		// print final statistics
		masterSim.writeMetrics();

		// free FMU slaves
		masterSim.freeSlaves();

	}
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		IBK::IBK_Message("Try running with --verbosity-level=4 for more detailed outputs to track down errors.", IBK::MSG_ERROR, FUNC_ID);
		return EXIT_FAILURE;

	}

#endif
}
