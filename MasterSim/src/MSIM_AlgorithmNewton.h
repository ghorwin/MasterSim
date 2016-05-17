#ifndef MSIM_ALGORITHMNEWTON_H
#define MSIM_ALGORITHMNEWTON_H

#include <vector>

//#include <IBK_DenseMatrix.h>

#include "MSIM_AbstractAlgorithm.h"

namespace MASTER_SIM {

/*! Implementation class for Newton algorithm.

	This algorithm uses a difference-quotient approximation to the Jacobi matrix which is
	treated as dense matrix.
*/
class AlgorithmNewton : public AbstractAlgorithm {
public:
	/*! Default constructor. */
	AlgorithmNewton(MasterSim * master) : AbstractAlgorithm(master) {}

	/*! Master-algorithm will evaluate all FMUs and advance state in time.
		Will throw an exception if any of the FMUs fails in unrecoverable manner.
	*/
	Result doStep();

	// Jacobian matrixes for each cycle, can be empty in case of only one slave per cycle
//	std::vector<DenseMatrix>	m_jacobianMatrix;
};

} // namespace MASTER_SIM

#endif // MSIM_ALGORITHMNEWTON_H
