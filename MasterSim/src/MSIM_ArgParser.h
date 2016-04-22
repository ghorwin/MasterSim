#ifndef MSIM_ARGPARSER_H
#define MSIM_ARGPARSER_H

#include <IBK_ArgParser.h>
#include <IBK_Path.h>

namespace MASTER_SIM {

/*! Specialized arg parser for master simulators. */
class ArgParser : public IBK::ArgParser {
public:
	ArgParser();

	/*! Parses arguments and converts options to utf8 strings. */
	void parse(int argc, const char * const argv[]);

	/*! The executable path or installation directory. */
	IBK::Path		m_executablePath;

	/*! The full path to the project file (absolute file path or relative to working directory). */
	IBK::Path		m_projectFile;

	/*! Full path to working directory. */
	IBK::Path		m_workingDir;
};

} // namespace MASTER_SIM


#endif // MSIM_ARGPARSER_H
