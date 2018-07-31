/* Common implementation of the FMI Interface Data Structure.
*/

#include "fmi2Functions.h"
#include "fmi2FunctionTypes.h"
#include "InstanceData.h"

#include <IBK_assert.h>
#include <IBK_messages.h>


InstanceData::InstanceData() :
	m_callbackFunctions(0),
	m_initializationMode(false),
	m_modelExchange(true),
	m_tInput(0),
	m_externalInputVarsModified(false),
	m_messageHandlerPtr(NULL),
	m_fmuStateSize(0)
{
}


InstanceData::~InstanceData() {
	delete m_messageHandlerPtr; // MessageHandler automatically resets DefaultMessageHandler upon deletion
	for (std::set<void*>::iterator it = m_fmuStates.begin(); it != m_fmuStates.end(); ++it) {
		free(*it);
	}
}


// only for ModelExchange
void InstanceData::updateIfModified() {

}


// only for Co-simulation
void InstanceData::integrateTo(double /*tCommunicationIntervalEnd*/) {

}


void InstanceData::logger(fmi2Status state, fmi2String category, const IBK::FormatString & message) {
	if (m_loggingOn) {
		m_callbackFunctions->logger(m_callbackFunctions->componentEnvironment,
									m_instanceName.c_str(), state, category,
									message.str().c_str());
	}
}


void InstanceData::setupMessageHandler(const IBK::Path & logfile) {
	const char * const FUNC_ID = "[InstanceData::setupMessageHandler]";

	if (m_messageHandlerPtr != NULL)
		throw IBK::Exception(IBK::FormatString("Message handler must not be created/initialized twice."), FUNC_ID);

	unsigned int verbosityLevel = IBK::VL_STANDARD;
	m_messageHandlerPtr = new IBK::MessageHandler;
	m_messageHandlerPtr->setConsoleVerbosityLevel(0); // disable console output
	m_messageHandlerPtr->setLogfileVerbosityLevel(verbosityLevel);
	m_messageHandlerPtr->m_contextIndentation = 48;
	std::string errmsg;
	if (!IBK::Path::makePath(logfile.parentPath()))
		throw IBK::Exception(IBK::FormatString("Error creating log directory '%1'.").arg(logfile.parentPath().absolutePath()), FUNC_ID);
	bool success = m_messageHandlerPtr->openLogFile(logfile.str(), false, errmsg);
	if (!success)
		throw IBK::Exception(IBK::FormatString("Error opening logfile '%1': %2").arg(logfile).arg(errmsg), FUNC_ID);
	IBK::MessageHandlerRegistry::instance().setMessageHandler(m_messageHandlerPtr);
}


template <typename T>
void checkIfIDExists(const T & m, int varID) {
	if (m.find(varID) == m.end() )
		throw IBK::Exception("Invalid or unknown ID.", "[checkIfIDExists]");

}

void InstanceData::setRealParameter(int varID, double value) {
	checkIfIDExists(m_realInput, varID);
	m_realInput[varID] = value;
	m_externalInputVarsModified = true;
}


void InstanceData::setIntParameter(int varID, int value) {
	checkIfIDExists(m_integerInput, varID);
	m_integerInput[varID] = value;
	m_externalInputVarsModified = true;
}


void InstanceData::setStringParameter(int varID, fmi2String value) {
	// special handling for ResultsRootDir parameter
	if (varID == 42)
		m_resultsRootDir = value;
	else {
		checkIfIDExists(m_stringInput, varID);
		m_stringInput[varID] = value;
	}
	m_externalInputVarsModified = true;
}


void InstanceData::setBoolParameter(int varID, bool value) {
	checkIfIDExists(m_boolInput, varID);
	m_boolInput[varID] = value;
	m_externalInputVarsModified = true;
}


void InstanceData::getRealParameter(int varID, double & value) {
	// update procedure for model exchange
	if (m_modelExchange)
		updateIfModified();
	checkIfIDExists(m_realOutput, varID);
	value = m_realOutput[varID];
}


void InstanceData::getIntParameter(int varID, int & value) {
	// update procedure for model exchange
	if(m_modelExchange)
		updateIfModified();
	checkIfIDExists(m_integerOutput, varID);
	value = m_integerOutput[varID];
}


void InstanceData::getStringParameter(int varID, fmi2String & value) {
	// update procedure for model exchange
	if(m_modelExchange)
		updateIfModified();
	checkIfIDExists(m_stringOutput, varID);
	value = m_stringOutput[varID].c_str();
}


void InstanceData::getBoolParameter(int varID, bool & value) {
	// update procedure for model exchange
	if(m_modelExchange)
		updateIfModified();
	checkIfIDExists(m_boolOutput, varID);
	value = m_boolOutput[varID];
}


void InstanceData::completedIntegratorStep() {
	// this function must only be called in ModelExchange mode!!!
	IBK_ASSERT(m_modelExchange);
	updateIfModified();
	completedIntegratorStep(m_tInput, &m_yInput[0]);
}

