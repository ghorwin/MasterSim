#include "MSIM_AlgorithmGaussSeidel.h"

#include <IBK_assert.h>
#include <IBK_messages.h>

#include "MSIM_MasterSim.h"

namespace MASTER_SIM {

AlgorithmGaussSeidel::Result AlgorithmGaussSeidel::doStep() {
	const char * const FUNC_ID = "[AlgorithmGaussSeidel::doStep]";

	// to make things simpler, let's just use fmi2status variables
	IBK_STATIC_ASSERT((int)fmiOK == (int)fmi2OK);

	// master and FMUs are expected to be at current time point t
	double t = m_master->m_tCurrent;

	// all slave output variables are expected to be in sync with internal states of slaves
	// i.e. cacheOutputs() has been called successfully on all slaves

	// global variable array is expected to be in sync with all slaves


	// ** algorithm start **

	// create a copy of variable array to updated variable array, since we will use this for input in Gauss-Seidel
	MasterSim::copyVector(m_master->m_realyt, m_master->m_realytNext);
	MasterSim::copyVector(m_master->m_intyt, m_master->m_intytNext);
	MasterSim::copyVector(m_master->m_boolyt, m_master->m_boolytNext);
	std::copy(m_master->m_stringyt.begin(), m_master->m_stringyt.end(), m_master->m_stringytNext.begin());

	// init for iteration, request state from all slaves unless time step adjustment is used, in which
	// case states have already been stored
	if (m_master->m_enableIteration && (m_master->m_project.m_errorControlMode != Project::EM_ADAPT_STEP)) {
		m_master->storeCurrentSlaveStates(m_master->m_iterationStates);
	}


	// loop over all cycles
	for (unsigned int c=0; c<m_master->m_cycles.size(); ++c) {
		const MasterSim::Cycle & cycle = m_master->m_cycles[c];

		unsigned int iteration = 0; // iteration counter in current cycle
		while (++iteration <= m_master->m_project.m_maxIterations) {

			// when iterating
			if (m_master->m_enableIteration) {
				// create copy of current variables
				/// \todo only copy variables that are port of this cycle, may speed up code a 'little' when number
				/// of FMUs and their inputs is very large
				MasterSim::copyVector(m_master->m_realytNext, m_master->m_realytNextIter);
				MasterSim::copyVector(m_master->m_intytNext, m_master->m_intytNextIter);
				MasterSim::copyVector(m_master->m_boolytNext, m_master->m_boolytNextIter);
				std::copy(m_master->m_stringytNext.begin(), m_master->m_stringytNext.end(), m_master->m_stringytNextIter.begin());

				// roll-back all slaves in this cycle, except for first iteration
				if (iteration > 1) {
					for (unsigned int s=0; s<cycle.m_slaves.size(); ++s) {
						Slave * slave = cycle.m_slaves[s];
						m_timer.start();
						slave->setState(t, m_master->m_iterationStates[slave->m_slaveIndex]);
						m_master->m_statRollBackTimes[slave->m_slaveIndex] = 1e-3*m_timer.difference(); // add elapsed time in seconds
						++m_master->m_statRollBackCounters[slave->m_slaveIndex];
					}
				}
			}

			// loop over all slaves
			for (unsigned int s=0; s<cycle.m_slaves.size(); ++s) {
				Slave * slave = cycle.m_slaves[s];

				// update input variables in all slaves, using variables from time t + stepsize (partially updated from previous slaves)
				m_master->updateSlaveInputs(slave, m_master->m_realytNext, m_master->m_intytNext, m_master->m_boolytNext, m_master->m_stringytNext);

				// advance slave
				m_timer.start();
				int res = slave->doStep(m_master->m_tStepSize, true);
				m_master->m_statSlaveEvalTimes[slave->m_slaveIndex] = 1e-3*m_timer.difference(); // add elapsed time in seconds
				++m_master->m_statSlaveEvalCounters[slave->m_slaveIndex];
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

				// slave is now at time level t + stepsize and its outputs are updated accordingly
				// sync results into vector with newly computed quantities, so that next slave will use newly
				// computed values already
				m_master->syncSlaveOutputs(slave, m_master->m_realytNext, m_master->m_intytNext, m_master->m_boolytNext, m_master->m_stringytNext);
			}

			// when iterating, do convergence test
			if (m_master->m_enableIteration) {
				// no need for convergence test if only one slave in this cycle
				if (cycle.m_slaves.size() == 1)
					break;

				// stability measure: if time step falls below a certain threshhold, we fall back to non-iterating
				// gauss seidel
				if (m_master->m_tStepSize < m_master->m_project.m_tStepSizeFallBackLimit) {
					IBK::IBK_Message(IBK::FormatString("t = %1, dt = %2 < %3 (limit), skipping iteration\n").arg(t).arg(m_master->m_tStepSize).arg(m_master->m_project.m_tStepSizeFallBackLimit), 
						IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DEVELOPER);
					break; // no more iterating
				}

				// convergence test is based on difference between
				if (m_master->doConvergenceTest())
					break; // break iteration loop
			}
			IBK::IBK_Message(IBK::FormatString("t = %1, dt = %2, Cycle #%3, Iteration #%4\n").arg(t).arg(m_master->m_tStepSize).arg(c).arg(iteration), 
				IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DEVELOPER);
		}
		if (m_master->m_enableIteration &&
			iteration > m_master->m_project.m_maxIterations)
			return R_ITERATION_LIMIT_EXCEEDED;
	}

	// ** algorithm end **

	// m_XXXyt     -> still values at time point t
	// m_XXXytNext -> values at time point t + tStepSize
	return R_CONVERGED;
}

} // namespace MASTER_SIM
