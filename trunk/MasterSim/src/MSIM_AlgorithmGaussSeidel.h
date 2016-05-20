#ifndef MSIM_ALGORITHMGAUSSSEIDEL_H
#define MSIM_ALGORITHMGAUSSSEIDEL_H

#include "MSIM_AbstractAlgorithm.h"

namespace MASTER_SIM {

/*! Implementation class for Gauss-Seidel algorithm.

	If run in non-iterative mode (m_maxIterations parameter == 1),
	this algorithm does not do any state-setting or state-getting, and is compatible with FMI for CoSim v1.
*/
class AlgorithmGaussSeidel : public AbstractAlgorithm {
public:
	/*! Default constructor. */
	AlgorithmGaussSeidel(MasterSim * master) : AbstractAlgorithm(master) {}

	/*! Master-algorithm will evaluate all FMUs and advance state in time.
		Will throw an exception if any of the FMUs fails in unrecoverable manner.
	*/
	Result doStep();

	/*! Performs convergence test by comparing values in m_ytNext and m_ytNextIter.
		\return Returns true if test has passed.
	*/
	bool doConvergenceTest();

};

} // namespace MASTER_SIM

#endif // MSIM_ALGORITHMGAUSSSEIDEL_H
