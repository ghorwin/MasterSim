#include "MSIM_OutputWriter.h"

#include <iostream>
#include <fstream>
#include <algorithm>

#include <IBK_Exception.h>
#include <IBK_FormatString.h>
#include <IBK_messages.h>
#include <IBK_StringUtils.h>

#include "MSIM_AbstractSlave.h"
#include "MSIM_FMUSlave.h"
#include "MSIM_FMU.h"
#include "MSIM_Project.h"


namespace MASTER_SIM {

OutputWriter::OutputWriter() :
	m_project(NULL),
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
#ifdef _MSC_VER
			m_stringOutputs = new std::ofstream(stringOutputFilename.wstr().c_str(), std::ios_base::app);
#else
			m_stringOutputs = new std::ofstream(stringOutputFilename.c_str(), std::ios_base::app);
#endif
		}
		else {
#ifdef _MSC_VER
			m_stringOutputs = new std::ofstream(stringOutputFilename.wstr().c_str());
#else
			m_stringOutputs = new std::ofstream(stringOutputFilename.c_str());
#endif
			// write first line
			*m_stringOutputs << descriptions << std::endl;
		}
		m_stringOutputs->precision(14);
	}


	// now value outputs (boolean, integer and real)
	descriptions = IBK::FormatString("Time [%1]").arg(m_project->m_outputTimeUnit.name()).str();
	// collect variable references from all slaves
	for (unsigned int s=0; s<m_slaves.size(); ++s) {
		const AbstractSlave * slave = m_slaves[s];

		// loop all boolean variables in slave
		for (unsigned int v=0; v<slave->m_boolVarNames.size(); ++v) {
			std::string flatName = slave->m_name + "." + slave->m_boolVarNames[v];
			descriptions += " \t" + flatName + " [---]"; // booleans are unit-less
			m_boolOutputMapping.push_back( std::make_pair(slave, v));
		}

		// loop all integer variables in slave
		for (unsigned int v=0; v<slave->m_intVarNames.size(); ++v) {
			std::string flatName = slave->m_name + "." + slave->m_intVarNames[v];
			descriptions += " \t" + flatName + " [---]"; // ints are unit-less
			m_intOutputMapping.push_back( std::make_pair(slave, v));
		}

		// loop all real variables in slave
		for (unsigned int v=0; v<slave->m_doubleVarNames.size(); ++v) {
			std::string flatName = slave->m_name + "." + slave->m_doubleVarNames[v] + " [" + slave->m_doubleVarUnits[v] + "]";
			descriptions += " \t" + flatName;
			m_realOutputMapping.push_back( std::make_pair(slave, v));
		}

	} // for - slaves


	// finally add descriptions for all double parameters
	for (unsigned int s=0; s<m_slaves.size(); ++s) {
		const AbstractSlave * slave = m_slaves[s];
		const FMUSlave * fmuSlave = dynamic_cast<const FMUSlave *>(slave);
		if (fmuSlave != nullptr) {
			for (unsigned int v=0; v<fmuSlave->fmu()->m_modelDescription.m_variables.size(); ++v) {
				const MASTER_SIM::FMIVariable & var = fmuSlave->fmu()->m_modelDescription.m_variables[v];
				if (var.m_type == MASTER_SIM::FMIVariable::VT_DOUBLE) {
					std::string unit = var.m_unit;
					if (unit.empty())
						unit = "---";
					std::string flatName = slave->m_name + "." + var.m_name + " [" + unit + "]";
					descriptions += " \t" + flatName;
				}
			}
		}
	} // for - slaves


	{
		IBK::IBK_Message( IBK::FormatString("Creating output file 'values.csv'.\n"),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		IBK::Path outputFilename = m_resultsDir / "values.csv";
		// if we restart, we simply re-open the files for writing
		if (reopen) {
#ifdef _MSC_VER
			m_valueOutputs = new std::ofstream(outputFilename.wstr().c_str(), std::ios_base::app);
#else
			m_valueOutputs = new std::ofstream(outputFilename.c_str(), std::ios_base::app);
#endif
		}
		else {
#ifdef _MSC_VER
			m_valueOutputs = new std::ofstream(outputFilename.wstr().c_str());
#else
			m_valueOutputs = new std::ofstream(outputFilename.c_str());
#endif
			// write first line
			*m_valueOutputs << descriptions << std::endl;
		}
		m_valueOutputs->precision(14);
	}

}


void OutputWriter::setupProgressReport() {

	// statistics and progress file
	IBK::Path progressOutputFilename = m_logDir / "progress.txt";
#ifdef _MSC_VER
	m_progressOutputs = new std::ofstream(progressOutputFilename.wstr().c_str());
#else
	m_progressOutputs = new std::ofstream(progressOutputFilename.c_str());
#endif
	*m_progressOutputs << "Time\t Percentage completed [%]" << std::endl;

	/// \todo For restart, add elapsed seconds + simtime
	m_progressFeedback.setup(m_progressOutputs, m_project->m_tStart.value, m_project->m_tEnd.value, m_projectFile, 0, 0);
}


void OutputWriter::appendOutputs(double t) {
	// skip output writing, if last output was written within minimum output
	// time step size; mind rounding errors here!
	if (m_tLastOutput >= 0 && m_tLastOutput + m_project->m_hOutputMin.value > t + 1e-8) {
		m_progressFeedback.writeFeedbackFromF(t);
		return;
	}
	if (m_tLastOutput == -1) {
		// subtract a little time from last time stamp that outputs were written
		// in order to get the final progress statistics printed out for sure
		m_progressFeedback.m_lastElapsedSecondsWithOutput -= 1;
		m_tLastOutput = 0;
	}
	// increase m_tLastOutput by selected steps until t is surpassed
	while (m_tLastOutput < t)
		m_tLastOutput += m_project->m_hOutputMin.value;

	m_progressFeedback.writeFeedback(t, false);

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
	*m_valueOutputs << t;

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

	// real parameters
	for (unsigned int s=0; s<m_slaves.size(); ++s) {
		const AbstractSlave * slave = m_slaves[s];
		const FMUSlave * fmuSlave = dynamic_cast<const FMUSlave *>(slave);
		if (fmuSlave != nullptr) {
			for (unsigned int v=0; v<fmuSlave->fmu()->m_modelDescription.m_variables.size(); ++v) {
				const MASTER_SIM::FMIVariable & var = fmuSlave->fmu()->m_modelDescription.m_variables[v];
				if (var.m_type == MASTER_SIM::FMIVariable::VT_DOUBLE) {
					std::string value = var.m_startValue;
					if (value.empty())
						value = "0.0";
					*m_valueOutputs << '\t' << value;
				}
			}
		}
	} // for - slaves

	*m_valueOutputs << std::endl;


	// 2. statistics of master / counter variables

}



} // namespace MASTER_SIM
