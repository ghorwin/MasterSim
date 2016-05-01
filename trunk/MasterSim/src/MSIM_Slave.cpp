#include "MSIM_Slave.h"

#include <IBK_Exception.h>
#include <IBK_assert.h>

#include "MSIM_FMU.h"

namespace MASTER_SIM {

void fmi2LoggerCallback( fmi2ComponentEnvironment c, fmi2String instanceName, fmi2Status status,
							  fmi2String category, fmi2String message, ... )
{
	/// \todo use vsprintf to forward message into string, the feed into message handler
}


fmi2CallbackFunctions Slave::m_fmi2CallBackFunctions = {
	.logger					= fmi2LoggerCallback,
	.allocateMemory			= calloc,
	.freeMemory				= free,
	.stepFinished			= NULL,
	.componentEnvironment	= NULL
};




Slave::Slave(FMU * fmu, const std::string & name) :
	m_fmu(fmu),
	m_name(name),
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
//		m_fmu->m_fmi1Functions.instantiate(m_name.c_str(),
//										   fmiCoSimulation,
//										   m_fmu->m_modelDescription.m_guid.c_str(),
//										   &m_fmi2CallBackFunctions,
//										   fmi2False,  // not visible
//										   fmi2False); // no debug logging for now
	}
	else {
		m_fmu->m_fmi2Functions.instantiate(m_name.c_str(),
										   fmi2CoSimulation,
										   m_fmu->m_modelDescription.m_guid.c_str(),
										   m_fmu->resourcePath(),
										   &m_fmi2CallBackFunctions,
										   fmi2False,  // not visible
										   fmi2False); // no debug logging for now
	}

	// resize vectors
	m_boolOutputs.resize(m_fmu->m_boolValueRefs.size());
	m_intOutputs.resize(m_fmu->m_intValueRefs.size());
	m_doubleOutputs.resize(m_fmu->m_doubleValueRefs.size());
	m_stringOutputs.resize(m_fmu->m_stringValueRefs.size());
}


int Slave::doStep(double tEnd, bool noSetFMUStatePriorToCurrentPoint) {
	const char * const FUNC_ID = "[Slave::doStep]";
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


void Slave::cacheOutputs() {
	const char * const FUNC_ID = "[Slave::cacheOutputs]";
	if (!m_fmu->m_boolValueRefs.empty()) {
		fmi2Status r = m_fmu->m_fmi2Functions.getBoolean(m_component, &m_fmu->m_boolValueRefs[0],
				m_fmu->m_boolValueRefs.size(), &m_boolOutputs[0]);
		if (r != fmi2OK)	throw IBK::Exception("Error retrieving values from slave.", FUNC_ID);
	}
	if (!m_fmu->m_intValueRefs.empty()) {
		fmi2Status r = m_fmu->m_fmi2Functions.getInteger(m_component, &m_fmu->m_intValueRefs[0],
				m_fmu->m_intValueRefs.size(), &m_intOutputs[0]);
		if (r != fmi2OK)	throw IBK::Exception("Error retrieving values from slave.", FUNC_ID);
	}
	if (!m_fmu->m_doubleValueRefs.empty()) {
		fmi2Status r = m_fmu->m_fmi2Functions.getReal(m_component, &m_fmu->m_doubleValueRefs[0],
				m_fmu->m_doubleValueRefs.size(), &m_doubleOutputs[0]);
		if (r != fmi2OK)	throw IBK::Exception("Error retrieving values from slave.", FUNC_ID);
	}
	// strings are queried one-by-one
	for (unsigned int i=0; i<m_fmu->m_stringValueRefs.size(); ++i) {
		const char * str;
		fmi2Status r = m_fmu->m_fmi2Functions.getString(m_component, &m_fmu->m_stringValueRefs[i], 1, &str);
		if (r != fmi2OK)	throw IBK::Exception("Error retrieving values from slave.", FUNC_ID);
		IBK_ASSERT(str != NULL);
		m_stringOutputs[i] = std::string(str);
	}
}

} // namespace MASTER_SIM
