#ifndef MSIM_MASTERSIM_H
#define MSIM_MASTERSIM_H

#include <IBK_Path.h>

#include "MSIM_Project.h"
#include "MSIM_ArgParser.h"
#include "MSIM_FMUManager.h"
#include "MSIM_Slave.h"

/*! Namespace MASTER_SIM holds all classes, functions, types of the MasterSim library. */
namespace MASTER_SIM {

class AlgorithmGaussJacobi;

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

	/*! Initialize all FMUs (e.g. load dlls/shared libs, parse ModelDescription, do error checking.
		Throws an exception if an error occurs during instantiation.
	*/
	void instantiateFMUs(const ArgParser & args, const Project & prj);

	/*! Initialize FMUs (enter initialization mode, iterate over start conditions, everything
		up to ExitInitializationMode).
		After call to this function, everything is initialized so that doStep() can be called.
	*/
	void initialize();

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
	double tCurrent() const { return m_tCurrent; }

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
		using tStepSize() member function. The time point reached by the solver
		can be queried with tCurrent().
	*/
	void doStep();

	/*! Time step size used in last call to doStep(). */
	double tStepSize() const { return m_tStepSize; }

	/*! Updates outputs when scheduled.
		This function is typically called after each successful step and decides
		internally, whether outputs shall be written already, or not.
	*/
	void writeOutputs();

private:
	/*! Defines a group of FMUs that belong to a cycle.
		A cycle may only contain a single slave.
	*/
	struct Cycle {
		/*! Holds pointers to all slaves belonging to this cycle in the order they should be
			evaluated (for Gauss-Seidel). */
		std::vector<Slave*>			m_slaves;

		/*! All global variable indexes that are exchanged in this cycle, only variables
			that are both inputs and outputs of any of the slaves in this cycle.
		*/
		std::vector<unsigned int>	m_variableIndexes;
	};

	/*! This function handles the FMU unzipping and shared library loading stuff. */
	void importFMUs();

	/*! Checks all imported FMUs whether they provide the necessary functionality for the selected master algorithm. */
	void checkCapabilities();

	/*! Here all simulation slaves are instantiated. */
	void instatiateSlaves();

	/*! Performs error checking.
		If test is successful, m_stepSizeProposed is updated as well.
		\return Returns true if test was passed.
	*/
	bool doErrorCheck();

	/*! Performs convergence test by comparing values in m_ytNext and m_ytNextIter.
		\return Returns true if test has passed.
	*/
	bool doConvergenceTest();

	/*! Updates all inputs of a given slave using the variables in provided vector. */
	void updateSlaveInputs(Slave * slave, const std::vector<double> & variables);

	/*! Updates all inputs of a given slave using the variables in provided vector. */
	void syncSlaveOutputs(const Slave * slave, const std::vector<double> & variables);

	/*! Loops over all slaves and retrieves current states. */
	void storeCurrentSlaveStates(std::vector<void *> & slaveStates);

	/*! Copy of arg parser. */
	ArgParser				m_args;
	/*! Copy of project data. */
	Project					m_project;

	/*! Holds and owns all FMU objects (contant of the FMU archives). */
	FMUManager				m_fmuManager;

	/*! Vector of instantiated simulation slaves (owned by MasterSim). */
	std::vector<Slave*>		m_slaves;

	/*! All cycles in order of their evaluation priority. */
	std::vector<Cycle>		m_cycles;

	AlgorithmGaussJacobi	*m_algorithmGaussJacobi;


	/*! Current simulation time point. */
	double					m_tCurrent;

	/*! Simulation time step (that was used in last call to doStep(). */
	double					m_tStepSize;

	/*! Simulation time step that shall be used in next call to doStep(). */
	double					m_tStepSizeProposed;

	/*! Last time point when outputs were written. */
	double					m_tLastOutput;

	/*! Slave variables (input and output) at current master time. */
	std::vector<double>		m_yt;

	/*! Slave variables (input and output) at next master time (may be iterative quantities). */
	std::vector<double>		m_ytNext;

	/*! Slave variables (input and output) at next master time and last iteration level, needed for convergence test. */
	std::vector<double>		m_ytNextIter;

	/*! Vector for holding states of FMU slaves at begin of master algorithm to roll back during iterations. */
	std::vector<void*>		m_iterationStates;

	template<typename T>
	static void copyVector(const std::vector<T> & src, std::vector<T> & target) {
		IBK_ASSERT(src.size() == target.size());
		IBK_ASSERT(!src.empty());

		std::memcpy(&target[0], &src[0], src.size()*sizeof(T));
	}

	friend class AlgorithmGaussJacobi;
	friend class AlgorithmGaussSeidel;
};

} // namespace MASTER_SIM

#endif // MSIM_MASTERSIM_H
