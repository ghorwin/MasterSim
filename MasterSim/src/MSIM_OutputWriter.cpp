#include "MSIM_OutputWriter.h"

#include <iostream>
#include <fstream>

#include <DATAIO_DataIO.h>

#include <IBK_Exception.h>
#include <IBK_FormatString.h>
#include <IBK_messages.h>
#include <IBK_StringUtils.h>

#include "MSIM_Slave.h"
#include "MSIM_FMU.h"
#include "MSIM_Project.h"
//#include "MSIM_ModelDescription.h"

namespace MASTER_SIM {

OutputWriter::OutputWriter() :
	m_tLastOutput(-1),
	m_stringOutputs(NULL)
{
}


OutputWriter::~OutputWriter() {
	// release output files
	for (unsigned int i=0; i<m_outputFiles.size(); ++i)
		delete m_outputFiles[i].m_dataIO;

	for (unsigned int i=0; i<m_realOutputFiles.size(); ++i)
		delete m_realOutputFiles[i].m_dataIO;

	delete m_stringOutputs;
}


void OutputWriter::openOutputFiles(bool reopen) {
	const char * const FUNC_ID = "[OutputWriter::openOutputFile]";

	// close existing output files if present
	if (!m_outputFiles.empty()) {
		for (unsigned int i=0; i<m_outputFiles.size(); ++i)
			delete m_outputFiles[i].m_dataIO;
		m_outputFiles.clear();
	}
	if (!m_realOutputFiles.empty()) {
		for (unsigned int i=0; i<m_realOutputFiles.size(); ++i)
			delete m_realOutputFiles[i].m_dataIO;
		m_realOutputFiles.clear();
	}

	// check if result path exists, otherwise create it
	if (!m_resultsDir.exists()) {
		if (!IBK::Path::makePath(m_resultsDir))
			throw IBK::Exception(IBK::FormatString("Cannot create directory for results '%1'.").arg(m_resultsDir), FUNC_ID);
	}

	std::vector<IBK::Path> fileNames;
	// compose path to output file
	fileNames.push_back(m_resultsDir / "boolean.d6o");
	fileNames.push_back(m_resultsDir / "integer.d6o");

	IBK::IBK_Message("Creating output files\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::MessageIndentor indent; (void)indent;


	// create output files for booleans and integer

	for (unsigned int fileType=0; fileType<2; ++fileType) {
		m_outputFiles.push_back(OutputFileData());
		OutputFileData & outputFileData = m_outputFiles.back();
		outputFileData.m_dataIO = new DATAIO::DataIO;
		DATAIO::DataIO * dataIO = outputFileData.m_dataIO;
		const IBK::Path & outputFile = fileNames[fileType];

		if (reopen) {
			try {
				dataIO->reopenForWriting(outputFile);
				/// \todo check header read from file for consistency
				continue;
			}
			catch (IBK::Exception & ex) {
				IBK::IBK_Message(IBK::FormatString("%1").arg(ex.what()), IBK::MSG_WARNING, FUNC_ID);
				IBK::IBK_Message(IBK::FormatString("Cannot reopen output file '%1', creating new one instead.").arg(outputFile), IBK::MSG_WARNING, FUNC_ID);
			}
		}

		dataIO->m_projectFileName = m_projectFile;

		// populate heder
		dataIO->m_isBinary = false;

		// no geometry file
		dataIO->m_geoFileHash = 0;
		dataIO->m_geoFileName.clear();

		// we write sets of scalar variables
		dataIO->m_type = DATAIO::DataIO::T_REFERENCE;

		// no space or time integration
		dataIO->m_spaceType = DATAIO::DataIO::ST_SINGLE;
		dataIO->m_timeType = DATAIO::DataIO::TT_NONE;

		// encode quantity to list all variable names
		unsigned int varCount = 0;
		for (unsigned int s=0; s<m_slaves.size(); ++s) {
			const Slave * slave = m_slaves[s];

			switch (fileType) {
				case OF_BOOLEAN : {
					// loop all boolean variables in slave
					for (unsigned int v=0; v<slave->fmu()->m_boolValueRefsOutput.size(); ++v) {
						const FMIVariable & var = slave->fmu()->m_modelDescription.variableByRef(slave->fmu()->m_boolValueRefsOutput[v]);
						dataIO->m_quantity += slave->m_name + "." + var.m_name + " | ";
						dataIO->m_nums.push_back(var.m_valueReference);
						m_outputFiles[fileType].m_outputMapping.push_back( std::make_pair(slave, v));
						++varCount;
					}
				} break;
				case OF_INTEGER : {
					// loop all boolean variables in slave
					for (unsigned int v=0; v<slave->fmu()->m_intValueRefsOutput.size(); ++v) {
						const FMIVariable & var = slave->fmu()->m_modelDescription.variableByRef(slave->fmu()->m_intValueRefsOutput[v]);
						dataIO->m_quantity += slave->m_name + "." + var.m_name + " | ";
						dataIO->m_nums.push_back(var.m_valueReference);
						m_outputFiles[fileType].m_outputMapping.push_back( std::make_pair(slave, v));
						++varCount;
					}
				} break;

			} // switch
		} // for
		if (varCount == 0) {
			IBK::IBK_Message( IBK::FormatString("Skipping output file '%1', no outputs of this type are generated\n")
							  .arg(outputFile.filename()), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			delete m_outputFiles[fileType].m_dataIO;
			m_outputFiles[fileType].m_dataIO = NULL;
			continue;
		}
		else {
			dataIO->m_quantity = dataIO->m_quantity.substr(0, dataIO->m_quantity.size()-3);
		}

		dataIO->m_timeUnit = m_project->m_outputTimeUnit.name();
		dataIO->m_valueUnit = "---"; // boolean and integer get the undefined unit

		dataIO->m_filename = outputFile;
		try {
			IBK::IBK_Message( IBK::FormatString("Creating output file '%1' with '%2' outputs.\n").arg(outputFile).arg(varCount),
							  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			dataIO->writeHeader();
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error creating output file '%1'.").arg(outputFile), FUNC_ID);
		}
	}

	// for string outputs, things are fairly simple as well
	std::string descriptions;
	// collect output variable references for strings
	for (unsigned int s=0; s<m_slaves.size(); ++s) {
		const Slave * slave = m_slaves[s];

		// loop all string variables in slave
		for (unsigned int v=0; v<slave->fmu()->m_stringValueRefsOutput.size(); ++v) {
			const FMIVariable & var = slave->fmu()->m_modelDescription.variableByRef(slave->fmu()->m_stringValueRefsOutput[v]);
			descriptions += slave->m_name + "." + var.m_name + " \t ";
			m_stringOutputMapping.push_back( std::make_pair(slave, v));
		}
	} // for
	// create output file if at least one output of type string is written
	IBK::Path stringOutputFilename = m_resultsDir / "strings.csv";
	if (m_stringOutputMapping.empty()) {
		IBK::IBK_Message( IBK::FormatString("Skipping output file 'strings.csv', no outputs of this type are generated\n"),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	}
	else {
		IBK::IBK_Message( IBK::FormatString("Creating output file 'strings.csv' with '%1' outputs.\n").arg(m_stringOutputMapping.size()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
#ifdef _MSC_VER
		m_stringOutputs = new std::ofstream(stringOutputFilename.wstr().c_str());
#else
		m_stringOutputs = new std::ofstream(stringOutputFilename.c_str());
#endif
		*m_stringOutputs << descriptions << std::endl;
	}


	// for double outputs, we first create a vector with all output units and the corresponding output references
	// afterwards the files are created
	std::vector<std::string>	units;

	for (unsigned int s=0; s<m_slaves.size(); ++s) {
		const Slave * slave = m_slaves[s];

		// loop over all real variables in slave
		for (unsigned int v=0; v<slave->fmu()->m_doubleValueRefsOutput.size(); ++v) {
			const FMIVariable & var = slave->fmu()->m_modelDescription.variableByRef(slave->fmu()->m_doubleValueRefsOutput[v]);
			// try to find unit
			unsigned int u=0;
			for (; u<units.size(); ++u)
				if (units[u] == var.m_unit)
					break;
			// not found? create new file
			OutputFileData * outputFileData = NULL;
			DATAIO::DataIO * dataIO = NULL;
			if (u == units.size()) {
				// remember unit
				units.push_back(var.m_unit);
				// add new file data
				m_realOutputFiles.push_back(OutputFileData());
				// create new dataIO container
				outputFileData = &m_realOutputFiles.back(); // get pointer already in vector (resource already owned)
				outputFileData->m_dataIO = new DATAIO::DataIO; //  create new dataIO container and store in struct (becomes owned)
				dataIO = outputFileData->m_dataIO; // get convenience pointer
//					try {
//						dataIO->reopenForWriting(outputFile);
//						/// \todo check header read from file for consistency
//						continue;
//					}
//					catch (IBK::Exception & ex) {
//						IBK::IBK_Message(IBK::FormatString("%1").arg(ex.what()), IBK::MSG_WARNING, FUNC_ID);
//						IBK::IBK_Message(IBK::FormatString("Cannot reopen output file '%1', creating new one instead.").arg(outputFile), IBK::MSG_WARNING, FUNC_ID);
//					}
//				}

				dataIO->m_projectFileName = m_projectFile;

				// populate heder
				dataIO->m_isBinary = false;

				// no geometry file
				dataIO->m_geoFileHash = 0;
				dataIO->m_geoFileName.clear();

				// we write sets of scalar variables
				dataIO->m_type = DATAIO::DataIO::T_REFERENCE;

				// no space or time integration
				dataIO->m_spaceType = DATAIO::DataIO::ST_SINGLE;
				dataIO->m_timeType = DATAIO::DataIO::TT_NONE;

				dataIO->m_timeUnit = m_project->m_outputTimeUnit.name();
				if (var.m_unit.empty())
					dataIO->m_valueUnit = "---";
				else
					dataIO->m_valueUnit = var.m_unit;

				std::string fname = IBK::FormatString("real_%1.d6o").arg(dataIO->m_valueUnit).str();
				fname = IBK::replace_string(fname, "/", "_");
				dataIO->m_filename = m_resultsDir / fname;
			}
			else {
				outputFileData = &m_realOutputFiles[u];
				dataIO = outputFileData->m_dataIO;
			}

			// remember variable name and store reference
			dataIO->m_quantity += slave->m_name + "." + var.m_name + " | ";
			dataIO->m_nums.push_back(var.m_valueReference);
			outputFileData->m_outputMapping.push_back( std::make_pair(slave, v));

		}
	} // for

	// now that we have collected the mappings, create the files
	if (m_realOutputFiles.empty()) {
		IBK::IBK_Message( IBK::FormatString("Skipping output files of type real, no outputs of this type are generated\n"),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	}
	else {
		for (unsigned int i=0; i<m_realOutputFiles.size(); ++i) {
			const std::string & outputFile = m_realOutputFiles[i].m_dataIO->m_filename.str();
			try {
				IBK::IBK_Message( IBK::FormatString("Creating output file '%1' with '%2' outputs.\n")
								  .arg(outputFile).arg(m_realOutputFiles[i].m_outputMapping.size()),
								  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
				m_realOutputFiles[i].m_dataIO->writeHeader();
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, IBK::FormatString("Error creating output file '%1' of type real.")
									 .arg(outputFile), FUNC_ID);
			}
		}

	}

	unsigned int nValues = 0;
	nValues = std::max<unsigned int>(nValues, m_outputFiles[0].m_outputMapping.size());
	nValues = std::max<unsigned int>(nValues, m_outputFiles[1].m_outputMapping.size());
	for (std::vector<OutputFileData>::const_iterator outputFileDataIt = m_realOutputFiles.begin(); outputFileDataIt != m_realOutputFiles.end(); ++outputFileDataIt)
		nValues = std::max<unsigned int>(nValues, outputFileDataIt->m_outputMapping.size());
	m_valueVector.resize(nValues);
}


void OutputWriter::writeOutputs(double t) {
	// skip output writing, if last output was written within minimum output
	// time step size
	if (m_tLastOutput >= 0 && m_tLastOutput + m_project->m_tOutputStepMin > t)
		return;

	m_tLastOutput = t;

	// dump state of master to output files

	// 1. state of output variables

	// boolean and integer outputs first
	OutputFileData & outputFileData = m_outputFiles[OF_BOOLEAN];
	if (outputFileData.m_dataIO != NULL) {
		for (unsigned int i=0; i<outputFileData.m_outputMapping.size(); ++i) {
			const std::pair<const Slave*, unsigned int> & outRef = outputFileData.m_outputMapping[i];
			// gather all data in output file
			m_valueVector[i] = outRef.first->m_boolOutputs[outRef.second];
		}
		outputFileData.m_dataIO->appendData(t, &m_valueVector[0]);
	}

	outputFileData = m_outputFiles[OF_INTEGER];
	if (outputFileData.m_dataIO != NULL) {
		for (unsigned int i=0; i<outputFileData.m_outputMapping.size(); ++i) {
			const std::pair<const Slave*, unsigned int> & outRef = outputFileData.m_outputMapping[i];
			// gather all data in output file
			m_valueVector[i] = outRef.first->m_intOutputs[outRef.second];
		}
		outputFileData.m_dataIO->appendData(t, &m_valueVector[0]);
	}

	// now all real outputs
	for (std::vector<OutputFileData>::const_iterator it = m_realOutputFiles.begin(); it != m_realOutputFiles.end(); ++it) {
		for (unsigned int i=0; i<outputFileData.m_outputMapping.size(); ++i) {
			const std::pair<const Slave*, unsigned int> & outRef = it->m_outputMapping[i];
			// gather all data in output file
			m_valueVector[i] = outRef.first->m_doubleOutputs[outRef.second];
		}
		outputFileData.m_dataIO->appendData(t, &m_valueVector[0]);
	}

	// string outputs
	if (m_stringOutputs != NULL) {
		for (std::vector< std::pair<const Slave*, unsigned int> >::const_iterator it = m_stringOutputMapping.begin(); it != m_stringOutputMapping.end(); ++it) {
			*m_stringOutputs << it->first->m_stringOutputs[it->second] << " \t ";
			// gather all data in output file
		}
		*m_stringOutputs << std::endl;
	}


	// 2. statistics of master / counter variables

}



} // namespace MASTER_SIM
