#include "MSIM_FileReaderSlave.h"

#include <IBK_Exception.h>
#include <IBK_assert.h>
#include <IBK_messages.h>

#include <cstdlib>
#include <cstdarg>
#include <cstdio>

#include "MSIM_FMU.h"

namespace MASTER_SIM {

FileReaderSlave::FileReaderSlave(const IBK::Path & filepath, const std::string & name) :
	AbstractSlave(name)
{
	m_filepath = filepath;
}


FileReaderSlave::~FileReaderSlave() {
}


void FileReaderSlave::instantiate() {

	// read file

	// setup linear splines

	// resize vectors
//	m_boolOutputs.resize(m_fmu->m_boolValueRefsOutput.size());
//	m_intOutputs.resize(m_fmu->m_intValueRefsOutput.size());
//	m_doubleOutputs.resize(m_fmu->m_doubleValueRefsOutput.size());
//	m_stringOutputs.resize(m_fmu->m_stringValueRefsOutput.size());
}


void FileReaderSlave::setupExperiment(double relTol, double tStart, double tEnd) {
	m_t = tStart;
	// check value range in file and issue warning if data is less than simulation time frame
}


void FileReaderSlave::enterInitializationMode() {
	// nothing to do
}


void FileReaderSlave::exitInitializationMode() {
	// nothing to do
}


int FileReaderSlave::doStep(double stepSize, bool noSetFMUStatePriorToCurrentPoint) {
	const char * const FUNC_ID = "[FileReaderSlave::doStep]";
	IBK_FastMessage(IBK::VL_DEVELOPER)(IBK::FormatString("Interval [%1 ... %2] with h=%3\n")
									   .arg(m_t,22,'e',15).arg(m_t+stepSize,22,'e',15).arg(stepSize,16,'e',12),
			IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DEVELOPER);

	m_t += stepSize; // advance current time
	cacheOutputs();
	return fmi2OK;
}


void FileReaderSlave::currentState(fmi2FMUstate * state) const {
	// basically just the time point, but that's not worth saving
}


void FileReaderSlave::setState(double t, fmi2FMUstate /*slaveState*/) {
	m_t = t;
}


void FileReaderSlave::cacheOutputs() {
	const char * const FUNC_ID = "[FileReaderSlave::cacheOutputs]";
	int res = fmi2OK;

	// apply spline interpolation

	if (res != fmi2OK)	throw IBK::Exception("Error retrieving values from slave.", FUNC_ID);
}


} // namespace MASTER_SIM
