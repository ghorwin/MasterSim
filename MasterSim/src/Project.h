#ifndef PROJECT_H
#define PROJECT_H

#include <vector>

#include "MSIM_Simulator.h"

namespace MASTER_SIM {

/*! Stores content of configuration/project file. */
class Project {
public:
	Project();

	/*! Reads project file.
		Throws an exception if reading fails.
	*/
	void read(const IBK::Path & prjFile);


	// Content of project file

	/*! Operation modi of the master algorithm. */
	enum MasterMode {
		/*! Gauss-Seidel iteration with single iteration per step. */
		MM_GAUSS_SEIDEL_SINGLE		= 1,
		/*! Gauss-Seidel iteration with iteration until convergence. */
		MM_GAUSS_SEIDEL_ITERATIVE	= 2,
		/*! Newton iteration. */
		MM_NEWTON					= 9
	};

	/*! Number of interface values between FMUs. */
	unsigned int	m_nValues;

	/*! Number of simulation model instances. */
	unsigned int	m_nModelInstances;

	/*! Starting time point of simulation, if not 0 serialization of existing state of FMUs is done at startup. */
	double			m_tStart;

	/*! Simulation end time point, must be > 0. */
	double			m_tEnd;

	/*! Maximum for communication step size (only in variable step-size mode). */
	double			m_tStepMax;

	/*! Initial step size (for variable step-size mode) or fixed step size (for constant step-size mode). */
	double			m_tStepStart;

	/*! Operation mode of the master algorithm. */
	MasterMode		m_masterMode;

	// MasterDebug - replaced by command-line argument --verbosity-level
	// OutputGnuplot - replaced by output mode parameter

	/*! Maximum number of iterations per communication step (within each priority/cycle). */
	unsigned int	m_maxSteps;

	/*! Absolute tolerance - used for convergence check and for time integration error control. */
	double			m_absTol;
	/*! Relative tolerance - used for convergence check and for time integration error control. */
	double			m_relTol;

	/*! All simulators coupled in this master scenario. */
	std::vector<Simulator>	m_simulators;

//	simulator   0   fmus/simx/Part1.fmu
//	simulator   1   fmus/simx/Part2.fmu
//	simulator   2   fmus/simx/Part3.fmu
//	graph #val #sim -1(out)/1(in) valueref
//	0   0   -1      r Part1.x2
//	0   1    1      r Part2.x2
//	1   0   -1      r Part1.x1
//	1   1    1      r Part2.x1
//	2   1   -1      r Part2.x3
//	2   2    1      r Part3.x3
//	3   2   -1      r Part3.x4
//	3   1    1      r Part2.x4
//	end
//	priority #sim priority
//	0   0
//	1   1
//	2   1
//	end
//	cycles  #prior    0(no)/1(yes)
//	0   0
//	1   1
//	end
};

} // namespace MASTER_SIM


#endif // PROJECT_H
