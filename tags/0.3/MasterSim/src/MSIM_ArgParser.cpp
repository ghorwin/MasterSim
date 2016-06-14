#include "MSIM_ArgParser.h"

#include <IBK_StringUtils.h>

namespace MASTER_SIM {

ArgParser::ArgParser() : m_verbosityLevel(1)
{
	m_appname = "MasterSimulator";
	addOption('v', "version", "Show version info.", "<true|false>", "false");
	addOption('x', "close-on-exit", "Close console window after finishing simulation.", "<true|false>", "false");
	addOption('t', "test-init", "Run the initialization and stop right afterwards.", "<true|false>", "false");
	addOption(0, "skip-unzip", "Do not unzip FMUs and expect them to be unzipped in extraction directories.", "<true|false>", "false");
	addOption(0, "verbosity-level", "Level of output detail (0-3).", "0..3", "1");
	addOption(0, "working-dir", "Working directory for master.", "working-directory", "Parent path of project file.");
}

void ArgParser::parse(int argc, const char * const argv[]) {
//	const char * const FUNC_ID = "[SolverArgsParser::parse]";

	// parse parent
	IBK::ArgParser::parse(argc, argv);


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
