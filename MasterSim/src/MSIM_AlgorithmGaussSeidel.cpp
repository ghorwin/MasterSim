#include "MSIM_AlgorithmGaussSeidel.h"

#include <IBK_assert.h>

#include "MSIM_MasterSim.h"

namespace MASTER_SIM {

AlgorithmGaussSeidel::Result AlgorithmGaussSeidel::doStep() {
	const char * const FUNC_ID = "[AlgorithmGaussSeidel::doStep]";

	// to make things simpler, that's just use fmi2status variables
	IBK_STATIC_ASSERT((int)fmiOK == (int)fmi2OK);

	// master and FMUs are expected to be at time t
	double t = m_master->m_tCurrent;
	// and we integrate to tNext
	double tNext = t + m_master->m_tStepSize;

	// all slave output variables are expected to be in sync with internal states of slaves
	// i.e. cacheOutputs() has been called successfully on all slaves

	// global variable array is expected to be in sync with all slaves

	// ** algorithm start **

	// create a copy of variable array to iterative variable array
	MasterSim::copyVector(m_master->m_yt, m_master->m_ytNext);

	// when iterating, request state from all slaves
	if (m_master->m_project.m_maxIterations > 1) {
		m_master->storeCurrentSlaveStates(m_master->m_iterationStates);
	}


	// loop over all cycles
	for (unsigned int c=0; c<m_master->m_cycles.size(); ++c) {
		const MasterSim::Cycle & cycle = m_master->m_cycles[c];

		unsigned int iteration = 0; // iteration counter in current cycle
		while (++iteration <= m_master->m_project.m_maxIterations) {

			// when iterating
			if (m_master->m_project.m_maxIterations > 1) {
				// create copy of current variables
				/// \todo only copy variables that are port of this cycle, may speed up code a 'little'
				MasterSim::copyVector(m_master->m_ytNext, m_master->m_ytNextIter);

				// roll-back all slaves in this cycle, except for first iteration
				if (iteration > 1) {
					for (unsigned int s=0; s<cycle.m_slaves.size(); ++s) {
						Slave * slave = cycle.m_slaves[s];
						slave->setState(m_master->m_iterationStates[slave->m_slaveIndex]);
					}
				}
			}

			// loop over all slaves
			for (unsigned int s=0; s<cycle.m_slaves.size(); ++s) {
				Slave * slave = cycle.m_slaves[s];

				// update input variables in all slaves, using variables from time t
				m_master->updateSlaveInputs(slave, m_master->m_ytNext);

				// advance slave, we have no roll-back
				int res = slave->doStep(tNext, true);
				switch (res) {
					case fmi2Discard	:
					case fmi2Error		:
						return R_RETRY;
					case fmi2Pending	: throw IBK::Exception("Asynchronous slaves are not supported, yet.", FUNC_ID);
					case fmi2Fatal		:
						throw IBK::Exception(IBK::FormatString("Error in doStep() call of FMU slave '%1'").arg(slave->m_name), FUNC_ID);
					case fmi2OK			:
					default				:
						break;
				}

				// slave is now at time level tNext and its outputs are updated accordingly
				// sync results into vector with newly computed quantities, so that next slave will use newly
				// computed values already
				m_master->syncSlaveOutputs(slave, m_master->m_ytNext);
			}

			// when iterating, do convergence test
			if (m_master->m_project.m_maxIterations > 1) {
				if (m_master->doConvergenceTest())
					break; // break iteration loop
			}
		}
		if (m_master->m_project.m_maxIterations != 1 &&
			iteration > m_master->m_project.m_maxIterations)
			return R_ITERATION_LIMIT_EXCEEDED;
	}

	// ** algorithm end **

	// m_yt     -> still values at time point t
	// m_ytNext -> values at time point tNext
	return R_CONVERGED;
}

} // namespace MASTER_SIM
