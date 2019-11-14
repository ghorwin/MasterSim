#include "MSIM_Project.h"

#include <iostream>
#include <fstream>
#include <cstdlib> // for std::rand()

#include <IBK_StringUtils.h>
#include <IBK_FileUtils.h>
#include <IBK_assert.h>
#include <IBK_UnitList.h>

#include "MSIM_Constants.h"

namespace MASTER_SIM {

Project::Project() :
	m_tStart("tStart", 0, "a"),
	m_tEnd("tEnd", 1, "a"),
	m_hFallBackLimit("hFallBackLimit", 1e-3, "s"),
	m_hMin("hMin", 1e-5, "s"),
	m_hMax("hMax", 30, "min"),
	m_hStart("hStart", 10, "min"),
	m_masterMode(MM_GAUSS_SEIDEL),
	m_errorControlMode(EM_NONE),
	m_adjustStepSize(false),
	m_preventOversteppingOfEndTime(true),
	m_maxIterations(1),
	m_absTol(1e-6),
	m_relTol(1e-5),
	m_hOutputMin("hOutputMin", 10, "min"),
	m_outputTimeUnit("s"),
	m_writeInternalVariables(false)
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
	bool leadingComment = true;
	while (std::getline(in, line)) {
		++lineNr;
		IBK::trim(line);
		// skip empty lines or comments
		if (line.find_first_not_of(" \t\r") == std::string::npos)
			continue;

		// check for comment
		if (line[0] == '#') {
			if (leadingComment) {
				// special treatment of Created and LastModified lines
				std::string::size_type pos;
				if ((pos = line.find("Created")) != std::string::npos) {
					m_created = line.substr(pos  + 8);
					IBK::trim(m_created);
				}
				else if ((pos = line.find("LastModified")) != std::string::npos) {
					m_lastEdited = line.substr(pos  + 13);
					IBK::trim(m_lastEdited);
				}
				else
					m_comment += line + "\n";
			}
			continue;
		}

		leadingComment = false;

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
				IBK::explode(graphEdges, tokens, " \t", IBK::EF_TrimTokens);
				if (tokens.size() != 2 && tokens.size() != 4)
					throw IBK::Exception(IBK::FormatString("Expected format 'graph <connectorStart> <connectorEnd> [<offset> <scaleFactor>]', got '%1'.").arg(line), FUNC_ID);
				GraphEdge g;
				g.m_outputVariableRef = tokens[0];
				g.m_inputVariableRef = tokens[1];
				if (tokens.size() == 4) {
					try {
						g.m_offset = IBK::string2val<double>(tokens[2]);
						g.m_scaleFactor = IBK::string2val<double>(tokens[3]);
					} catch (...) {
						throw IBK::Exception(IBK::FormatString("Invalid offset or scale factor in graph '%1'.").arg(line), FUNC_ID);
					}
				}
				m_graph.push_back(g);
				continue;
			}

			if (line.find("parameter") == 0) {
				std::string paraString = line.substr(9);
				IBK::trim(paraString);
				// extract variable name
				std::string::size_type spacePos = paraString.find_first_of(" \t");
				if (spacePos == std::string::npos)
					throw IBK::Exception(IBK::FormatString("Expected format 'parameter <flat name> <value>', got '%1'").arg(line), FUNC_ID);
				std::string value = paraString.substr(spacePos);
				std::string parameter = paraString.substr(0, spacePos);
				IBK::trim(value);
				// special handling for string parameters, replace \\ with \ character
				value = IBK::replace_string(value, "\\\\", "\\");
				value = IBK::replace_string(value, "\\n", "\n");
				IBK::trim(paraString);
				// extract slave name
				std::size_t dotPos = parameter.find('.');
				if (dotPos == std::string::npos)
					throw IBK::Exception(IBK::FormatString("Expected parameter variable name in format <slave name>.<parameter name>', got '%1'").arg(line), FUNC_ID);
				std::string slaveName = parameter.substr(0, dotPos);
				parameter = parameter.substr(dotPos+1);
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

			/// \todo Implement support for output filter
			if (line.find("outputOnly") == 0) {
				std::string outputVarString = line.substr(6);
				IBK::trim(outputVarString);
				if (outputVarString.empty())
					throw IBK::Exception(IBK::FormatString("Expected format 'outputOnly <flat variable name>', got '%1'").arg(line), FUNC_ID);
				m_outputFilter.insert(outputVarString);
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
			} else if (keyword == "outputTimeUnit") {
				try {
					m_outputTimeUnit = IBK::Unit(value);
					double t=2;
					IBK::UnitList::instance().convert(m_outputTimeUnit, IBK::Unit("s"),t); // test convert to seconds
				} catch (...) {
					throw IBK::Exception( IBK::FormatString("Invalid outputTimeUnit '%1' or cannot be converted to 's' .").arg(value), FUNC_ID);
				}
			}
			else if (keyword == "adjustStepSize")
				m_adjustStepSize = (value == "true" || value == "yes" || value == "1");
			else if (keyword == "preventOversteppingOfEndTime")
				m_preventOversteppingOfEndTime = (value == "true" || value == "yes" || value == "1");
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
				else if (value == "ADAPT_STEP" || value == "STEP_DOUBLING")
					m_errorControlMode = EM_STEP_DOUBLING;
				else
					throw IBK::Exception(IBK::FormatString("Unknown/undefined master mode '%1'.").arg(value), FUNC_ID);
			}
			else if (keyword == "maxIterations")
				m_maxIterations = IBK::string2val<unsigned int>(value);
			else if (keyword == "writeInternalVariables")
				m_writeInternalVariables = (value == "true" || value == "yes" || value == "1");
			else
				throw IBK::Exception(IBK::FormatString("Unknown keyword '%1'").arg(keyword), FUNC_ID);
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error in line #%1: '%2'.").arg(lineNr).arg(line), FUNC_ID);
		}
	}
}


void writeParameter(const IBK::Parameter & p, std::ostream& out, unsigned int indent, unsigned int paramWidth) {
	double v = p.value;
	IBK::UnitList::instance().convert(IBK::Unit(p.IO_unit.base_id()), p.IO_unit, v);
	out << std::string(indent, ' ');
	if (p.name.size()) {
		if (paramWidth > p.name.size())
			out << std::setw(paramWidth) << std::left << p.name << " ";
		else
			out << p.name << " ";
	}
	out << v << " " << p.IO_unit.name() << std::endl;
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

	// write created and last-modified strings
	out << "# Created:\t" << m_created << std::endl;
	out << "# LastModified:\t" << m_lastEdited << std::endl << std::endl;
	// write leading comment, must hold '\n' separated lines with each line beginning with '#'
	out << m_comment;
	if (!m_comment.empty())
		out << std::endl;

	if (!m_tStart.empty())		writeParameter(m_tStart, out, KEYWORD_INDENTATION, KEYWORD_WIDTH);
	if (!m_tEnd.empty())		writeParameter(m_tEnd, out, KEYWORD_INDENTATION, KEYWORD_WIDTH);
	if (!m_hMax.empty())		writeParameter(m_hMax, out, KEYWORD_INDENTATION, KEYWORD_WIDTH);
	if (!m_hMin.empty())		writeParameter(m_hMin, out, KEYWORD_INDENTATION, KEYWORD_WIDTH);
	if (!m_hFallBackLimit.empty())		writeParameter(m_hFallBackLimit, out, KEYWORD_INDENTATION, KEYWORD_WIDTH);
	if (!m_hStart.empty())		writeParameter(m_hStart, out, KEYWORD_INDENTATION, KEYWORD_WIDTH);
	if (!m_hOutputMin.empty())		writeParameter(m_hOutputMin, out, KEYWORD_INDENTATION, KEYWORD_WIDTH);
	out << std::setw(KEYWORD_WIDTH) << std::left << "outputTimeUnit" << " " << m_outputTimeUnit << std::endl;
	out << std::setw(KEYWORD_WIDTH) << std::left << "adjustStepSize" << " " << (m_adjustStepSize ? "yes" : "no") << std::endl;
	out << std::setw(KEYWORD_WIDTH) << std::left << "preventOversteppingOfEndTime" << " " << (m_preventOversteppingOfEndTime ? "yes" : "no") << std::endl;
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
		case EM_STEP_DOUBLING : out << "STEP_DOUBLING"; break;
	}
	out << std::endl;
	out << std::setw(KEYWORD_WIDTH) << std::left << "maxIterations" << " " << m_maxIterations << std::endl;
	out << std::setw(KEYWORD_WIDTH) << std::left << "writeInternalVariables" << " " << (m_writeInternalVariables ? "yes" : "no") << std::endl;
	out << std::endl;

	// write simulators
	for (unsigned int i=0; i<m_simulators.size(); ++i) {
		const SimulatorDef & simDef = m_simulators[i];
		unsigned int res = simDef.m_color.toQRgb();
		res &= 0x00FFFFFF;
		std::stringstream cstr;
		cstr << std::hex << std::setw(6) << std::setfill('0') << res;
		out << "simulator " << i << " " << simDef.m_cycle << " " << simDef.m_name << " #" << cstr.str() << " \"" << simDef.m_pathToFMU.str() << "\"" << std::endl;
	}
	out << std::endl;

	// write graph
	for (unsigned int i=0; i<m_graph.size(); ++i) {
		const Project::GraphEdge & edge = m_graph[i];
		out << "graph " << edge.m_outputVariableRef << " " << edge.m_inputVariableRef;
		if (edge.m_offset != 0.0 || edge.m_scaleFactor != 1.0) {
			out << " " << edge.m_offset << " " << edge.m_scaleFactor;
		}
		out << std::endl;
	}

	// write parameters
	for (unsigned int i=0; i<m_simulators.size(); ++i) {
		const SimulatorDef & simDef = m_simulators[i];
		for (std::map<std::string, std::string>::const_iterator it = simDef.m_parameters.begin(); it != simDef.m_parameters.end(); ++it) {
			// do not write empty parameters
			if (it->second.empty())
				continue;
			out << "parameter " << simDef.m_name << "." << it->first << "   " << it->second << std::endl;
		}
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


void Project::checkGraphs(const std::map<IBK::Path, MASTER_SIM::ModelDescription> & modelDescriptions,
						  std::vector<GraphCheckErrorCodes> & validEdges) const
{
	validEdges.resize(m_graph.size());

	// process all connectors
	std::set<std::string> connectedInletSockets; // stores all connected inlet sockets; graphs connecting to an already connected inlet socket are treated as invalid
	for (unsigned int i=0; i<m_graph.size(); ++i) {
		const GraphEdge & graphEdge = m_graph[i];
		// target socket already taken? this would be invalid
		if (connectedInletSockets.find(graphEdge.m_outputVariableRef) != connectedInletSockets.end()) {
			validEdges[i] = GEC_TargetSocketAlreadyConnected;
			continue; // target socket
		}
		try {
			// extract slave and variable name from graph
			std::string sourceSlaveName;
			std::string variableName;
			GraphEdge::splitReference(graphEdge.m_outputVariableRef, sourceSlaveName, variableName);
			// check if the referenced slave exists
			const SimulatorDef & sourceSim = simulatorDefinition(sourceSlaveName);
			// check if referenced variable exists
			std::map<IBK::Path, MASTER_SIM::ModelDescription>::const_iterator it = modelDescriptions.find(sourceSim.m_pathToFMU);
			// not found or maybe not yet read? mark connection as "undetermined"
			if (it == modelDescriptions.end()) {
				validEdges[i] = GEC_Undetermined;
				continue;
			}
			// check if socket exists
			const ModelDescription & modelDesc = it->second;
			const FMIVariable & var = modelDesc.variable(variableName);
			// check for correct type
			if (var.m_causality != FMIVariable::C_OUTPUT) {
				validEdges[i] = GEC_SourceSocketNotOutlet;
				continue;
			}
			// all ok so far
		} catch (...) {
			validEdges[i] = GEC_InvalidSourceSocketName;
		}
		// now the same for the target
		try {
			// extract slave and variable name from graph
			std::string slaveName;
			std::string variableName;
			GraphEdge::splitReference(graphEdge.m_inputVariableRef, slaveName, variableName);
			// check if the referenced slave exists
			const SimulatorDef & sim = simulatorDefinition(slaveName);
			// check if referenced variable exists
			std::map<IBK::Path, MASTER_SIM::ModelDescription>::const_iterator it = modelDescriptions.find(sim.m_pathToFMU);
			// not found or maybe not yet read? mark connection as "undetermined"
			if (it == modelDescriptions.end()) {
				validEdges[i] = GEC_Undetermined;
				continue;
			}
			// check if socket exists
			const ModelDescription & modelDesc = it->second;
			const FMIVariable & var = modelDesc.variable(variableName);
			// check for correct type
			if (var.m_causality != FMIVariable::C_INPUT) {
				validEdges[i] = GEC_TargetSocketNotInlet;
				continue;
			}
			// all ok so far, remember the target slot to be taken already
			connectedInletSockets.insert(graphEdge.m_outputVariableRef);
			validEdges[i] = GEC_NoError;
		} catch (...) {
			validEdges[i] = GEC_InvalidSourceSocketName;
		}
	}
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
	IBK_ASSERT(pos != std::string::npos)
	std::vector<std::string> tokens;
	// first extract
	try {
		IBK::explode(simulatorDef.substr(pos+9), tokens, " \t", IBK::EF_UseQuotes | IBK::EF_TrimTokens);
		if (tokens.size() != 5)
			throw IBK::Exception("Missing properties.", FUNC_ID);
		m_cycle = IBK::string2val<unsigned int>(tokens[1]);
		m_name = IBK::trim_copy(tokens[2]);
		m_color = IBK::Color::fromHtml(tokens[3]);
		m_pathToFMU = IBK::Path( IBK::trim_copy(tokens[4], "\"") ); // mind: may be a relative path to project, and we keep it that way
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Bad format of simulator definition line '%1'.").arg(simulatorDef), FUNC_ID);
	}
}


std::string Project::GraphEdge::extractSlaveName( const std::string & variableRef) {
	std::vector<std::string> tokens;
	unsigned int count = IBK::explode_in2(variableRef, tokens, '.');
	if (count < 2)
		throw IBK::Exception(IBK::FormatString("Invalid syntax in graph variable reference '%1'.").arg(variableRef),
							 "[Project::GraphEdge::extractSlaveName]");
	return tokens[0];
}


void Project::GraphEdge::splitReference(const std::string & variableRef, std::string & slaveName, std::string & variableName) {
	size_t dotPos = variableRef.find('.');
	if (dotPos == std::string::npos || dotPos == 0)
		throw IBK::Exception(IBK::FormatString("Invalid syntax in graph variable reference '%1'.").arg(variableRef),
								 "[Project::GraphEdge::splitReference]");
	slaveName = variableRef.substr(0, dotPos);
	IBK::trim(slaveName);
	variableName = variableRef.substr(dotPos+1);
	IBK::trim(variableName);
	if (slaveName.empty() || variableName.empty())
		throw IBK::Exception(IBK::FormatString("Invalid syntax in graph variable reference '%1'.").arg(variableRef),
								 "[Project::GraphEdge::splitReference]");
}


std::string Project::GraphEdge::replaceSlaveName( const std::string & variableRef, const std::string & newSlaveName) {
	std::vector<std::string> tokens;
	unsigned int count = IBK::explode_in2(variableRef, tokens, '.');
	if (count < 2)
		throw IBK::Exception(IBK::FormatString("Invalid syntax in graph variable reference '%1'.").arg(variableRef), "[Project::GraphEdge::replaceSlaveName]");
	return newSlaveName + "." + tokens[1];
}

} // namespace MASTER_SIM
