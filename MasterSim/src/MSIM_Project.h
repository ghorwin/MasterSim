#ifndef MSIM_PROJECT_H
#define MSIM_PROJECT_H

#include <vector>
#include <string>

#include <IBK_Path.h>
#include <IBK_Unit.h>
#include <IBK_Color.h>
#include <IBK_Parameter.h>

namespace MASTER_SIM {

/*! Stores content of configuration/project file. */
class Project {
public:
	/*! Operation modi of the master algorithm. */
	enum MasterMode {
		/*! Gauss-Jacoby (without iteration). */
		MM_GAUSS_JACOBI,
		/*! Gauss-Seidel either with or without iteration (disabled when maxIterations == 1).
			Without iteration also error checking and time step adjustment is disabled.
			With iteration requires FMUs to support roll-back.
		*/
		MM_GAUSS_SEIDEL,
		/*! Newton iteration.
			Requires FMUs to support roll-back.
		*/
		MM_NEWTON
	};

	/*! Different options for controlling time integration error. */
	enum ErrorControlMode {
		/*! Error test disabled, does not execute step-doubling, use for non-iterative methods. */
		EM_NONE,
		/*! Does error test including step-doubling (requires FMUs to support variable steps and roll back) but
			only informs about missed checks, does not reduce time step.
		*/
		EM_CHECK,
		/*! Does error test including step-doubling (requires FMUs to support variable steps and roll back).
			Halves time step in case of error fault.

			\todo Idea: when test failed and time step is halved, the result of the first half-step should be used as converged result and
				error test repeated. So the re-doing of the full step is not needed. If time step is reduced to a fraction
				of the original step assuming linear growth of error in time, then a full step has to be taken again
				with the proposed time step (current implementation).
		*/
		EM_ADAPT_STEP
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
		/*! Color of simulator. */
		IBK::Color		m_color;

		/*! Parameter list, key = variable name, value = value as string.
			The conversion into the corresponding data type is done,
			once the model description has been read and the
			FMI variables are known for this slave/fmu.
		*/
		std::map<std::string, std::string>	m_parameters;
	};

	/*! Defines an edge of the coupling graph. */
	struct GraphEdge {
		/*! Extracts name of output slave (everything in front of first dot). */
		std::string outputSlaveName() const { return extractSlaveName(m_outputVariableRef); }
		/*! Extracts name of input slave (everything in front of first dot). */
		std::string inputSlaveName() const { return extractSlaveName(m_inputVariableRef); }

		/*! Extracts name of slave (everything in front of first dot). */
		static std::string extractSlaveName( const std::string & variableRef);

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
	void read(const IBK::Path & prjFile, bool headerOnly);

	/*! Writes project file.
		Throws an exception if writing fails.
	*/
	void write(const IBK::Path & prjFile) const;

	/*! Retrieves simulator definition for a given slave/simulator name. */
	const SimulatorDef & simulatorDefinition(const std::string & slaveName) const;

	// Content of project file

	// Meta data

	/*! Time stamp when project was last edited. */
	std::string					m_lastEdited;
	/*! Time stamp when project was created. */
	std::string					m_created;
	/*! Descriptive comment (utf8 encoded). */
	std::string					m_comment;



	/*! Starting time point of simulation, if not 0 serialization of existing state of FMUs is done at startup. */
	IBK::Parameter				m_tStart;

	/*! Simulation end time point, must be > 0. */
	IBK::Parameter				m_tEnd;

	/*! Lower time step limit (should be larger than m_tStepMin), if time step falls below this value,
		Gauss-Seidel without iteration is used. This allows to pass over step changes in outputs.
	*/
	IBK::Parameter				m_hFallBackLimit;

	/*! Lower limit for communication step size (only in variable step-size mode). */
	IBK::Parameter				m_hMin;

	/*! Maximum for communication step size (only in variable step-size mode). */
	IBK::Parameter				m_hMax;

	/*! Initial step size (for variable step-size mode) or fixed step size (for constant step-size mode). */
	IBK::Parameter				m_hStart;


	/*! Operation mode of the master algorithm. */
	MasterMode					m_masterMode;

	/*! Type of error control/time step adjustment scheme used. */
	ErrorControlMode			m_errorControlMode;

	/*! If error control mode is not adjusting step size, this option can be used for iterative
		methods to enable step size adjustment.
		If adjustStepSize is off, and error control mode does not adjust step size, any iterative
		algorithm that fails to converge will cause the simulation to stop.
	*/
	bool						m_adjustStepSize;

	/*! Maximum number of iterations per communication step (within each priority/cycle). */
	unsigned int				m_maxIterations;

	/*! Absolute tolerance - used for convergence check and for time integration error control. */
	double						m_absTol;
	/*! Relative tolerance - used for convergence check and for time integration error control. */
	double						m_relTol;

	/*! Minimum output time step. */
	IBK::Parameter				m_tOutputStepMin;

	/*! If true, binary output files are written (smaller and faster). */
	bool						m_binaryOutputFiles;

	/*! Unit defined as output time for output files. */
	IBK::Unit					m_outputTimeUnit;


	/*! All simulators coupled in this master scenario. */
	std::vector<SimulatorDef>	m_simulators;

	/*! Defines the connection graph. */
	std::vector<GraphEdge>		m_graph;

};

} // namespace MASTER_SIM


#endif // MSIM_PROJECT_H
