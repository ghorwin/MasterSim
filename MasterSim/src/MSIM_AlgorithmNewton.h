#ifndef MSIM_ALGORITHMNEWTON_H
#define MSIM_ALGORITHMNEWTON_H

#include <vector>

#include <IBKMK_DenseMatrix.h>

#include "MSIM_AbstractAlgorithm.h"

namespace MASTER_SIM {

class Slave;

/*! Implementation class for Newton algorithm.

	This algorithm uses a difference-quotient approximation to the Jacobi matrix which is
	treated as dense matrix.
*/
class AlgorithmNewton : public AbstractAlgorithm {
public:
	/*! Default constructor. */
	AlgorithmNewton(MasterSim * master) : AbstractAlgorithm(master) {}

	/*! Initializes Jacobian matrix and auxilliary structures. */
	void init();

	/*! Master-algorithm will evaluate all FMUs and advance state in time.
		Will throw an exception if any of the FMUs fails in unrecoverable manner.
	*/
	Result doStep();

	/*! Advances slave using xi as inputs and updates xi1 (only real values). */
	Result evaluateSlave(MASTER_SIM::Slave * slave, const std::vector<double> & xi, std::vector<double> & xi1);

	/*! Computes Jacobian using DQ approximation and factorizes it.
		\param c Cycle index.
	*/
	void generateJacobian(unsigned int c);


	/*! Jacobian matrixes for each cycle, can be empty in case of only one slave per cycle. */
	std::vector<IBKMK::DenseMatrix>				m_jacobianMatrix;

	/*! Maps the index of a variable in the matrix to the index of the corresponding variable in
		the global index array. First index is the cycle, second index the matrix index.
		\code
		unsigned int varIdx = m_variableIdxMapping[cycle][matrixIdx];
		\endcode
	*/
	std::vector< std::vector<unsigned int> >	m_variableIdxMapping;

	/*! Residuals of Newton equation (full vector size with all real variables). */
	std::vector<double>							m_res;
	/*! Right-hand-side of Newton equation system (size matches dimension of Jacobian matrix/only coupled variables). */
	std::vector<double>							m_rhs;
};

} // namespace MASTER_SIM

#endif // MSIM_ALGORITHMNEWTON_H
