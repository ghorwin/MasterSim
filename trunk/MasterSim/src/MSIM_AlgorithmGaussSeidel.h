#ifndef MSIM_ALGORITHMGAUSSSEIDEL_H
#define MSIM_ALGORITHMGAUSSSEIDEL_H

namespace MASTER_SIM {

class MasterSim;

/*! Implementation class for Gauss-Seidel algorithm.

	If run in non-iterative mode (m_maxIterations parameter == 1),
	this algorithm does not do any state-setting or state-getting, and is compatible with FMI for CoSim v1.
*/
class AlgorithmGaussSeidel {
public:
	enum Result {
		R_CONVERGED,
		R_DIVERGED,
		R_ITERATION_LIMIT_EXCEEDED,
		R_RETRY
	};

	/*! Default constructor. */
	AlgorithmGaussSeidel(MasterSim * master) : m_master(master) {
	}

	/*! Master-algorithm will evaluate all FMUs and advance state in time.
		Will throw an exception if any of the FMUs fails in unrecoverable manner.
	*/
	Result doStep();

private:
	/*! Cached pointer to master data structure. */
	MasterSim	*m_master;
};

} // namespace MASTER_SIM

#endif // MSIM_ALGORITHMGAUSSSEIDEL_H
