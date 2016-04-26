#ifndef MSIM_MASTERSIM_H
#define MSIM_MASTERSIM_H

#include <IBK_Path.h>

#include "MSIM_Project.h"
#include "MSIM_ArgParser.h"
#include "MSIM_FMUManager.h"

/*! Namespace MASTER_SIM holds all classes, functions, types of the MasterSim library. */
namespace MASTER_SIM {

class MasterSimulator {
public:
	MasterSimulator();

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
		This function calls doStep() until simulation time has reached/passed time point.

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

	/*! Updates outputs when scheduled. */
	void writeOutputs();

private:
	/*! Copy of arg parser. */
	ArgParser				m_args;
	/*! Copy of project data. */
	Project					m_project;

	/*! Holds and owns all FMU objects. */
	FMUManager				m_fmuManager;


	double					m_tCurrent;
	double					m_tStepSize;

	double					m_tLastOutput;
};

} // namespace MASTER_SIM

#endif // MSIM_MASTERSIM_H
