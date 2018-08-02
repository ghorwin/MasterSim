#include "MSIM_AlgorithmNewton.h"

#include <IBK_assert.h>
#include <IBK_messages.h>

#include "MSIM_MasterSim.h"
#include "MSIM_Slave.h"

namespace MASTER_SIM {

void AlgorithmNewton::init() {
	// resize matrix and variable index mapping vectors
	size_t nCycles = m_master->m_cycles.size();
	m_jacobianMatrix.resize(nCycles);
	m_variableIdxMapping.resize(nCycles);
	m_res.resize(m_master->m_realyt.size());
	// Note: typically, rhs will be smaller than res with more than one cycle is used
	//       however, since we need continuous memory mapping we have to copy values anyway and
	//       we can just reuse the array for all cycles - so we make it as big as the possible maximum (all values)
	m_rhs.resize(m_master->m_realyt.size());

	// now process each cycle
	for (size_t c=0; c<nCycles; ++c) {
		const MasterSim::Cycle & cycle = m_master->m_cycles[c];
		// loop over all variables and collect indexes of all variables
		// that are both input and output of the slaves in this cycle
		for (size_t varIdx=0; varIdx<m_master->m_realVariableMapping.size(); ++varIdx) {
			// check if slave is in current cycle
			bool foundInput = false;
			bool foundOutput = false;
			for (unsigned int s=0; s<cycle.m_slaves.size(); ++s) {
				const Slave * slave = cycle.m_slaves[s];
				// mind that an output of a slave may be directly connected to its own input
				// so we can have the same slave being used
				if (m_master->m_realVariableMapping[varIdx].m_inputSlave == slave)
					foundInput = true;
				if (m_master->m_realVariableMapping[varIdx].m_outputSlave == slave)
					foundOutput = true;
			}
			if (foundInput && foundOutput) {
				// remember global variable index
				m_variableIdxMapping[c].push_back((unsigned int)varIdx);
			}
		}

		// finally resize DenseMatrix
		size_t dim = m_variableIdxMapping[c].size();
		if (dim != 0)
			m_jacobianMatrix[c].resize((unsigned int)dim);
		// Note: dim == 0 means there are no outputs of the slaves in the current cycle connected
		//       to any of the inputs. Therefore we do not need to iterate in this cycle and can
		//       just accept the results from the first doStep() calculations.
	}
}


AlgorithmNewton::Result AlgorithmNewton::doStep() {
	const char * const FUNC_ID = "[AlgorithmNewton::doStep]";

	// to make things simpler, let's just use fmi2status variables
	IBK_STATIC_ASSERT((int)fmiOK == (int)fmi2OK);

	// master and FMUs are expected to be at current time point t
	double t = m_master->m_t;

	// all slave output variables are expected to be in sync with internal states of slaves
	// i.e. cacheOutputs() has been called successfully on all slaves

	// global variable array is expected to be in sync with all slaves

	// Storage memory used:
	//   m_master->m_realyt          - y_t
	//   m_master->m_realytNext      - y_{t+h}^i, inputs to slaves
	//   m_master->m_realytNextIter  - y_{t+h}^i, copy of inputs to slaves
	//   m_res                       - Sy = S(y_{t+h}^i), holds function evaluation based on iterative guess

	// residual calculated:               m_rhs = mapping(m_res - m_master->m_realytNext)
	// Newton-backsolving inplace in:     m_rhs - delta_y^{i+1}
	// convergence test based on:         ||delta_y^{i+1}||
	// new solution at end of iteration:  m_master->m_realytNext = m_master->m_realytNextIter + m_res

	// final result:                      m_master->m_realytNext


	// ** algorithm start **

	// create a copy of variable array to updated variable array, since we will use this for input in Newton
	// currently we use constant extrapolation
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

			// copy current estimates to y_{t+h} to NextIter vectors, they will be used to update ytNext vectors once cycle
			// is complete
			MasterSim::copyVector(m_master->m_realytNext, m_master->m_realytNextIter);
			MasterSim::copyVector(m_master->m_intytNext, m_master->m_intytNextIter);
			MasterSim::copyVector(m_master->m_boolytNext, m_master->m_boolytNextIter);
			std::copy(m_master->m_stringytNext.begin(), m_master->m_stringytNext.end(), m_master->m_stringytNextIter.begin());

			if (iteration > 1) {
				// except for the first iteration, roll-back all slaves in this cycle
				for (unsigned int s=0; s<cycle.m_slaves.size(); ++s) {
					Slave * slave = cycle.m_slaves[s];
					slave->setState(t, m_master->m_iterationStates[slave->m_slaveIndex]);
				}
			}

			// loop over all slaves and compute S(y_{t+h}^i)
			for (unsigned int s=0; s<cycle.m_slaves.size(); ++s) {
				Slave * slave = cycle.m_slaves[s];
				Result res = evaluateSlave(slave, m_master->m_realytNext, m_res);
				if (res != R_CONVERGED) {
					++m_nFMUErrors;
					return res;
				}
			}

			// m_res now holds Sy and m_realytNext holds y_{t+h}^i (last iteration step)

			size_t varCount = m_variableIdxMapping[c].size();
			if (varCount == 0) {
				// if we do not have any variables connected in this cycle, we can move on to the next cycle
				break;
			}

			// TODO : We may add here a convergence test already and stop if we are fulfilling
			//        the tolerance criterion.

			// compute difference of all variables connected in current cycle and store in vector m_res
			for (unsigned int i=0; i<varCount; ++i) {
				unsigned int varIdx = m_variableIdxMapping[c][i]; // global index of variable
				m_rhs[i] = m_res[varIdx] - m_master->m_realytNext[varIdx]; // = - (y - Sy)
			}

			if (iteration == 1) {
				// in first iteration generate Jacobian matrix, if dimension is 0, this is a NOOP
				generateJacobian(c);
			}

			// backsolve with Jacobian, we use only the first m_variableIdxMapping[c].size() values of m_res
			m_jacobianMatrix[c].backsolve(&m_rhs[0]);
			// m_rhs contains now delta_y^{i+1}

			// do convergence test with WRMS norm of
			double norm = 0;
			for (unsigned int i=0; i<varCount; ++i) {
				unsigned int varIdx = m_variableIdxMapping[c][i]; // global index of variable
				double delta = m_rhs[i];
				double absValue = std::fabs(m_master->m_realytNextIter[varIdx]);
				double weight = absValue*m_master->m_project.m_relTol + m_master->m_project.m_absTol;
				delta /= weight;
				norm += delta*delta;
				// also update next iterative guess
				m_master->m_realytNext[varIdx] = m_master->m_realytNextIter[varIdx] + m_rhs[i];
			}
			norm = std::sqrt(norm/varCount);
			IBK_FastMessage(IBK::VL_INFO)(IBK::FormatString("  NEWTON: WRMS norm = %1\n").arg(norm, 12, 'f', 0), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
			if (norm < 1)
				break; // break iteration loop
		} // while (++iteration <= m_master->m_project.m_maxIterations)

		if (iteration > m_master->m_project.m_maxIterations) {
			++m_nIterationLimitExceeded;
			return R_ITERATION_LIMIT_EXCEEDED;
		}

		// cycle done, also sync variables of other types with global variable array ytNext so that the results
		// of the current cycle are available in the next cycle
		for (unsigned int s=0; s<cycle.m_slaves.size(); ++s) {
			Slave * slave = cycle.m_slaves[s];
			// Note: when we have converged, m_master->m_realytNext holds corrected solution (with delta added), when
			//       we now sync again with the last state of the slaves, we actually get the last iterative value
			//       stored in the slave. However, the difference should be within the error limit.
			m_master->syncSlaveOutputs(slave, m_master->m_realytNext, m_master->m_intytNext, m_master->m_boolytNext, m_master->m_stringytNext, false);
		}

	} // cycles

	// ** algorithm end **

	// m_XXXyt     -> still values at time point t
	// m_XXXytNext -> values at time point t + h
	return R_CONVERGED;
}


AbstractAlgorithm::Result AlgorithmNewton::evaluateSlave(Slave * slave, const std::vector<double> & xi, std::vector<double> & xi1) {
	const char * const FUNC_ID = "[AlgorithmNewton::evaluateF]";
	// update input variables in all slaves, using variables from time t
	m_master->updateSlaveInputs(slave, xi, m_master->m_intyt, m_master->m_boolyt, m_master->m_stringyt, true);

	// advance slave
	m_timer.start();
	int res = slave->doStep(m_master->m_h, true);
	m_master->m_statSlaveEvalTimes[slave->m_slaveIndex] += 1e-3*m_timer.stop(); // add elapsed time in seconds
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

	// slave is now at time level t + h and its outputs are updated accordingly
	// sync results into vector with newly computed quantities
	m_master->syncSlaveOutputs(slave, xi1, m_master->m_intytNext, m_master->m_boolytNext, m_master->m_stringytNext, true);
	return R_CONVERGED;
}


void AlgorithmNewton::generateJacobian(unsigned int c) {
	const char * const FUNC_ID = "[AlgorithmNewton::generateJacobian]";
	if (m_jacobianMatrix[c].n() == 0)
		return;

	MasterSim::Cycle & cycle = m_master->m_cycles[c];

	// m_realyt holds y_{t} = y_{t+h}^0
	// m_realytNextIter holds also y_{t+h}^0

	MasterSim::copyVector(m_res, m_master->m_realytNext);
	// m_realytNext now holds Sy=S(y_{t+h}^0)

	// We now modify values of m_realyt, evaluate slaves, compute DQ approximations and
	// restore values in m_realyt

	// loop all variables in this cycle
	size_t varCount = m_variableIdxMapping[c].size();
	for (unsigned int i=0; i<(unsigned int)varCount; ++i) {
		unsigned int varIdx = m_variableIdxMapping[c][i]; // global index of variable
		// modify variable
		double delta = std::fabs(m_master->m_realytNext[varIdx])*m_master->m_project.m_relTol + 0.01*m_master->m_project.m_absTol;
		m_master->m_realytNextIter[varIdx] += delta;
		// evaluate all slaves in cycle
		for (unsigned int s=0; s<cycle.m_slaves.size(); ++s) {
			Slave * slave = cycle.m_slaves[s];
			// reset slave
			slave->setState(m_master->m_t, m_master->m_iterationStates[slave->m_slaveIndex]);
			// then evaluate slave
			evaluateSlave(slave, m_master->m_realytNextIter, m_res);
		}
		// restore original values
		m_master->m_realytNextIter[varIdx] -= delta;
		// compute dS/dy = (Sy(y+delta) - Sy(y))/delta

		for (unsigned int j=0; j<varCount; ++j) {
			unsigned int colVarIdx = m_variableIdxMapping[c][j]; // global index of variable
			// process column i, row j
			double dq = -(m_res[colVarIdx] - m_master->m_realytNext[colVarIdx])/delta;
			// add unit matrix
			if (j == i)
				dq += 1;
			// store in Jacobian matrix column
			m_jacobianMatrix[c](i,j) = dq;
		}

	}

	// factorize matrix
	if (m_jacobianMatrix[c].lu() != 0)
		throw IBK::Exception("Error during LU factorization of matrix.", FUNC_ID);
}


} // namespace MASTER_SIM
