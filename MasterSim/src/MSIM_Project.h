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

	/*! Maximum number of iterations per communication step (within each priority/cycle). */
	unsigned int	m_maxSteps;

	/*! Absolute tolerance - used for convergence check and for time integration error control. */
	double			m_absTol;
	/*! Relative tolerance - used for convergence check and for time integration error control. */
	double			m_relTol;

	/*! Minimum output time step. */
	double			m_tOutputStepMin;

	/*! All simulators coupled in this master scenario. */
	std::vector<SimulatorDef>	m_simulators;

	/*! Defines the connection graph. */
	std::vector<GraphEdge>		m_graph;

};

} // namespace MASTER_SIM


#endif // MSIM_PROJECT_H
