#include "MSIM_Project.h"

#include <iostream>
#include <fstream>
#include <cstdlib> // for std::rand()

#include <IBK_StringUtils.h>
#include <IBK_FileUtils.h>
#include <IBK_assert.h>

#include "MSIM_Constants.h"

namespace MASTER_SIM {

Project::Project() :
	m_tStart("tStart", 0, "a"),
	m_tEnd("tEnd", 1, "a"),
	m_hFallBackLimit("hFallBackLimit", 1e-3, "s"),
	m_hMin("hMin", 1e-5, "s"),
	m_hMax("hMax", 30, "min"),
	m_hStart("hStart", 0.01, "s"),
	m_masterMode(MM_GAUSS_JACOBI),
	m_errorControlMode(EM_NONE),
	m_adjustStepSize(true),
	m_maxIterations(10),
	m_absTol(1e-6),
	m_relTol(1e-5),
	m_hOutputMin("hOutputMin", 120, "s"),
	m_binaryOutputFiles(false),
	m_outputTimeUnit("s")
{
}


const unsigned int COLOR_COUNT = 10;
const char * const SIMULATOR_COLORS[COLOR_COUNT] = {
	"#FA8072",
	"#0000CD",
	"#9400D3",
	"#008000",
	"#20B2AA",
	"#6A5ACD",
	"#FF8C00",
	"#A0522D",
	"#4682B4",
	"#FFD700"
};


void Project::read(const IBK::Path & prjFile, bool /* headerOnly */) {
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
				sim.m_color = IBK::Color::fromHtml(SIMULATOR_COLORS[m_simulators.size() % COLOR_COUNT]);
				sim.parse(line);
				m_simulators.push_back(sim);
				continue;
			}

			if (line.find("graph") == 0) {
				std::string graphEdges = line.substr(5);
				IBK::trim(graphEdges);
				std::vector<std::string> tokens;
				if (IBK::explode(graphEdges, tokens, " \t", IBK::EF_TrimTokens) != 2)
					throw IBK::Exception(IBK::FormatString("Expected format 'graph <connectorStart> <connectorEnd>', got '%1'.").arg(line), FUNC_ID);
				GraphEdge g;
				g.m_outputVariableRef = IBK::trim_copy(tokens[0]);
				g.m_inputVariableRef = IBK::trim_copy(tokens[1]);
				m_graph.push_back(g);
				continue;
			}

			if (line.find("parameter") == 0) {
				std::string paraString = line.substr(9);
				IBK::trim(paraString);
				// general parameters
				if (IBK::explode(paraString, tokens, " \t", IBK::EF_TrimTokens) != 2)
					throw IBK::Exception(IBK::FormatString("Expected format 'parameter <flat name> <value>', got '%1'").arg(line), FUNC_ID);
				std::string value = tokens[1];
				// extract slave name
				std::string parameter = tokens[0];
				if (IBK::explode(parameter, tokens, ".", IBK::EF_TrimTokens) != 2)
					throw IBK::Exception(IBK::FormatString("Expected parameter name in format <slave name>.<parameter name>', got '%1'").arg(line), FUNC_ID);
				std::string slaveName = tokens[0];
				parameter = tokens[1];
				// add paramter to slave
				unsigned int s=0;
				for (; s<m_simulators.size(); ++s) {
					if (m_simulators[s].m_name == slaveName) {
						m_simulators[s].m_parameters[parameter] = value;
						break;
					}
				}
				if (s == m_simulators.size())
					throw IBK::Exception(IBK::FormatString("Unknown slave referenced in parameter in line '%1'").arg(line), FUNC_ID);
				continue;
			}

			// general parameters
			if (IBK::explode_in2(line, tokens, " \t") != 2)
				throw IBK::Exception(IBK::FormatString("Expected format '<keyword> <value>'.").arg(line), FUNC_ID);

			std::string keyword = IBK::trim_copy(tokens[0]);
			std::string value = IBK::trim_copy(tokens[1]);

			if (keyword == "tStart") {
				if (!m_tStart.set(keyword, value)) throw IBK::Exception( IBK::FormatString("Invalid format of parameter in line '%1'.").arg(line), FUNC_ID);
			} else if (keyword == "tEnd") {
				if (!m_tEnd.set(keyword, value)) throw IBK::Exception( IBK::FormatString("Invalid format of parameter in line '%1'.").arg(line), FUNC_ID);
			} else if (keyword == "hMax") {
				if (!m_hMax.set(keyword, value)) throw IBK::Exception( IBK::FormatString("Invalid format of parameter in line '%1'.").arg(line), FUNC_ID);
			} else if (keyword == "hMin") {
				if (!m_hMin.set(keyword, value)) throw IBK::Exception( IBK::FormatString("Invalid format of parameter in line '%1'.").arg(line), FUNC_ID);
			} else if (keyword == "hFallBackLimit") {
				if (!m_hFallBackLimit.set(keyword, value)) throw IBK::Exception( IBK::FormatString("Invalid format of parameter in line '%1'.").arg(line), FUNC_ID);
			} else if (keyword == "hStart") {
				if (!m_hStart.set(keyword, value)) throw IBK::Exception( IBK::FormatString("Invalid format of parameter in line '%1'.").arg(line), FUNC_ID);
			} else if (keyword == "hOutputMin") {
				if (!m_hOutputMin.set(keyword, value)) throw IBK::Exception( IBK::FormatString("Invalid format of parameter in line '%1'.").arg(line), FUNC_ID);
			} else if (keyword == "binaryOutputFiles")
				m_binaryOutputFiles = (value == "true" || value == "yes" || value == "1");
			else if (keyword == "adjustStepSize")
				m_adjustStepSize = (value == "true" || value == "yes" || value == "1");
			else if (keyword == "absTol")
				m_absTol = IBK::string2val<double>(value);
			else if (keyword == "relTol")
				m_relTol = IBK::string2val<double>(value);
			else if (keyword == "MasterMode") {
				if (value == "GAUSS_JACOBI")
					m_masterMode = MM_GAUSS_JACOBI;
				else if (value == "GAUSS_SEIDEL")
					m_masterMode = MM_GAUSS_SEIDEL;
				else if (value == "NEWTON")
					m_masterMode = MM_NEWTON;
				else
					throw IBK::Exception(IBK::FormatString("Unknown/undefined master mode '%1'.").arg(value), FUNC_ID);
			}
			else if (keyword == "ErrorControlMode") {
				if (value == "NONE")
					m_errorControlMode = EM_NONE;
				else if (value == "CHECK")
					m_errorControlMode = EM_CHECK;
				else if (value == "ADAPT_STEP")
					m_errorControlMode = EM_ADAPT_STEP;
				else
					throw IBK::Exception(IBK::FormatString("Unknown/undefined master mode '%1'.").arg(value), FUNC_ID);
			}
			else if (keyword == "maxIterations")
				m_maxIterations = IBK::string2val<unsigned int>(value);
			else
				throw IBK::Exception(IBK::FormatString("Unknown keyword '%1'").arg(keyword), FUNC_ID);
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error in line #%1: '%2'.").arg(lineNr).arg(line), FUNC_ID);
		}
	}
}


void Project::write(const IBK::Path & prjFile) const {
	const char * const FUNC_ID = "[Project::write]";

	std::ofstream out;
#if defined(_WIN32) && !defined(__MINGW32__)
	out.open(prjFile.wstr().c_str());
#else
	out.open(prjFile.str().c_str());
#endif // _WIN32

	if (!out)
		throw IBK::Exception( IBK::FormatString("Cannot open file '%1'").arg(prjFile), FUNC_ID);


	if (!m_tStart.empty())		m_tStart.write(out, KEYWORD_INDENTATION, KEYWORD_WIDTH, true);
	if (!m_tEnd.empty())		m_tEnd.write(out, KEYWORD_INDENTATION, KEYWORD_WIDTH, true);
	if (!m_hMax.empty())		m_hMax.write(out, KEYWORD_INDENTATION, KEYWORD_WIDTH, true);
	if (!m_hMin.empty())		m_hMin.write(out, KEYWORD_INDENTATION, KEYWORD_WIDTH, true);
	if (!m_hFallBackLimit.empty())		m_hFallBackLimit.write(out, KEYWORD_INDENTATION, KEYWORD_WIDTH, true);
	if (!m_hStart.empty())		m_hStart.write(out, KEYWORD_INDENTATION, KEYWORD_WIDTH, true);
	if (!m_hOutputMin.empty())		m_hOutputMin.write(out, KEYWORD_INDENTATION, KEYWORD_WIDTH, true);
	out << std::setw(KEYWORD_WIDTH) << std::left << "binaryOutputFiles" << " " << (m_binaryOutputFiles ? "yes" : "no") << std::endl;
	out << std::setw(KEYWORD_WIDTH) << std::left << "adjustStepSize" << " " << (m_adjustStepSize ? "yes" : "no") << std::endl;
	out << std::setw(KEYWORD_WIDTH) << std::left << "absTol" << " " << m_absTol << std::endl;
	out << std::setw(KEYWORD_WIDTH) << std::left << "relTol" << " " << m_relTol << std::endl;
	out << std::setw(KEYWORD_WIDTH) << std::left << "MasterMode" << " ";
	switch (m_masterMode) {
		case MM_GAUSS_JACOBI : out << "GAUSS_JACOBI"; break;
		case MM_GAUSS_SEIDEL : out << "GAUSS_SEIDEL"; break;
		case MM_NEWTON : out << "NEWTON"; break;
	}
	out << std::endl;
	out << std::setw(KEYWORD_WIDTH) << std::left << "ErrorControlMode" << " ";
	switch (m_errorControlMode) {
		case EM_NONE : out << "NONE"; break;
		case EM_CHECK : out << "CHECK"; break;
		case EM_ADAPT_STEP : out << "ADAPT_STEP"; break;
	}
	out << std::endl;
	out << std::setw(KEYWORD_WIDTH) << std::left << "maxIterations" << " " << m_maxIterations << std::endl;
	out << std::endl;

	// write simulators
	for (unsigned int i=0; i<m_simulators.size(); ++i) {
		const SimulatorDef & simDef = m_simulators[i];
		out << "simulator " << i << " " << simDef.m_cycle << " " << simDef.m_name << " " << simDef.m_color.toHtmlString() << " \"" << simDef.m_pathToFMU.str() << "\"" << std::endl;
	}
	out << std::endl;

	// write graph
	for (unsigned int i=0; i<m_graph.size(); ++i) {
		const Project::GraphEdge & edge = m_graph[i];
		out << "graph " << edge.m_outputVariableRef << " " << edge.m_inputVariableRef << std::endl;
	}

	// write parameters
	for (unsigned int i=0; i<m_simulators.size(); ++i) {
		const SimulatorDef & simDef = m_simulators[i];
		for (std::map<std::string, std::string>::const_iterator it = simDef.m_parameters.begin(); it != simDef.m_parameters.end(); ++it)
			out << "parameter " << simDef.m_name << "." << it->first << "   " << it->second << std::endl;
	}
	out << std::endl;
}


const Project::SimulatorDef & Project::simulatorDefinition(const std::string & slaveName) const {
	for (unsigned int s=0; s<m_simulators.size(); ++s) {
		if (m_simulators[s].m_name == slaveName)
			return m_simulators[s];
	}
	throw IBK::Exception(IBK::FormatString("Cannot find simulator/slave definition with name '%1'.").arg(slaveName), "[Project::simulatorDefinition]");
}


Project::SimulatorDef::SimulatorDef() :
	m_cycle(0)
{
	m_color = IBK::Color::fromHtml(SIMULATOR_COLORS[rand() % COLOR_COUNT]);
}


void Project::SimulatorDef::parse(const std::string & simulatorDef) {
	const char * const FUNC_ID = "[Project::SimulatorDef::parse]";
	// simulator   0 0 Part1 #00bdcf "fmus/simx/Part1.fmu"
	std::size_t pos = simulatorDef.find("simulator");
	IBK_ASSERT(pos != std::string::npos);
	std::vector<std::string> tokens;
	// first extract
	try {
		IBK::explode(simulatorDef.substr(pos+9), tokens, " \t", IBK::EF_UseQuotes | IBK::EF_TrimTokens);
		if (tokens.size() != 5)
			throw IBK::Exception("Missing properties.", FUNC_ID);
		m_cycle = IBK::string2val<unsigned int>(tokens[1]);
		m_name = IBK::trim_copy(tokens[2]);
		m_color = IBK::Color::fromHtml(tokens[3]);
		m_pathToFMU = IBK::Path( IBK::trim_copy(tokens[4], "\"") );
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Bad format of simulator definition line '%1'.").arg(simulatorDef), FUNC_ID);
	}
}


std::string Project::GraphEdge::extractSlaveName( const std::string & variableRef) {
	std::vector<std::string> tokens;
	unsigned int count = IBK::explode_in2(variableRef, tokens, '.');
	if (count < 2)
		throw IBK::Exception(IBK::FormatString("Invalid syntax in graph variable reference '%1'.").arg(variableRef), "[Project::GraphEdge::extractSlaveName]");
	return tokens[0];
}


std::string Project::GraphEdge::replaceSlaveName( const std::string & variableRef, const std::string & newSlaveName) {
	std::vector<std::string> tokens;
	unsigned int count = IBK::explode_in2(variableRef, tokens, '.');
	if (count < 2)
		throw IBK::Exception(IBK::FormatString("Invalid syntax in graph variable reference '%1'.").arg(variableRef), "[Project::GraphEdge::replaceSlaveName]");
	return newSlaveName + "." + tokens[1];
}

} // namespace MASTER_SIM
