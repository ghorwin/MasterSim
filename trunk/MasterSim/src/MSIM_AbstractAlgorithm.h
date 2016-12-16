#ifndef MSIM_ABSTRACTALGORITHM_H
#define MSIM_ABSTRACTALGORITHM_H

#include <IBK_StopWatch.h>

namespace MASTER_SIM {

class MasterSim;

/*! Abstract base class for implementation of master algorithms.
*/
class AbstractAlgorithm {
public:
	/*! Return values for the doStep() function. */
	enum Result {
		R_CONVERGED,
		R_ITERATION_LIMIT_EXCEEDED,
		R_RETRY
	};

	/*! Default constructor. */
	AbstractAlgorithm(MasterSim * master) :
		m_master(master),
		m_nIterations(0),
		m_nIterationLimitExceeded(0),
		m_nFMUErrors(0)
	{}

	/*! Virtual d'tor. */
	virtual ~AbstractAlgorithm() {}

	/*! Initialization function, called once all slaves have been set up. */
	virtual void init() {}

	/*! Main stepper function for master algorithm. */
	virtual Result doStep() = 0;

	/*! Returns collected statistics. */
	void stats(unsigned int	& nIterations, unsigned int & nIterationLimitExceeded, unsigned int & nFMUErrors) {
		m_nIterations = nIterations;
		m_nIterationLimitExceeded = nIterationLimitExceeded;
		m_nFMUErrors = nFMUErrors;
	}

protected:
	/*! Cached pointer to master data structure (not owned). */
	MasterSim		*m_master;

	/*! Timer to use for instrumenting calls to FMUs. */
	IBK::StopWatch	m_timer;

	/*! Number of iterations. */
	unsigned int	m_nIterations;
	/*! Number of iteration limits exceeded. */
	unsigned int	m_nIterationLimitExceeded;
	/*! Number of times the algorithm has to be repeated because of FMU errors. */
	unsigned int	m_nFMUErrors;
};

} // namespace MASTER_SIM

#endif // MSIM_ABSTRACTALGORITHM_H
