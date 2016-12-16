#ifndef MSIM_ALGORITHMGAUSSJACOBI_H
#define MSIM_ALGORITHMGAUSSJACOBI_H

#include "MSIM_AbstractAlgorithm.h"

namespace MASTER_SIM {

/*! Implementation class for Gauss-Jacobi algorithm.

	This algorithm does not do any state-setting or state-getting, and is
	compatible with FMI for CoSim v1.
*/
class AlgorithmGaussJacobi : public AbstractAlgorithm {
public:
	/*! Default constructor. */
	AlgorithmGaussJacobi(MasterSim * master) : AbstractAlgorithm(master) {}

	/*! Initialization function, called once all slaves have been set up. */
	void init();

	/*! Master-algorithm will evaluate all FMUs and advance state in time.
		Will throw an exception if any of the FMUs fails in unrecoverable manner.
	*/
	Result doStep();
};

} // namespace MASTER_SIM

#endif // MSIM_ALGORITHMGAUSSJACOBI_H
