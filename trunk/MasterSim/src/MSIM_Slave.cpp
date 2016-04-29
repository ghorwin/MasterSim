#include "MSIM_Slave.h"

#include "MSIM_FMU.h"

namespace MASTER_SIM {

Slave::Slave(FMU * fmu, const std::string & name) :
	m_fmu(fmu),
	m_name(name),
	m_component(NULL)
{
}

void Slave::instantiateSlave() {

}

Slave::~Slave() {
	if (m_component != NULL)
		m_fmu->m_fmi2Functions.freeInstance(m_component);
}

} // namespace MASTER_SIM
