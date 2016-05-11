#include "MSIM_AlgorithmGaussJacobi.h"

#include <IBK_assert.h>

#include "MSIM_MasterSim.h"

namespace MASTER_SIM {

AbstractAlgorithm::Result AlgorithmGaussJacobi::doStep() {
	const char * const FUNC_ID = "[AlgorithmGaussJacobi::doStep]";

	// to make things simpler, let's just use fmi2status variables
	IBK_STATIC_ASSERT((int)fmiOK == (int)fmi2OK);

	// master and FMUs are expected to be at current time point t

	// all slave output variables are expected to be in sync with internal states of slaves
	// i.e. cacheOutputs() has been called successfully on all slaves

	// global variable array is expected to be in sync with all slaves


	// ** algorithm start **

	// loop over all cycles
	for (unsigned int c=0; c<m_master->m_cycles.size(); ++c) {
		const MasterSim::Cycle & cycle = m_master->m_cycles[c];

		// loop over all slaves
		for (unsigned int s=0; s<cycle.m_slaves.size(); ++s) {
			Slave * slave = cycle.m_slaves[s];

			// update input variables in all slaves, using variables from time t
			m_master->updateSlaveInputs(slave, m_master->m_realyt, m_master->m_intyt, m_master->m_boolyt, m_master->m_stringyt);

			// advance slave
			m_timer.start();
			int res = slave->doStep(m_master->m_tStepSize, true);
			m_master->m_statSlaveEvalTimes[slave->m_slaveIndex] = 1e-3*m_timer.difference(); // add elapsed time in seconds
			if (res != fmi2OK)
				throw IBK::Exception(IBK::FormatString("Error in doStep() call of FMU slave '%1'").arg(slave->m_name), FUNC_ID);


			// slave is now at time level t + stepsize and its outputs are updated accordingly
			// sync results into vector with newly computed quantities
			m_master->syncSlaveOutputs(slave, m_master->m_realytNext, m_master->m_intytNext, m_master->m_boolytNext, m_master->m_stringytNext);
		}
	}

	// ** algorithm end **


	// m_XXXyt     -> still values at time point t
	// m_XXXytNext -> values at time point t + tStepSize
	return R_CONVERGED; // no other option since we don't iterate
}


} // namespace MASTER_SIM
