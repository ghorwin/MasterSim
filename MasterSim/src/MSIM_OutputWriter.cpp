#include "MSIM_OutputWriter.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <memory>

#include <IBK_Exception.h>
#include <IBK_FormatString.h>
#include <IBK_messages.h>
#include <IBK_StringUtils.h>
#include <IBK_UnitList.h>
#include <IBK_FileUtils.h>

#include "MSIM_AbstractSlave.h"
#include "MSIM_FMUSlave.h"
#include "MSIM_FMU.h"
#include "MSIM_Project.h"


namespace MASTER_SIM {

OutputWriter::OutputWriter() :
	m_project(NULL),
	m_tEarliestOutputTime(-1),
	m_tLastOutput(-1),
	m_valueOutputs(NULL),
	m_stringOutputs(NULL),
	m_progressOutputs(NULL)
{
}


OutputWriter::~OutputWriter() {
	delete m_valueOutputs;
	delete m_stringOutputs;
	delete m_progressOutputs;
}


void OutputWriter::openOutputFiles(bool reopen) {
	const char * const FUNC_ID = "[OutputWriter::openOutputFile]";

	// check if result path exists, otherwise create it
	if (!m_resultsDir.exists()) {
		if (!IBK::Path::makePath(m_resultsDir))
			throw IBK::Exception(IBK::FormatString("Cannot create directory for results '%1'.").arg(m_resultsDir), FUNC_ID);
	}

	if (m_project->m_hOutputMin.value <= 0)
		throw IBK::Exception(IBK::FormatString("Invalid hOutputMin parameter: %1").arg(m_project->m_hOutputMin.toString(true)), FUNC_ID);

	// first string outputs, collect variables of type string from all slaves and create
	// a map of output column index to slave and variable index
	std::string descriptions = IBK::FormatString("Time [%1]").arg(m_project->m_outputTimeUnit.name()).str();
	// collect output variable references for strings
	for (unsigned int s=0; s<m_slaves.size(); ++s) {
		const AbstractSlave * slave = m_slaves[s];

		// loop all string variables in slave
		for (unsigned int v=0; v<slave->m_stringVarNames.size(); ++v) {
			std::string flatName = slave->m_name + "." + slave->m_stringVarNames[v];
			descriptions += " \t" + flatName;
			m_stringOutputMapping.push_back( std::make_pair(slave, v));
		}
	} // for - slaves

	// create output file if at least one output of type string is written
	IBK::Path stringOutputFilename = m_resultsDir / "strings.csv";
	if (m_stringOutputMapping.empty()) {
		IBK::IBK_Message( IBK::FormatString("Skipping output file 'strings.csv', no outputs of this type are generated.\n"),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	}
	else {
		IBK::IBK_Message( IBK::FormatString("Creating output file 'strings.csv' with %1 outputs.\n").arg(m_stringOutputMapping.size()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		// if we restart, we simply re-open the files for writing
		if (reopen) {
			m_stringOutputs = IBK::create_ofstream(stringOutputFilename, std::ios_base::app);
		}
		else {
			m_stringOutputs = IBK::create_ofstream(stringOutputFilename);

			// write first line
			*m_stringOutputs << descriptions << std::endl;
		}
		m_stringOutputs->precision(14);
	}


	// now value outputs (boolean, integer and real)
	descriptions = IBK::FormatString("Time [%1]").arg(m_project->m_outputTimeUnit.name()).str();
	std::string boolDescriptions;
	std::string intDescriptions;
	std::string realDescriptions;
	int outputVars = 0;
	// collect variable references from all slaves
	for (unsigned int s=0; s<m_slaves.size(); ++s) {
		const AbstractSlave * slave = m_slaves[s];

		// loop all boolean variables in slave
		for (unsigned int v=0; v<slave->m_boolVarNames.size(); ++v) {
			std::string flatName = slave->m_name + "." + slave->m_boolVarNames[v];
			boolDescriptions += " \t" + flatName + " [-]"; // booleans are unit-less
			m_boolOutputMapping.push_back( std::make_pair(slave, v));
			++outputVars;
		}

		// loop all integer variables in slave
		for (unsigned int v=0; v<slave->m_intVarNames.size(); ++v) {
			std::string flatName = slave->m_name + "." + slave->m_intVarNames[v];
			intDescriptions += " \t" + flatName + " [-]"; // ints are unit-less
			m_intOutputMapping.push_back( std::make_pair(slave, v));
			++outputVars;
		}

		// loop all real variables in slave
		for (unsigned int v=0; v<slave->m_doubleVarNames.size(); ++v) {
			std::string flatName = slave->m_name + "." + slave->m_doubleVarNames[v] + " [" + slave->m_doubleVarUnits[v] + "]";
			realDescriptions += " \t" + flatName;
			m_realOutputMapping.push_back( std::make_pair(slave, v));
			++outputVars;
		}

	} // for - slaves


//#define DUMP_PARAMETERS
#ifdef DUMP_PARAMETERS
	// add descriptions for all double parameters
	for (unsigned int s=0; s<m_slaves.size(); ++s) {
		const AbstractSlave * slave = m_slaves[s];
		const FMUSlave * fmuSlave = dynamic_cast<const FMUSlave *>(slave);
		if (fmuSlave != nullptr) {
			for (unsigned int v=0; v<fmuSlave->fmu()->m_modelDescription.m_variables.size(); ++v) {
				const MASTER_SIM::FMIVariable & var = fmuSlave->fmu()->m_modelDescription.m_variables[v];
				if (var.m_type == MASTER_SIM::FMIVariable::VT_DOUBLE) {
					std::string unit = var.m_unit;
					if (unit.empty())
						unit = "-";
					std::string flatName = slave->m_name + "." + var.m_name + " [" + unit + "]";
					descriptions += " \t" + flatName;
				}
			}
		}
	} // for - slaves
#endif // DUMP_PARAMETERS

	if (outputVars == 0) {
		IBK::IBK_Message("No output quantities defined in slaves, MasterSim will not generate any results!",
						 IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
	}


	{
		IBK::IBK_Message( IBK::FormatString("Creating output file 'values.csv'.\n"),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		IBK::Path outputFilename = m_resultsDir / "values.csv";
		// if we restart, we simply re-open the files for writing
		if (reopen) {
			m_valueOutputs = IBK::create_ofstream(outputFilename, std::ios_base::app);
		}
		else {
			m_valueOutputs = IBK::create_ofstream(outputFilename);

			// write first line
			*m_valueOutputs << descriptions // XXX needs to be same order as we write the data
				<< boolDescriptions
				<< intDescriptions
				<< realDescriptions
				<< std::endl;
		}
		m_valueOutputs->precision(14);
	}


	// finally, also create the "synonymous variables" file

	std::stringstream synonymousVars;
	std::set<const FMU * > processedFMUs;
	for (unsigned int s=0; s<m_slaves.size(); ++s) {
		const AbstractSlave * slave = m_slaves[s];
		// skip all but FMUslaves
		const FMUSlave * fmuSlave = dynamic_cast<const FMUSlave*>(slave);
		if (fmuSlave == nullptr)
			continue;
		// check (in case of multiple instances of fmu slaves), if the fmu was already processed
		const FMU * fmuPtr = fmuSlave->fmu();
		if (processedFMUs.find(fmuPtr) != processedFMUs.end())
			continue;
		processedFMUs.insert(fmuPtr);

		// now process map with synonymous variables
		for (auto m : fmuPtr->m_synonymousVars) {
			const std::pair<FMIVariable::VarType, unsigned int> & valueRef = m.first;
			// lookup written variable name
			const FMIVariable & var = fmuPtr->m_modelDescription.variableByRef(valueRef.first, valueRef.second);
			// process all variable synonyms in table
			for (auto varName : m.second) {
				// first column: fmu file path (currently only filename; full path is only needed when different fmus with same name are used)
				// second column: variable name that appears in 'values.csv' file
				// other columns: variables with same value reference
				synonymousVars << fmuPtr->fmuFilePath().filename().str() << '\t' << var.m_name << '\t' << varName << '\n';
			}
		}
	}
	// if we have at least one synonymous variable, write the respective file
	std::string fileContent = synonymousVars.str();
	if (fileContent.size() != 0) {
		IBK::Path synFilename = m_resultsDir / "synonymous_variables.txt";
		std::ofstream synStream;
		if (!IBK::open_ofstream(synStream, synFilename)) {
			IBK::IBK_Message(IBK::FormatString("Cannot open file '%1' for writing.").arg(synFilename), IBK::MSG_WARNING, FUNC_ID);
		}
		synStream << synonymousVars.rdbuf();
	}
}


void OutputWriter::setupProgressReport() {

	// statistics and progress file
	IBK::Path progressOutputFilename = m_logDir / "progress.txt";
	m_progressOutputs = IBK::create_ofstream(progressOutputFilename);

	/// \todo For restart, add elapsed seconds + simtime
	m_progressFeedback.setup(m_progressOutputs, m_project->m_tStart.value, m_project->m_tEnd.value, m_projectFile, 0, 0);
}


void OutputWriter::appendOutputs(double t) {
	// skip output writing, if last output was written within minimum output
	// time step size; mind rounding errors here!
	if (m_tEarliestOutputTime >= 0 && m_tEarliestOutputTime > t) {
		m_progressFeedback.writeFeedbackFromF(t);
		return;
	}
	if (m_tEarliestOutputTime == -1) {
		// subtract a little time from last time stamp that outputs were written
		// in order to get the final progress statistics printed out for sure
		m_progressFeedback.m_lastElapsedSecondsWithOutput -= 1;
		m_tEarliestOutputTime = 0;
	}
	// increase m_tEarliestOutputTime by selected steps until t is surpassed,
	// this will be the first time that we allow next outputs to be made again
	// we use the multiplication method here, since this avoids accumulation of rounding errors
	int i = 0;
	while (++i*m_project->m_hOutputMin.value <= t);
	m_tEarliestOutputTime = i*m_project->m_hOutputMin.value;

	m_progressFeedback.writeFeedback(t, false);
	m_tLastOutput = t; // remember this output time

	// 1. dump state of master to output files

	// string outputs
	if (m_stringOutputs != NULL) {
		*m_stringOutputs << t;
		for (std::vector< std::pair<const AbstractSlave*, unsigned int> >::const_iterator it = m_stringOutputMapping.begin(); it != m_stringOutputMapping.end(); ++it) {
			*m_stringOutputs << '\t' << it->first->m_stringOutputs[it->second];
			// gather all data in output file
		}
		*m_stringOutputs << std::endl;
	}

	// value outputs

	// perform time unit conversion
	double tOut = t;
	IBK::UnitList::instance().convert(IBK::Unit("s"), IBK::Unit(m_project->m_outputTimeUnit), tOut);
	*m_valueOutputs << tOut;

	// booleans
	for (std::vector< std::pair<const AbstractSlave*, unsigned int> >::const_iterator it = m_boolOutputMapping.begin();
		 it != m_boolOutputMapping.end(); ++it)
	{
		*m_valueOutputs << '\t' << it->first->m_boolOutputs[it->second];
	}
	// integer
	for (std::vector< std::pair<const AbstractSlave*, unsigned int> >::const_iterator it = m_intOutputMapping.begin();
		 it != m_intOutputMapping.end(); ++it)
	{
		*m_valueOutputs << '\t' << it->first->m_intOutputs[it->second];
	}
	// real
	for (std::vector< std::pair<const AbstractSlave*, unsigned int> >::const_iterator it = m_realOutputMapping.begin();
		 it != m_realOutputMapping.end(); ++it)
	{
		*m_valueOutputs << '\t' << it->first->m_doubleOutputs[it->second];
	}

#ifdef DUMP_PARAMETERS
	// real parameters
	for (unsigned int s=0; s<m_slaves.size(); ++s) {
		const AbstractSlave * slave = m_slaves[s];
		const FMUSlave * fmuSlave = dynamic_cast<const FMUSlave *>(slave);
		if (fmuSlave != nullptr) {
			for (unsigned int v=0; v<fmuSlave->fmu()->m_modelDescription.m_variables.size(); ++v) {
				const MASTER_SIM::FMIVariable & var = fmuSlave->fmu()->m_modelDescription.m_variables[v];
				if (var.m_type == MASTER_SIM::FMIVariable::VT_DOUBLE) {
					// query current parameter value from slave
					std::string value = var.m_startValue;
					if (value.empty())
						value = "0.0";
					*m_valueOutputs << '\t' << value;
				}
			}
		}
	} // for - slaves
#endif // DUMP_PARAMETERS

	*m_valueOutputs << std::endl;


	// 2. statistics of master / counter variables

}



} // namespace MASTER_SIM
