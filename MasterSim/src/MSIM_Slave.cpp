#include "MSIM_Slave.h"

#include <IBK_Exception.h>
#include <IBK_assert.h>
#include <IBK_messages.h>

#include <cstdlib>
#include <cstdarg>

#include "MSIM_FMU.h"

namespace MASTER_SIM {

void fmiLoggerCallback( fmiComponent c, fmiString instanceName, fmiStatus status,
						fmiString category, fmiString message, ... )
{
	/// \todo use vsprintf to forward message into string, the feed into message handler
}

void fmi2LoggerCallback( fmi2ComponentEnvironment c, fmi2String instanceName, fmi2Status status,
							  fmi2String category, fmi2String message, ... )
{
	IBK::msg_type_t msgType = IBK::MSG_PROGRESS;
	switch (status) {
		case fmi2Warning	: msgType = IBK::MSG_WARNING; break;
		case fmi2Error		: msgType = IBK::MSG_ERROR; break;
		default :;
	}

	/// \todo use vsprintf to forward message into string, the feed into message handler
	static char buffer[5000];
	va_list args;
	va_start (args, message);
	std::vsnprintf(buffer, 5000, message, args);
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


Slave::Slave(FMU * fmu, const std::string & name) :
	m_name(name),
	m_fmu(fmu),
	m_t(0),
	m_component(NULL)
{
}


Slave::~Slave() {
	if (m_component != NULL)
		m_fmu->m_fmi2Functions.freeInstance(m_component);
}


void Slave::instantiateSlave() {
	if (m_fmu->m_modelDescription.m_fmuType & ModelDescription::CS_v1) {
		m_component = m_fmu->m_fmi1Functions.instantiateSlave(m_name.c_str(),
												m_fmu->m_modelDescription.m_guid.c_str(),
												m_fmu->resourcePath(),
												"application/x-mastersim",
												0, // timeout
												fmiFalse, // visible
												fmiFalse, // interactive
												m_fmiCallBackFunctions,
												fmiFalse); // no debug logging for now
	}
	else {
		m_component = m_fmu->m_fmi2Functions.instantiate(m_name.c_str(),
										   fmi2CoSimulation,
										   m_fmu->m_modelDescription.m_guid.c_str(),
										   m_fmu->resourcePath(),
										   &m_fmi2CallBackFunctions,
										   fmi2False,  // not visible
										   fmi2False); // no debug logging for now
	}

	if (m_component == NULL)
		throw IBK::Exception("Error instantiating slave.", "[Slave::instantiateSlave]");

	// resize vectors
	m_boolOutputs.resize(m_fmu->m_boolValueRefsOutput.size());
	m_intOutputs.resize(m_fmu->m_intValueRefsOutput.size());
	m_doubleOutputs.resize(m_fmu->m_doubleValueRefsOutput.size());
	m_stringOutputs.resize(m_fmu->m_stringValueRefsOutput.size());
}


int Slave::doStep(double tEnd, bool noSetFMUStatePriorToCurrentPoint) {
//	const char * const FUNC_ID = "[Slave::doStep]";
	if (m_fmu->m_modelDescription.m_fmuType & ModelDescription::CS_v1) {
		return fmi2OK;
	}
	else {
		fmi2Status res = m_fmu->m_fmi2Functions.doStep(m_component, m_t, tEnd,
													   noSetFMUStatePriorToCurrentPoint ? fmi2True : fmi2False);

		// if integration was successful, updated cached output quantities
		if (res == fmi2OK) {
			cacheOutputs();
		}
		return res;
	}
}


void * Slave::currentState() const {
	return NULL;
}


void Slave::setState(void * slaveState) {

}


void Slave::cacheOutputs() {
	const char * const FUNC_ID = "[Slave::cacheOutputs]";
	if (!m_fmu->m_boolValueRefsOutput.empty()) {
		fmi2Status r = m_fmu->m_fmi2Functions.getBoolean(m_component, &m_fmu->m_boolValueRefsOutput[0],
				m_fmu->m_boolValueRefsOutput.size(), &m_boolOutputs[0]);
		if (r != fmi2OK)	throw IBK::Exception("Error retrieving values from slave.", FUNC_ID);
	}
	if (!m_fmu->m_intValueRefsOutput.empty()) {
		fmi2Status r = m_fmu->m_fmi2Functions.getInteger(m_component, &m_fmu->m_intValueRefsOutput[0],
				m_fmu->m_intValueRefsOutput.size(), &m_intOutputs[0]);
		if (r != fmi2OK)	throw IBK::Exception("Error retrieving values from slave.", FUNC_ID);
	}
	if (!m_fmu->m_doubleValueRefsOutput.empty()) {
		fmi2Status r = m_fmu->m_fmi2Functions.getReal(m_component, &m_fmu->m_doubleValueRefsOutput[0],
				m_fmu->m_doubleValueRefsOutput.size(), &m_doubleOutputs[0]);
		if (r != fmi2OK)	throw IBK::Exception("Error retrieving values from slave.", FUNC_ID);
	}
	// strings are queried one-by-one
	for (unsigned int i=0; i<m_fmu->m_stringValueRefsOutput.size(); ++i) {
		const char * str;
		fmi2Status r = m_fmu->m_fmi2Functions.getString(m_component, &m_fmu->m_stringValueRefsOutput[i], 1, &str);
		if (r != fmi2OK)	throw IBK::Exception("Error retrieving values from slave.", FUNC_ID);
		IBK_ASSERT(str != NULL);
		m_stringOutputs[i] = std::string(str);
	}
}

} // namespace MASTER_SIM
