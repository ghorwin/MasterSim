#ifndef MSIM_PROJECT_H
#define MSIM_PROJECT_H

#include <vector>
#include <string>
#include <IBK_Path.h>

namespace MASTER_SIM {

/*! Stores content of configuration/project file. */
class Project {
public:
	/*! Operation modi of the master algorithm. */
	enum MasterMode {
		/*! Gauss-Jacoby (without iteration). */
		MM_GAUSS_JACOBI,
		/*! Gauss-Seidel with iteration until convergence or maxIterations. */
		MM_GAUSS_SEIDEL,
		/*! Newton iteration. */
		MM_NEWTON
	};

	/*! Different options for controlling time integration error. */
	enum ErrorControlMode {
		EM_NONE,
		EM_CHECK_ONLY
	};

	/*! Holds all information that define a simulator. */
	struct SimulatorDef {
		/*! Parses content of simulator definition line. */
		void parse(const std::string & simulatorDef);

		/*! Unique identification name. */
		std::string		m_name;
		/*! Cycle for this simulator. */
		unsigned int	m_cycle;
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

	/*! Type of error control/time step adjustment scheme used. */
	ErrorControlMode	m_errorControlMode;

	/*! Maximum number of iterations per communication step (within each priority/cycle). */
	unsigned int	m_maxIterations;

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
