#include "MSIM_Slave.h"

#include <IBK_Exception.h>
#include <IBK_assert.h>
#include <IBK_messages.h>

#include <cstdlib>
#include <cstdarg>
#include <cstdio>

#include "MSIM_FMU.h"

namespace MASTER_SIM {

void fmiLoggerCallback( fmiComponent /* c */, fmiString instanceName, fmiStatus status,
						fmiString category, fmiString message, ... )
{
	IBK::msg_type_t msgType = IBK::MSG_PROGRESS;
	switch (status) {
		case fmi2Warning	: msgType = IBK::MSG_WARNING; break;
		case fmi2Error		: msgType = IBK::MSG_ERROR; break;
		default :;
	}

	static char buffer[5000];
	va_list args;
	va_start (args, message);
#if defined(_WIN32)
	#if defined(_MSC_VER)
		vsnprintf_s(buffer, 5000, 4999, message, args);
	#else
		vsnprintf(buffer, 5000, message, args);
	#endif
#else
	std::vsnprintf(buffer, 5000, message, args);
#endif
	IBK::IBK_Message( IBK::FormatString("[%1:%2] %3\n").arg(instanceName).arg(category).arg(buffer), msgType, "[fmiLoggerCallback]", IBK::VL_INFO);
}


void fmi2LoggerCallback( fmi2ComponentEnvironment /* c */, fmi2String instanceName,
						 fmi2Status status, fmi2String category, fmi2String message, ... )
{
	IBK::msg_type_t msgType = IBK::MSG_PROGRESS;
	switch (status) {
		case fmi2Warning	: msgType = IBK::MSG_WARNING; break;
		case fmi2Error		: msgType = IBK::MSG_ERROR; break;
		default :;
	}

	static char buffer[5000];
	va_list args;
	va_start (args, message);
#if defined(_WIN32)
	#if defined(_MSC_VER)
		vsnprintf_s(buffer, 5000, 4999, message, args);
	#else
		vsnprintf(buffer, 5000, message, args);
	#endif
#else
	std::vsnprintf(buffer, 5000, message, args);
#endif
	IBK::IBK_Message( IBK::FormatString("[%1:%2] %3\n").arg(instanceName).arg(category).arg(buffer), msgType, "[fmi2LoggerCallback]", IBK::VL_INFO);
}

#if defined(_MSC_VER)

fmiCallbackFunctions Slave::m_fmiCallBackFunctions = {
	{ fmiLoggerCallback }, { calloc }, { free }, {NULL}
};

fmi2CallbackFunctions Slave::m_fmi2CallBackFunctions = {
	{ fmi2LoggerCallback }, { calloc }, { free }, {NULL}, {NULL}
};

#else

fmiCallbackFunctions Slave::m_fmiCallBackFunctions = {
	.logger					= fmiLoggerCallback,
	.allocateMemory			= calloc,
	.freeMemory				= free,
	.stepFinished			= NULL,
};

fmi2CallbackFunctions Slave::m_fmi2CallBackFunctions = {
	.logger					= fmi2LoggerCallback,
	.allocateMemory			= calloc,
	.freeMemory				= free,
	.stepFinished			= NULL,
	.componentEnvironment	= NULL
};

#endif

bool Slave::m_useDebugLogging = true;

Slave::Slave(FMU * fmu, const std::string & name) :
	m_name(name),
	m_fmu(fmu),
	m_t(0),
	m_component(NULL)
{
}


Slave::~Slave() {
	if (m_component != NULL) {
		if (m_fmu->m_modelDescription.m_fmuType & ModelDescription::CS_v1)
			m_fmu->m_fmi1Functions.freeSlaveInstance(m_component);
		else
			m_fmu->m_fmi2Functions.freeInstance(m_component);
	}
}


void Slave::instantiateSlave() {
	if (m_fmu->m_modelDescription.m_fmuType & ModelDescription::CS_v1) {
		m_component = m_fmu->m_fmi1Functions.instantiateSlave(
							m_name.c_str(),
							m_fmu->m_modelDescription.m_guid.c_str(),
							m_fmu->resourcePath(),
							"application/x-mastersim",
							0, // timeout
							fmiFalse, // visible
							fmiFalse, // interactive
							m_fmiCallBackFunctions,
							m_useDebugLogging ? fmiTrue : fmiFalse); // debug logging
	}
	else {
		m_component = m_fmu->m_fmi2Functions.instantiate(
							m_name.c_str(),
							fmi2CoSimulation,
							m_fmu->m_modelDescription.m_guid.c_str(),
							m_fmu->resourcePath(),
							&m_fmi2CallBackFunctions,
							fmi2False,  // not visible
							m_useDebugLogging ? fmi2True : fmi2False); // debug logging
	}

	if (m_component == NULL)
		throw IBK::Exception("Error instantiating slave.", "[Slave::instantiateSlave]");

	// resize vectors
	m_boolOutputs.resize(m_fmu->m_boolValueRefsOutput.size());
	m_intOutputs.resize(m_fmu->m_intValueRefsOutput.size());
	m_doubleOutputs.resize(m_fmu->m_doubleValueRefsOutput.size());
	m_stringOutputs.resize(m_fmu->m_stringValueRefsOutput.size());
}


void Slave::setupExperiment(double relTol, double tStart, double tEnd) {
	if (m_fmu->m_modelDescription.m_fmuType & ModelDescription::CS_v1) {
		// fmi 1 code
	}
	else {
		// fmi 2 code
		if (m_fmu->m_fmi2Functions.setupExperiment(m_component, fmi2True, relTol, tStart, fmi2True, tEnd) != fmi2OK)
			throw IBK::Exception( IBK::FormatString("Error while setting up simulation in slave '%1'.").arg(m_name), "[Slave::setupExperiment]");
	}

}


void Slave::enterInitializationMode() {
	if (m_fmu->m_modelDescription.m_fmuType & ModelDescription::CS_v2) {
		if (m_fmu->m_fmi2Functions.enterInitializationMode(m_component) != fmi2OK)
			throw IBK::Exception( IBK::FormatString("Error while entering initialization mode in slave '%1'.").arg(m_name), "[Slave::enterInitializationMode]");
	}
}


void Slave::exitInitializationMode() {
	if (m_fmu->m_modelDescription.m_fmuType & ModelDescription::CS_v2) {
		if (m_fmu->m_fmi2Functions.exitInitializationMode(m_component) != fmi2OK)
			throw IBK::Exception( IBK::FormatString("Error while leaving initialization mode in slave '%1'.").arg(m_name), "[Slave::exitInitializationMode]");
	}
}


int Slave::doStep(double stepSize, bool noSetFMUStatePriorToCurrentPoint) {
//	const char * const FUNC_ID = "[Slave::doStep]";
	if (m_fmu->m_modelDescription.m_fmuType & ModelDescription::CS_v1) {
		return fmi2OK;
	}
	else {
		fmi2Status res = m_fmu->m_fmi2Functions.doStep(m_component, m_t, stepSize,
													   noSetFMUStatePriorToCurrentPoint ? fmi2True : fmi2False);

		// if integration was successful, updated cached output quantities
		if (res == fmi2OK) {
			cacheOutputs();
		}
		return res;
	}
}


void Slave::currentState(fmi2FMUstate * state) const {
	IBK_ASSERT(m_fmu->m_modelDescription.m_fmuType & ModelDescription::CS_v2);

	if (m_fmu->m_fmi2Functions.getFMUstate(m_component, state) != fmi2OK) {
		throw IBK::Exception(IBK::FormatString("Failed getting FMU state from slave '%1'.").arg(m_name), "[Slave::currentState]");
	}
}


void Slave::setState(fmi2FMUstate slaveState) {
	if (m_fmu->m_fmi2Functions.setFMUstate(m_component, slaveState) != fmi2OK) {
		throw IBK::Exception(IBK::FormatString("Failed setting FMU state in slave '%1'.").arg(m_name), "[Slave::setState]");
	}
}


void Slave::cacheOutputs() {
	const char * const FUNC_ID = "[Slave::cacheOutputs]";
	int res;
	if (m_fmu->m_modelDescription.m_fmuType & ModelDescription::CS_v1) {
		// booleans must be converted one-by-one for FMI 1
		for (unsigned int i=0; i<m_fmu->m_boolValueRefsOutput.size(); ++i) {
			fmiBoolean boolValue;
			res = m_fmu->m_fmi1Functions.getBoolean(m_component, &m_fmu->m_boolValueRefsOutput[i], 1, &boolValue);
			m_boolOutputs[i] = (boolValue == fmiTrue);
		}
		if (!m_fmu->m_intValueRefsOutput.empty()) {
			res = m_fmu->m_fmi1Functions.getInteger(m_component, &m_fmu->m_intValueRefsOutput[0],
					m_fmu->m_intValueRefsOutput.size(), &m_intOutputs[0]);
		}
		if (!m_fmu->m_doubleValueRefsOutput.empty()) {
			res = m_fmu->m_fmi1Functions.getReal(m_component, &m_fmu->m_doubleValueRefsOutput[0],
					m_fmu->m_doubleValueRefsOutput.size(), &m_doubleOutputs[0]);
		}
		// strings are queried one-by-one
		for (unsigned int i=0; i<m_fmu->m_stringValueRefsOutput.size(); ++i) {
			const char * str;
			res = m_fmu->m_fmi1Functions.getString(m_component, &m_fmu->m_stringValueRefsOutput[i], 1, &str);
			if (res != fmi2OK) break;
			IBK_ASSERT(str != NULL);
			m_stringOutputs[i] = std::string(str);
		}
	}
	else {
		if (!m_fmu->m_boolValueRefsOutput.empty()) {
			res = m_fmu->m_fmi2Functions.getBoolean(m_component, &m_fmu->m_boolValueRefsOutput[0],
					m_fmu->m_boolValueRefsOutput.size(), &m_boolOutputs[0]);
		}
		if (!m_fmu->m_intValueRefsOutput.empty()) {
			res = m_fmu->m_fmi2Functions.getInteger(m_component, &m_fmu->m_intValueRefsOutput[0],
					m_fmu->m_intValueRefsOutput.size(), &m_intOutputs[0]);
		}
		if (!m_fmu->m_doubleValueRefsOutput.empty()) {
			res = m_fmu->m_fmi2Functions.getReal(m_component, &m_fmu->m_doubleValueRefsOutput[0],
					m_fmu->m_doubleValueRefsOutput.size(), &m_doubleOutputs[0]);
		}
		// strings are queried one-by-one
		for (unsigned int i=0; i<m_fmu->m_stringValueRefsOutput.size(); ++i) {
			const char * str;
			res = m_fmu->m_fmi2Functions.getString(m_component, &m_fmu->m_stringValueRefsOutput[i], 1, &str);
			if (res != fmi2OK) break;
			IBK_ASSERT(str != NULL);
			m_stringOutputs[i] = std::string(str);
		}
	}
	if (res != fmi2OK)	throw IBK::Exception("Error retrieving values from slave.", FUNC_ID);
}


void Slave::setReal(unsigned int valueReference, double value) {
	int res;
	if (m_fmu->m_modelDescription.m_fmuType & ModelDescription::CS_v1) {
		res = m_fmu->m_fmi1Functions.setReal(m_component, &valueReference, 1, &value);
	}
	else {
		res = m_fmu->m_fmi2Functions.setReal(m_component, &valueReference, 1, &value);
	}
	if (res != fmi2OK) {
		throw IBK::Exception("Error setting input variable.", "[Slave::setReal]");
	}
}


void Slave::setInteger(unsigned int valueReference, int value) {
	int res;
	if (m_fmu->m_modelDescription.m_fmuType & ModelDescription::CS_v1) {
		res = m_fmu->m_fmi1Functions.setInteger(m_component, &valueReference, 1, &value);
	}
	else {
		res = m_fmu->m_fmi2Functions.setInteger(m_component, &valueReference, 1, &value);
	}
	if (res != fmi2OK) {
		throw IBK::Exception("Error setting input variable.", "[Slave::setInteger]");
	}
}


void Slave::setBoolean(unsigned int valueReference, fmi2Boolean value) {
	int res;
	if (m_fmu->m_modelDescription.m_fmuType & ModelDescription::CS_v1) {
		fmiBoolean val = (value == fmi2True) ? fmiTrue : fmiFalse;
		res = m_fmu->m_fmi1Functions.setBoolean(m_component, &valueReference, 1, &val);
	}
	else {
		res = m_fmu->m_fmi2Functions.setBoolean(m_component, &valueReference, 1, &value);
	}
	if (res != fmi2OK) {
		throw IBK::Exception("Error setting input variable.", "[Slave::setBoolean]");
	}
}


void Slave::setString(unsigned int valueReference, const std::string & str) {
	int res;
	const char * const cstr = str.c_str();
	if (m_fmu->m_modelDescription.m_fmuType & ModelDescription::CS_v1) {
		res = m_fmu->m_fmi1Functions.setString(m_component, &valueReference, 1, &cstr);
	}
	else {
		res = m_fmu->m_fmi2Functions.setString(m_component, &valueReference, 1, &cstr);
	}
	if (res != fmi2OK) {
		throw IBK::Exception("Error setting input variable.", "[Slave::setString]");
	}
}


void Slave::setValue(const FMIVariable & var, const std::string & value) {
	// convert value into type
	switch (var.m_type) {
		case FMIVariable::VT_BOOL :
			if (value == "true")	setBoolean(var.m_valueReference, fmi2True);
			else					setBoolean(var.m_valueReference, fmi2False);
			break;
		case FMIVariable::VT_INT :
			setInteger(var.m_valueReference, IBK::string2val<int>(value));
			break;
		case FMIVariable::VT_DOUBLE :
			setReal(var.m_valueReference, IBK::string2val<double>(value));
			break;
		case FMIVariable::VT_STRING :
			setString(var.m_valueReference, value);
			break;
	}
}


} // namespace MASTER_SIM
