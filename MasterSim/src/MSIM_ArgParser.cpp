#include "MSIM_ArgParser.h"

#include <iostream>
#include <IBK_StringUtils.h>

#include "MSIM_Constants.h"

namespace MASTER_SIM {

ArgParser::ArgParser() : m_verbosityLevel(1) {
	m_appname = "MasterSimulator";
	m_syntaxArguments = "[flags] [options] <project file>";
	// configure man page output
	m_manManualName = "MasterSim Manual";
	m_manReleaseDate = MASTER_SIM::RELEASE_DATE;
	m_manVersionString = MASTER_SIM::LONG_VERSION;
	m_manShortDescription = "FMI Co-Simulation Master";

	// Note: mind the line breaks that end format commands!
	m_manLongDescription = ".B MasterSimulator\n"
			"simulates the co-simulation scenario defined in the msim "
			"project file. By default all temporary files and all output is created in a subdirectory "
			"with the same name as the project. You can change that with the\n"
			".BR --working-dir\n"
			"option.";

	addOption('v', "version", "Show version info.", "<true|false>", "false");
	addOption('x', "close-on-exit", "Close console window after finishing simulation.", "<true|false>", "false");
	addOption('t', "test-init", "Run the initialization and stop right afterwards.", "<true|false>", "false");
	addOption(0, "skip-unzip", "Do not unzip FMUs and expect them to be unzipped in extraction directories.", "<true|false>", "false");
	addOption(0, "verbosity-level", "Level of output detail (0-3).", "0..3", "1");
	addOption(0, "working-dir", "Working directory for master, where FMUs are extracted to and simulation results/log files are written.", "working-directory", "Project file path without extension.");
}

void ArgParser::parse(int argc, const char * const argv[]) {
//	const char * const FUNC_ID = "[SolverArgsParser::parse]";

	// parse parent
	IBK::ArgParser::parse(argc, argv);

//	for (const std::string & a : args())
//		std::cout << a << std::endl;

	// now extract arguments
	if (args().size() > 0) {
#if defined(_WIN32)
		std::string p = args()[0];
		std::wstring wp = IBK::ANSIToWstring(p, false);
		m_executablePath = IBK::WstringToUTF8(wp);
#else
		m_executablePath = args()[0];
#endif
	}

	if (args().size() > 1) {
#if defined(_WIN32)
		std::string p = args()[1];
		std::wstring wp = IBK::ANSIToWstring(p, false);
		m_projectFile = IBK::WstringToUTF8(wp);
#else
		m_projectFile = args()[1];
#endif
		// remove "file://" prefix
		if (m_projectFile.str().find("file://") == 0)
			m_projectFile = IBK::Path(m_projectFile.str().substr(7));
		try {
			// get working directory (defaults to project file path)
			m_workingDir = m_projectFile.withoutExtension();
		}
		catch (...) {
			m_workingDir.clear();
		}
	}

	// if invalid working dir (project file path is relative) use current directory
	if (!m_workingDir.isValid())
		m_workingDir = IBK::Path::current();
	// override with command line option
	if (hasOption("working-dir")) {
#if defined(_WIN32)
		std::string p = option("working-dir");
		std::wstring wp = IBK::ANSIToWstring(p, false);
		m_workingDir = IBK::WstringToUTF8(wp);
#else
		m_workingDir = IBK::Path(option("working-dir"));
#endif
	}

	try {
		m_verbosityLevel = IBK::string2val<unsigned int>(option("verbosity-level"));
	}
	catch (...) {
		m_verbosityLevel = 1;
	}
}

void ArgParser::printHelp(std::ostream & out) const {
	out << "Syntax: " << m_appname << " [flags] [options] <project file>\n\n";
	const unsigned int TEXT_WIDTH = 79;
	const unsigned int TAB_WIDTH = 20;
	printFlags(out,TEXT_WIDTH,TAB_WIDTH);
	printOptions(out,TEXT_WIDTH,TAB_WIDTH);
}


} // namespace MASTER_SIM
