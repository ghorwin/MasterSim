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
		R_DIVERGED,
		R_ITERATION_LIMIT_EXCEEDED,
		R_RETRY
	};

	/*! Default constructor. */
	AbstractAlgorithm(MasterSim * master) : m_master(master) {}

	/*! Virtual d'tor. */
	virtual ~AbstractAlgorithm() {}

	/*! Initialization function, called once all slaves have been set up. */
	virtual void init() {}

	/*! Main stepper function for master algorithm. */
	virtual Result doStep() = 0;

protected:
	/*! Cached pointer to master data structure (not owned). */
	MasterSim		*m_master;

	/*! Timer to use for instrumenting calls to FMUs. */
	IBK::StopWatch	m_timer;
};

} // namespace MASTER_SIM

#endif // MSIM_ABSTRACTALGORITHM_H
