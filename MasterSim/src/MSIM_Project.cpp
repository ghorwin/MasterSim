#include "MSIM_Project.h"

#include <iostream>
#include <fstream>

#include <IBK_StringUtils.h>
#include <IBK_assert.h>

namespace MASTER_SIM {

Project::Project()
{
}


void Project::read(const IBK::Path & prjFile) {
	const char * const FUNC_ID = "[Project::read]";

	std::ifstream in;
#if defined(_WIN32) && !defined(__MINGW32__)
	in.open(prjFile.wstr().c_str());
#else
	in.open(prjFile.str().c_str());
#endif // _WIN32

	if (!in)
		throw IBK::Exception( IBK::FormatString("Cannot open file '%1'").arg(prjFile), FUNC_ID);

	// read file line-by-line
	std::string line;
	int lineNr = 0;
	while (std::getline(in, line)) {
		++lineNr;
		IBK::trim(line);
		// skip empty lines or comments
		if (line.find_first_not_of(" \t\r") == std::string::npos || line[0] == '#')
			continue;

		try {
			std::vector<std::string> tokens;

			// switch to different section
			if (line.find("simulator") == 0) {
				SimulatorDef sim;
				sim.parse(line);
				m_simulators.push_back(sim);
				continue;
			}

			if (line.find("graph") == 0) {
				std::string graphEdges = line.substr(5);
				std::vector<std::string> tokens;
				if (IBK::explode_in2(graphEdges, tokens) != 2)
					throw IBK::Exception(IBK::FormatString("Expected format 'graph <connectorStart> <connectorEnd>'.").arg(line), FUNC_ID);
				GraphEdge g;
				g.m_outputVariableRef = IBK::trim_copy(tokens[0]);
				g.m_inputVariableRef = IBK::trim_copy(tokens[1]);
				m_graph.push_back(g);
				continue;
			}

			// general parameters
			if (IBK::explode_in2(line, tokens) != 2)
				throw IBK::Exception(IBK::FormatString("Expected format '<keyword> <value>'.").arg(line), FUNC_ID);

			std::string keyword = IBK::trim_copy(tokens[0]);
			std::string value = tokens[1];

			if (keyword == "tstart")
				m_tStart = IBK::string2val<double>(value);
			else if (keyword == "tend")
				m_tEnd = IBK::string2val<double>(value);
			else if (keyword == "tstepmax")
				m_tStepMax = IBK::string2val<double>(value);
			else if (keyword == "tstepstart")
				m_tStepStart = IBK::string2val<double>(value);
			else if (keyword == "it_tol_abs")
				m_absTol = IBK::string2val<double>(value);
			else if (keyword == "it_tol_rel")
				m_relTol = IBK::string2val<double>(value);
			else if (keyword == "MasterMode")
				m_masterMode = (MasterMode)IBK::string2val<unsigned int>(value);
			else if (keyword == "it_max_steps")
				m_maxSteps = IBK::string2val<unsigned int>(value);
			else
				throw IBK::Exception(IBK::FormatString("Unknown keyword '%1'").arg(keyword), FUNC_ID);
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error in line #%1: '%2'.").arg(lineNr).arg(line), FUNC_ID);
		}
	}

	//	m_tOutputStepMin = 3600; // should be in project file
}


void Project::SimulatorDef::parse(const std::string & simulatorDef) {
	const char * const FUNC_ID = "[Project::SimulatorDef::parse]";
	// simulator   0 0 Part1 fmus/simx/Part1.fmu
	std::size_t pos = simulatorDef.find("simulator");
	IBK_ASSERT(pos != std::string::npos);
	std::vector<std::string> tokens;
	IBK::explode(simulatorDef.substr(pos+9), tokens, ' ');
	if (tokens.size() != 4)
		throw IBK::Exception( IBK::FormatString("Bad format of simulator definition line '%1'.").arg(simulatorDef), FUNC_ID);
	try {
		// convert first token to ID
		m_id = IBK::string2val<unsigned int>(tokens[0]);
		m_priority = IBK::string2val<unsigned int>(tokens[1]);
		m_name = IBK::trim_copy(tokens[2]);
		m_pathToFMU = IBK::Path(tokens[3]);
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Bad format of simulator definition line '%1'.").arg(simulatorDef), FUNC_ID);
	}
}


} // namespace MASTER_SIM
