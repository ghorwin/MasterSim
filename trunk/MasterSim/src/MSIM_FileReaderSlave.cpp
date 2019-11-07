#include "MSIM_FileReaderSlave.h"

#include <IBK_Exception.h>
#include <IBK_assert.h>
#include <IBK_messages.h>

#include <IBK_CSVReader.h>
#include <IBK_LinearSpline.h>
#include <IBK_UnitVector.h>

#include <cstdlib>
#include <cstdarg>
#include <cstdio>

#include "MSIM_FMU.h"

namespace MASTER_SIM {

FileReaderSlave::FileReaderSlave(const IBK::Path & filepath, const std::string & name) :
	AbstractSlave(name),
	m_fileReader(new IBK::CSVReader)
{
	m_filepath = filepath;
}


FileReaderSlave::~FileReaderSlave() {
	delete m_fileReader;
	for (auto d : m_valueSplines) {
		delete d;
	}
}


void FileReaderSlave::instantiate() {
	const char * const FUNC_ID = "[FileReaderSlave::instantiate]";

	IBK_ASSERT(m_valueSplines.empty()) // must only be called on empty object

	// read file
	try {
		m_fileReader->read(m_filepath, false, true);
		// special convention: no time unit, assume "s" seconds
		if (m_fileReader->m_units.size() > 0 && m_fileReader->m_units[0].empty())
			m_fileReader->m_units[0] = "s";

		if (m_fileReader->m_nRows == 0)
			throw IBK::Exception(IBK::FormatString("File '%1' does not contain any rows.").arg(m_filepath), FUNC_ID);
		if (m_fileReader->m_nColumns < 1)
			throw IBK::Exception(IBK::FormatString("File '%1' does not contain any data columns.").arg(m_filepath), FUNC_ID);
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error during initialization of slave '%1'").arg(m_name), FUNC_ID);
	}

	// setup linear splines
	unsigned int varCount = m_fileReader->m_nColumns-1;
	m_valueSplines.resize(varCount);
	m_columnVariableTypes.resize(varCount);
	for (unsigned int i=0; i<varCount; ++i)
		m_columnVariableTypes[i] = MASTER_SIM::FMIVariable::NUM_VT;

	// store variable names and units from captions
	for (unsigned int i=0; i<varCount; ++i) {
		std::string varName = m_fileReader->m_captions[i+1];
		std::string uStr = m_fileReader->m_units[i+1];
		if (uStr.empty())
			uStr = "-";
		m_typelessVarNames.push_back(varName);
		m_typelessVarUnits.push_back(uStr);
	}
}


void FileReaderSlave::setupExperiment(double /*relTol*/, double tStart, double /*tEnd*/) {
	m_t = tStart;
	/// \todo check value range in file and issue warning if data is less than simulation time frame
}


void FileReaderSlave::enterInitializationMode() {
	const char * const FUNC_ID = "[FileReaderSlave::enterInitializationMode]";
	// here, all columns/variables that are used in connections have been assigned a type

	IBK::UnitVector timeVec;
	timeVec.resize(m_fileReader->m_nRows);
	for (unsigned int i=0; i<m_fileReader->m_nRows; ++i) {
		timeVec[i] = m_fileReader->m_values[i][0];
	}
	try {
		timeVec.m_unit = IBK::Unit(m_fileReader->m_units[0]);
	} catch (...) {
		throw IBK::Exception(IBK::FormatString("Invalid/unrecognized time unit '%2' in file '%3'. Error during initialization of slave '%1'")
							 .arg(m_name).arg(m_fileReader->m_units[0]).arg(m_filepath), FUNC_ID);
	}
	// convert to seconds
	timeVec.convert(IBK::Unit("s"));

	// initialize linear splines for all vectors that hold numbers
	for (unsigned int j=0; j<m_columnVariableTypes.size(); ++j) {
		// for all but strings and unused variables we create linear splines

		if (m_columnVariableTypes[j] != FMIVariable::VT_STRING &&
			m_columnVariableTypes[j] != FMIVariable::NUM_VT)  // mind: some columns may be unused
		{
			m_valueSplines[j] = new IBK::LinearSpline;

			std::vector<double> y(m_fileReader->m_nRows);
			for (unsigned int i=0; i<m_fileReader->m_nRows; ++i) {
				y[i] = m_fileReader->m_values[i][j+1];
			}
			try {
				m_valueSplines[j]->setValues(timeVec.m_data, y);
			} catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, IBK::FormatString("Invalid interpolation table data in column '%2' in file '%3'. Error during initialization of slave '%1'")
									 .arg(m_name).arg(m_fileReader->m_captions[j+1]).arg(m_filepath), FUNC_ID);
			}
		}
		else {
			m_valueSplines[j] = nullptr;
		}
	}
}


void FileReaderSlave::exitInitializationMode() {
	// nothing to do
}


int FileReaderSlave::doStep(double stepSize, bool /*noSetFMUStatePriorToCurrentPoint*/) {
	const char * const FUNC_ID = "[FileReaderSlave::doStep]";
	IBK_FastMessage(IBK::VL_DEVELOPER)(IBK::FormatString("Interval [%1 ... %2] with h=%3\n")
									   .arg(m_t,22,'e',15).arg(m_t+stepSize,22,'e',15).arg(stepSize,16,'e',12),
			IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DEVELOPER);

	m_t += stepSize; // advance current time
	cacheOutputs();
	return fmi2OK;
}


void FileReaderSlave::currentState(fmi2FMUstate * /*state*/) const {
	// basically just the time point, but that's not worth saving
}


void FileReaderSlave::setState(double t, fmi2FMUstate /*slaveState*/) {
	m_t = t;
}


void FileReaderSlave::cacheOutputs() {
	const char * const FUNC_ID = "[FileReaderSlave::cacheOutputs]";
	int res = fmi2OK;

	unsigned int idxDouble = 0;
	unsigned int idxInt = 0;
	unsigned int idxBool = 0;
	unsigned int idxString = 0;
	for (unsigned int j=0; j<m_columnVariableTypes.size(); ++j) {
		switch (m_columnVariableTypes[j]) {
			case MASTER_SIM::FMIVariable::VT_DOUBLE :
				m_doubleOutputs[idxDouble++] = m_valueSplines[j]->value(m_t);
			break;
			case MASTER_SIM::FMIVariable::VT_INT :
				m_intOutputs[idxInt++] = m_valueSplines[j]->nonInterpolatedValue(m_t);
			break;
			case MASTER_SIM::FMIVariable::VT_BOOL :
				m_boolOutputs[idxBool++] = m_valueSplines[j]->nonInterpolatedValue(m_t);
			break;
			case MASTER_SIM::FMIVariable::VT_STRING : break; // TODO : later store string variables
			case MASTER_SIM::FMIVariable::NUM_VT : break; // nothing to do
		}
	}

	if (res != fmi2OK)	throw IBK::Exception("Error retrieving values from slave.", FUNC_ID);
}


} // namespace MASTER_SIM
