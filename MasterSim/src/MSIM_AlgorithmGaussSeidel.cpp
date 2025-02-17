#include "MSIM_AlgorithmGaussSeidel.h"

#include <cmath>

#include <IBK_assert.h>
#include <IBK_messages.h>

#include "MSIM_MasterSim.h"

namespace MASTER_SIM {

AlgorithmGaussSeidel::Result AlgorithmGaussSeidel::doStep() {
	const char * const FUNC_ID = "[AlgorithmGaussSeidel::doStep]";

	// to make things simpler, let's just use fmi2status variables
	IBK_STATIC_ASSERT((int)fmiOK == (int)fmi2OK);

	// master and FMUs are expected to be at current time point t
	double t = m_master->m_t;

	// all slave output variables are expected to be in sync with internal states of slaves
	// i.e. cacheOutputs() has been called successfully on all slaves

	// global variable array is expected to be in sync with all slaves


	// ** algorithm start **

	// create a copy of variable array to updated variable array, since we will use this for input in Gauss-Seidel
	MasterSim::copyVector(m_master->m_realyt, m_master->m_realytNext);
	MasterSim::copyVector(m_master->m_intyt, m_master->m_intytNext);
	MasterSim::copyVector(m_master->m_boolyt, m_master->m_boolytNext);
	std::copy(m_master->m_stringyt.begin(), m_master->m_stringyt.end(), m_master->m_stringytNext.begin());

	// loop over all cycles
	for (unsigned int c=0; c<m_master->m_cycles.size(); ++c) {
		const MasterSim::Cycle & cycle = m_master->m_cycles[c];

		unsigned int iteration = 0; // iteration counter in current cycle
		while (++iteration <= m_master->m_project.m_maxIterations) {
			++m_nIterations;

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
						AbstractSlave * slave = cycle.m_slaves[s];
						m_timer.start();
						slave->setState(t, m_master->m_iterationStates[slave->m_slaveIndex]);
						m_master->m_statRollBackTimes[slave->m_slaveIndex] += 1e-3*m_timer.stop(); // add elapsed time in seconds
						++m_master->m_statRollBackCounters[slave->m_slaveIndex];
					}
				}
			}

			// loop over all slaves
			for (unsigned int s=0; s<cycle.m_slaves.size(); ++s) {
				AbstractSlave * slave = cycle.m_slaves[s];

				// update input variables in all slaves, using variables from time t + stepsize (partially updated from previous slaves)
				m_master->updateSlaveInputs(slave, m_master->m_realytNext, m_master->m_intytNext, m_master->m_boolytNext, m_master->m_stringytNext, false);

				// advance slave
				m_timer.start();
				int res = slave->doStep(m_master->m_h, true);
				m_master->m_statSlaveEvalTimes[slave->m_slaveIndex] += 1e-3*m_timer.stop(); // add elapsed time in seconds
				++m_master->m_statSlaveEvalCounters[slave->m_slaveIndex];
				switch (res) {
					case fmi2Discard	:
					case fmi2Error		: {
						++m_nFMUErrors;
						return R_RETRY;
					}
					case fmi2Pending	:
						throw IBK::Exception("Asynchronous slaves are not supported, yet.", FUNC_ID);
					case fmi2Fatal		:
						throw IBK::Exception(IBK::FormatString("Error in doStep() call of FMU slave '%1'").arg(slave->m_name), FUNC_ID);
					case fmi2OK			:
					default				:
						break;
				}

				// slave is now at time level t + stepsize and its outputs are updated accordingly
				// sync results into vector with newly computed quantities, so that next slave will use newly
				// computed values already
				m_master->syncSlaveOutputs(slave, m_master->m_realytNext, m_master->m_intytNext, m_master->m_boolytNext, m_master->m_stringytNext, false);
			}

			// when iterating, do convergence test
			if (m_master->m_enableIteration) {
				// no need for convergence test if only one slave in this cycle
				if (cycle.m_slaves.size() == 1)
					break;

				// stability measure: if time step falls below a certain threshhold, we fall back to non-iterating
				// gauss seidel
				if (m_master->m_h < m_master->m_project.m_hFallBackLimit.value) {
					IBK_FastMessage(IBK::VL_DETAILED)(IBK::FormatString("  GAUSS-SEIDEL: t = %1, dt = %2 < %3 (limit), skipping iteration\n").arg(t).arg(m_master->m_h).arg(m_master->m_project.m_hFallBackLimit.value),
						IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DETAILED);
					break; // no more iterating
				}

				IBK_FastMessage(IBK::VL_DETAILED)(IBK::FormatString("  GAUSS-SEIDEL: t = %1, dt = %2, Cycle #%3, Iteration #%4\n").arg(t).arg(m_master->m_h).arg(c).arg(iteration),
					IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DETAILED);

				// convergence test is based on difference between
				if (doConvergenceTest())
					break; // break iteration loop
			}
		}
		// When m_master->m_project.m_maxIterations == 1 the flag m_master->m_enableIteration is false, so our solution is always
		// assumed to be converged.
		if (m_master->m_enableIteration && iteration > m_master->m_project.m_maxIterations) {
			++m_nIterationLimitExceeded;
			return R_ITERATION_LIMIT_EXCEEDED;
		}
	} // cycle loop

	// ** algorithm end **

	// m_XXXyt     -> still values at time point t
	// m_XXXytNext -> values at time point t + h
	return R_CONVERGED;
}


bool AlgorithmGaussSeidel::doConvergenceTest() {
	const char * const FUNC_ID = "[AlgorithmGaussSeidel::doConvergenceTest]";

	// compare all state-based values: int, bool and string
	for (unsigned int i=0; i<m_master->m_intytNextIter.size(); ++i) {
		if (m_master->m_intytNext[i] != m_master->m_intytNextIter[i])
			return false;
	}

	for (unsigned int i=0; i<m_master->m_boolytNextIter.size(); ++i) {
		if (m_master->m_boolytNext[i] != m_master->m_boolytNextIter[i])
			return false;
	}

	for (unsigned int i=0; i<m_master->m_stringytNextIter.size(); ++i) {
		if (m_master->m_stringytNext[i] != m_master->m_stringytNextIter[i])
			return false;
	}

	// WRMS norm of real values
	double norm = 0;
	size_t nValues = m_master->m_realyt.size();
	for (size_t i=0; i<nValues; ++i) {
		double diff = m_master->m_realytNextIter[i] - m_master->m_realytNext[i];
		double absValue = std::fabs(m_master->m_realytNextIter[i]);
		double weight = absValue*m_master->m_project.m_relTol + m_master->m_project.m_absTol;
		diff /= weight;
		norm += diff*diff;
	}

	norm = std::sqrt(norm/nValues);
	IBK_FastMessage(IBK::VL_DETAILED)(IBK::FormatString("  GAUSS-SEIDEL: WRMS norm = %1\n").arg(norm, 12, 'f', 0), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DETAILED);
	if (norm > 1) {
		return false;
	}


	return true;
}


} // namespace MASTER_SIM
