#include "MSIM_Slave.h"

#include "MSIM_FMU.h"

namespace MASTER_SIM {

void fmi2LoggerCallback( fmi2ComponentEnvironment c, fmi2String instanceName, fmi2Status status,
							  fmi2String category, fmi2String message, ... )
{
	/// \todo use vsprintf to forward message into string, the feed into message handler
}

//void stepFinishedCallBack( fmi2ComponentEnvironment c, fmi2Status status ) {
//	Slave * s = reinterpret_cast<Slave*>(c);
//	s->stepCompleted(status);
//}


fmi2CallbackFunctions Slave::m_fmi2CallBackFunctions = {
	.logger = fmi2LoggerCallback,
	.allocateMemory = calloc,
	.freeMemory = free,
	.stepFinished = NULL,
	.componentEnvironment = NULL
};




Slave::Slave(FMU * fmu, const std::string & name) :
	m_fmu(fmu),
	m_name(name),
	m_component(NULL)
{
}


Slave::~Slave() {
	if (m_component != NULL)
		m_fmu->m_fmi2Functions.freeInstance(m_component);
}


void Slave::instantiateSlave() {
	m_fmu->m_fmi2Functions.instantiate(m_name.c_str(),
									   fmi2CoSimulation,
									   m_fmu->m_modelDescription.m_guid.c_str(),
									   m_fmu->resourcePath(),
									   &m_fmi2CallBackFunctions,
									   fmi2False,  // not visible
									   fmi2False); // no debug logging for now

}



//void Slave::stepCompleted(fmi2Status status) {

//}

} // namespace MASTER_SIM
