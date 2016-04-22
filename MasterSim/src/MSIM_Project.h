#ifndef MSIM_PROJECT_H
#define MSIM_PROJECT_H

#include <vector>

#include "MSIM_Simulator.h"

namespace MASTER_SIM {

/*! Stores content of configuration/project file. */
class Project {
public:
	/*! Operation modi of the master algorithm. */
	enum MasterMode {
		/*! Gauss-Seidel iteration with single iteration per step. */
		MM_GAUSS_SEIDEL_SINGLE		= 1,
		/*! Gauss-Seidel iteration with iteration until convergence. */
		MM_GAUSS_SEIDEL_ITERATIVE	= 2,
		/*! Newton iteration. */
		MM_NEWTON					= 9
	};

	/*! Holds all information that define a simulator. */
	struct SimulatorDef {
		SimulatorDef() : m_id() {}
		/*! Parses content of simulator definition line. */
		void parse(const std::string & simulatorDef);

		/*! Descriptive name. */
		std::string		m_name;
		/*! Unique ID. */
		unsigned int	m_id;
		/*! Priority. */
		unsigned int	m_priority;
		/*! Path to FMU file. */
		IBK::Path		m_pathToFMU;
	};

	/*! Defines an edge of the coupling graph. */
	struct GraphEdge {
		/*! Variable reference in simulator and value ref that exports this variable. */
		std::string m_outputVariableRef;
		/*! Variable reference in simulator and value ref that imports this variable. */
		std::string m_inputVariableRef;
	};

	/*! Constructor. */
	Project();

	/*! Reads project file.
		Throws an exception if reading fails.
	*/
	void read(const IBK::Path & prjFile);

	// Content of project file


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
	std::vector<SimulatorDef>	m_simulators;

	/*! Defines the connection graph. */
	std::vector<GraphEdge>		m_graph;

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


#endif // MSIM_PROJECT_H
