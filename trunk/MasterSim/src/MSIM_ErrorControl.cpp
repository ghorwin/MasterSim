#include "MSIM_ErrorControl.h"

#include "MSIM_MasterSim.h"

namespace MASTER_SIM {

ErrorControl::ErrorControl(MasterSim* master) :
	m_master(master)
{
}


bool ErrorControl::checkLocalError() {

	// when this function is called, we excpect a converged solution
	//
	// - m_h -> holds size of current step
	// - m_realyt -> holds old states at time t
	// - m_realytNext -> holds states at t + h computed by master algorithm
	//
	//   (same for all other data types)

	// we now copy the real vectors to our temporary location
	m_hOriginal = m_master->m_t;
	if (m_realyt.size() != m_master->m_realyt.size()) {
		// resize required memory on first use
		unsigned int nValues = m_master->m_realyt.size();
		m_realyt.resize(nValues);
		m_realytFirst.resize(nValues);
	}
	MasterSim::copyVector(m_master->m_realyt, m_realyt);
	MasterSim::copyVector(m_master->m_realytNext, m_realytFirst);

	// we will now reset the state of all slaves to be back at time t
//	for (unsigned int s=0; s<m_master->m_slaves)


	return true;
}

} // namespace MASTER_SIM
