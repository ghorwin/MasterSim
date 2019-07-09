#ifndef MSIM_MASTERSIM_H
#define MSIM_MASTERSIM_H

#include <utility> // for std::pair

#include <IBK_Path.h>
#include <IBK_StopWatch.h>
#include <IBK_assert.h>

#include "MSIM_Project.h"
#include "MSIM_ArgParser.h"
#include "MSIM_FMUManager.h"
#include "MSIM_Slave.h"
#include "MSIM_OutputWriter.h"


/*! Namespace MASTER_SIM holds all classes, functions, types of the MasterSim library. */
namespace MASTER_SIM {

class AbstractAlgorithm;

/*! Main class of the library.
	This class encapsulates all functionality of the master and can be used from GUI
	applications as well as console masters.
*/
class MasterSim {
public:
	/*! Constructor. */
	MasterSim();

	/*! Destructor, cleans up slaves. */
	~MasterSim();

	/*! Writes version info to message handler. */
	static void writeVersionInfo();

	/*! Initialize all FMUs (e.g. load dlls/shared libs, parse ModelDescription, do error checking.
		Throws an exception if an error occurs during instantiation.
	*/
	void importFMUs(const ArgParser & args, const Project & prj);

	/*! Initialize FMUs (enter initialization mode, iterate over start conditions, everything
		up to ExitInitializationMode).
		After call to this function, everything is initialized so that doStep() can be called.
	*/
	void initialize();

	/*! (Re-)opens output files. */
	void openOutputFiles(bool reopen);

	/*! Restores state of master and all FMUs using data stored below state directory. */
	void restoreState(double t, const IBK::Path & stateDirectory);

	/*! Serializes all FMUs and state of master to file within stateDirectory. */
	void storeState(const IBK::Path & stateDirectory);

	/*! Contains the core simulation loop.
		This is essentially a convenience function around doStep() calls.
		This function calls doStep() until simulation time has reached/passed time point.
		After each successful step, the function writeOutputs() is called.
	*/
	void simulate();

	/*! Returns current time point (of master state). */
	double tCurrent() const { return m_t; }

	/*! Attempts to integrate forward a single step.
		For fixed step size mode, the step size m_h is used.
		For variably step size mode, the step size m_h is attempted, and in case of
		convergence failure or error check failure, step size is reduced and
		integration step attempted again.

		Function throws an Exception in case of failure (solver cannot continue).
		For fixed step size mode failure
		means convergence failure or FMU reported error. For variable step size
		failure means time step reduction below acceptable limit or any other error.

		For variable time step scheme you can query the time step finally used by
		using h() member function. The time point reached by the solver
		can be queried with tCurrent().
	*/
	void doStep();

	/*! Time step size used in last call to doStep(). */
	double h() const { return m_h; }

	/*! Appends current output variables of all slaves to output files. */
	void appendOutputs();

	/*! Writes final statistics. */
	void writeMetrics() const;

	/*! Releases slave memory (calls fmi2FreeInstance() FMU function first on slave, then frees slave memory). */
	void freeSlaves();

private:
	/*! Defines a group of FMUs that belong to a cycle.
		A cycle may only contain a single slave.
	*/
	struct Cycle {
		/*! Holds pointers to all slaves belonging to this cycle in the order they should be
			evaluated (for Gauss-Seidel).
			\note Pointers are not owned.
		*/
		std::vector<Slave*>			m_slaves;
	};

	/*! Variable mapping structure, used to map exchange quantities between FMUs for the different data types. */
	struct VariableMapping {
		unsigned int	m_globalIndex; // matches index of VariableMapping object in m_variableMappings vector

		Slave			*m_outputSlave;
		unsigned int	m_outputLocalIndex; // index of variable slot in locally cached output variables vector

		Slave			*m_inputSlave;
		unsigned int	m_inputValueReference;
	};

	/*! This function handles the FMU unzipping and shared library loading stuff. */
	void importFMUs();

	/*! Checks all imported FMUs whether they provide the necessary functionality for the selected master algorithm. */
	void checkCapabilities();

	/*! Here all simulation slaves are instantiated. */
	void instatiateSlaves();

	/*! Convenience function that extracts slave and variables names from flatVarName and
		looks up slave and FMI variable in associated FMU.
	*/
	std::pair<const Slave*, const FMIVariable *> variableByName(const std::string & flatVarName) const;

	/*! Collects all output variables from all slaves and adds them to the variables vector, ordered according to cycles. */
	void composeVariableVector();

	/*! Initializes the master algorithm. */
	void initMasterAlgorithm();

	/*! This generates default parameters recognized by the master.
		The parameters will be set for each FMU that imports the matching parameter and
		a value was not yet specified by the user manually.
	*/
	void setupDefaultParameters();

	/*! Computes initial conditions and updates output caches of all slaves so that master algorithms can start. */
	void initialConditions();

	/*! Error testing procedure based on Richardson-Extrapolation.
		This method will take two half-steps and compare the result
		obtained after the two half steps with the original result to
		estimate the error (may involve many doStep() calls to FMU slaves).

		This function starts with an integration step completed.
		It remembers the state at the old time point t, the newly
		computed states at time t + h. Then it resets all
		slaves back to the point t and takes two steps with size
		h/2. The comparison of the first (long) step result
		and the result obtained with the two steps makes up the error
		test.
		If test is successful, the result of the two smaller steps
		is stored in the m_xxxNext variables and m_h is reset
		to the total step size. So, the calling function can complete
		the step just as if doErrorCheck() was never called.

		In case of failure, the calling function should decide whether
		a reset of the step is meaningful or if the integration shall
		continue with the errorenous solution.

		If test is successful, m_hProposed is updated as well.
		\return Returns true if test was passed.
	*/
	bool doErrorCheckRichardson();

	/*! Simplified error check without iteration but extrapolation
		of comparison value from historical values (essentially a
		two-step method with extrapolation).

		This function starts with an integration step completed.
		It remembers the state at the old time point t, the newly
		computed states at time t + h.

		The test is based on extrapolated values using solution from
		t - h and t, and the newly computed solution t + h.

		In case of failure, the calling function should decide whether
		a reset of the step is meaningful or if the integration shall
		continue with the errorenous solution.

		If test is successful, m_hProposed is updated as well.
		\return Returns true if test was passed.
	*/
	bool doErrorCheckWithoutIteration();

	/*! Implementation of the error adjustment formulae.
		Computes new time step based on current error estimate and time step m_h but obeys upper and lower scaling limits.
		\return Returns the computed time step.
	*/
	double adaptTimeStepBasedOnErrorEstimate(double errEstimate) const;


	/*! Updates all connected inputs of a given slave using the variables in provided vector. */
	void updateSlaveInputs(Slave * slave,
						   const std::vector<double> & variables,
						   const std::vector<int> &intVariables,
						   const std::vector<fmi2Boolean> &boolVariables,
						   const std::vector<std::string> &stringVariables,
						   bool realOnly);

	/*! Copies all connected outputs of a given slave into the vector 'variables'. */
	void syncSlaveOutputs(const Slave * slave,
						  std::vector<double> & realVariables,
						  std::vector<int> & intVariables,
						  std::vector<fmi2Boolean> &boolVariables,
						  std::vector<std::string> & stringVariables,
						  bool realOnly);

	/*! Loops over all slaves and retrieves current states. */
	void storeCurrentSlaveStates(std::vector<void *> & slaveStates);

	/*! Loops over all slaves and restores state from saved states.
		This will not automatically cache the slave outputs and sync with the
		variable vectors.
	*/
	void restoreSlaveStates(double t, const std::vector<void*> & slaveStates);

	/*! Creates statistics files and appends statistics.
		This function is called once after each completed doStep();
	*/
	void writeStepStatistics();

	/*! Copy of arg parser. */
	ArgParser				m_args;
	/*! Copy of project data. */
	Project					m_project;

	/*! If true, all FMUs must have the variable time step flag set - required for adjusting time step size.
		This variable is set in checkCapabilities().
	*/
	bool					m_enableVariableStepSizes;

	/*! If true, the error test with adaptive time stepping is selected and prior to calling doStep()
		the states of all FMUs are stored.
		All FMUs must be able to get/set their state. This variable is set in checkCapabilities().
	*/
	bool					m_useErrorTestWithVariableStepSizes;

	/*! If true, an iterative master algorithm is used and state need to be stored prior to calling master algorithm.
		All FMUs must be able to get/set their state. This variable is set in checkCapabilities().
	*/
	bool					m_enableIteration;

	/*! Holds and owns all FMU objects (contant of the FMU archives). */
	FMUManager				m_fmuManager;

	/*! Vector of instantiated simulation slaves (owned by MasterSim). */
	std::vector<Slave*>		m_slaves;

	/*! All cycles in order of their evaluation priority. */
	std::vector<Cycle>		m_cycles;

	/*! Pointer to the actual master algorithm implementation (owned). */
	AbstractAlgorithm		*m_masterAlgorithm;

	/*! Current simulation time point. */
	double					m_t;

	/*! Simulation time step (that was used in last call to doStep(). */
	double					m_h;

	/*! Simulation time step that shall be used in next call to doStep(). */
	double					m_hProposed;

	/*! Manager of output files, handles all output file writing. */
	OutputWriter			m_outputWriter;

	/*! Output file stream for master statistics. */
	std::ofstream			*m_stepStatsOutput;

	/*! Mapping of all connected variables of type real. */
	std::vector<VariableMapping>	m_realVariableMapping;
	/*! Mapping of all connected variables of type int. */
	std::vector<VariableMapping>	m_intVariableMapping;
	/*! Mapping of all connected variables of type bool. */
	std::vector<VariableMapping>	m_boolVariableMapping;
	/*! Mapping of all connected variables of type string. */
	std::vector<VariableMapping>	m_stringVariableMapping;

	// exchange variables of type real

	/*! Slave variables (input and output) at current master time. */
	std::vector<double>				m_realyt;
	/*! Slave variables (input and output) at next master time (may be iterative quantities). */
	std::vector<double>				m_realytNext;
	/*! Slave variables (input and output) at next master time and last iteration level, needed for convergence test. */
	std::vector<double>				m_realytNextIter;

	// exchange variables of type int

	/*! Slave variables (input and output) at current master time. */
	std::vector<int>				m_intyt;
	/*! Slave variables (input and output) at next master time (may be iterative quantities). */
	std::vector<int>				m_intytNext;
	/*! Slave variables (input and output) at next master time and last iteration level, needed for convergence test. */
	std::vector<int>				m_intytNextIter;

	// exchange variables of type boolean

	/*! Slave variables (input and output) at current master time. */
	std::vector<fmi2Boolean>		m_boolyt;
	/*! Slave variables (input and output) at next master time (may be iterative quantities). */
	std::vector<fmi2Boolean>		m_boolytNext;
	/*! Slave variables (input and output) at next master time and last iteration level, needed for convergence test. */
	std::vector<fmi2Boolean>		m_boolytNextIter;

	// exchange variables of type string

	/*! Slave variables (input and output) at current master time. */
	std::vector<std::string>		m_stringyt;
	/*! Slave variables (input and output) at next master time (may be iterative quantities). */
	std::vector<std::string>		m_stringytNext;
	/*! Slave variables (input and output) at next master time and last iteration level, needed for convergence test. */
	std::vector<std::string>		m_stringytNextIter;


	// variables related to error checking

	/*! Time point at start of original step. */
	double							m_errTOriginal;
	/*! Slave variables (input and output) at time t+h, computed by long step. */
	std::vector<double>				m_errRealytFirst;

	/*! Slave variables (input and output) at current master time, type real, stored as backup for resetting state
		when error test fails.
		When error test is using the monitoring variant only, this holds the state at m_errTOriginal.
		It is initialized with an empty vector, so error checking is only possible after the second step.
	*/
	std::vector<double>				m_errRealyt;
	/*! Slave variables (input and output) at current master time, type int, stored as backup for resetting state when error test fails. */
	std::vector<int>				m_errIntyt;
	/*! Slave variables (input and output) at current master time, type bool, stored as backup for resetting state when error test fails. */
	std::vector<fmi2Boolean>		m_errBoolyt;
	/*! Slave variables (input and output) at current master time, type string, stored as backup for resetting state when error test fails. */
	std::vector<std::string>		m_errStringyt;

	/*! Error norm that was determined by step-doubling test to confirm the step. */
	double							m_acceptedErrRichardson;
	/*! Error norm that was determined by slope-check to confirm the step. */
	double							m_acceptedErrSlopeCheck;

	/*! Vector for holding states of FMU slaves at begin of master algorithm to roll back during iterations. */
	std::vector<void*>				m_iterationStates;
	/*! Vector for holding states of FMU slaves at begin of error check interval to roll back when error check has failed. */
	std::vector<void*>				m_errorCheckStates;

	/*! Counts for roll backs of all slaves (size nSlaves). */
	std::vector<unsigned int>		m_statRollBackCounters;
	/*! Time taken while setState() calls to all slaves during iteration (not Jacobi matrix setup). */
	std::vector<double>				m_statRollBackTimes;
	/*! Counts for setting states of all slaves (size nSlaves). */
	std::vector<unsigned int>		m_statStoreStateCounters;
	/*! Time taken while currentState() calls to all slaves during iteration (not Jacobi matrix setup). */
	std::vector<double>				m_statStoreStateTimes;
	/*! Counts for slave evaluation of all slaves (size nSlaves). */
	std::vector<unsigned int>		m_statSlaveEvalCounters;
	/*! Time taken while doStep() calls to all slaves during iteration (not Jacobi matrix setup). */
	std::vector<double>				m_statSlaveEvalTimes;

	IBK::StopWatch					m_timer;

	double							m_statOutputTime;
	double							m_statAlgorithmTime;
	unsigned int					m_statConvergenceFailsCounter;
	unsigned int					m_statErrorTestFailsCounter;
	double							m_statErrorTestTime;
	unsigned int					m_statStepCounter;
	unsigned int					m_statAlgorithmCallCounter;


	/*! Utility function to copy one vector to another using memcpy. */
	template<typename T>
	static void copyVector(const std::vector<T> & src, std::vector<T> & target) {
		IBK_ASSERT(src.size() == target.size());
		if (src.empty())
			return;

		std::memcpy(&target[0], &src[0], src.size()*sizeof(T));
	}

	friend class AlgorithmGaussJacobi;
	friend class AlgorithmGaussSeidel;
	friend class AlgorithmNewton;

	friend class ErrorControl;
};

} // namespace MASTER_SIM

#endif // MSIM_MASTERSIM_H
