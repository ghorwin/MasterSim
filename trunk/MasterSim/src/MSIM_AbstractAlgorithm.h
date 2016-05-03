#ifndef MSIM_ALGORITHMGAUSSJACOBI_H
#define MSIM_ALGORITHMGAUSSJACOBI_H

namespace MASTER_SIM {

class MasterSim;

/*! Implementation class for Gauss-Jacobi algorithm.

	This algorithm does not do any state-setting or state-getting, and is
	compatible with FMI for CoSim v1.
*/
class AlgorithmGaussJacobi {
public:
	/*! Default constructor. */
	AlgorithmGaussJacobi(MasterSim * master) : m_master(master) {}

	/*! Master-algorithm will evaluate all FMUs and advance state in time.
		Will throw an exception if any of the FMUs fails to compute or any
		other error occurs.
	*/
	void doStep();

private:
	/*! Cached pointer to master data structure. */
	MasterSim	*m_master;
};

} // namespace MASTER_SIM

#endif // MSIM_ALGORITHMGAUSSJACOBI_H
