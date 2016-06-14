#include "MSIM_MasterSim.h"

#include <memory> // for auto_ptr
#include <cmath>

#include <IBK_Exception.h>
#include <IBK_messages.h>
#include <IBK_StringUtils.h>
#include <IBK_assert.h>
#include <IBK_Time.h>

#include "MSIM_Constants.h"
#include "MSIM_FMU.h"
#include "MSIM_Slave.h"
#include "MSIM_AlgorithmGaussJacobi.h"
#include "MSIM_AlgorithmGaussSeidel.h"
#include "MSIM_AlgorithmNewton.h"

namespace MASTER_SIM {

MasterSim::MasterSim() :
	m_masterAlgorithm(NULL),
	m_t(0)
{
}


MasterSim::~MasterSim() {
	// clean up master algorithm
	delete m_masterAlgorithm;

	// release allocated memory of slaves
	for (unsigned int i=0; i<m_slaves.size(); ++i)
		delete m_slaves[i];
}


void MasterSim::writeVersionInfo() {
	const char * const FUNC_ID = "[MasterSim::writeVersionInfo]";
	IBK::IBK_Message( IBK::FormatString("MasterSimulator, version %1\n").arg(LONG_VERSION), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( "Copyright by Andreas Nicolai, released under GPL license (see LICENSE file).\n\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
}


void MasterSim::importFMUs(const ArgParser &args, const Project & prj) {
	//const char * const FUNC_ID = "[MasterSimulator::importFMUs]";

	// create copy of input data (needed for multi-threaded application)
	m_args = args;
	m_project = prj;

	// import all FMUs
	importFMUs();

	// check required capabilities of FMUs, this depends on the selected master algorithm
	checkCapabilities();
}


void MasterSim::initialize() {
	// instantiate all slaves
	instatiateSlaves();

	// collect all output and input variables from all slaves, ordered according to cycles
	composeVariableVector();

	// select master algorithm
	initMasterAlgorithm();

	// sets up default parameters like ResultsRootDir
	setupDefaultParameters();

	// compute initial conditions (enter and exit initialization mode)
	initialConditions();

	// setup output writer
	m_outputWriter.m_project = &m_project; // Note: persistant pointer, must be valid for lifetime of output writer
	m_outputWriter.m_slaves = m_slaves; // copy pointer as persistant pointers, must not modify m_slaves vector from here on
	m_outputWriter.m_resultsDir = (m_args.m_workingDir / "results").absolutePath();
	m_outputWriter.m_logDir = (m_args.m_workingDir / "logs").absolutePath();
	m_outputWriter.m_projectFile = m_args.m_projectFile.str();
	m_outputWriter.setupProgressReport();

	// setup time-stepping variables
	m_h = m_project.m_hStart.value;
	m_hProposed = m_h;

	// setup statistics
	m_statOutputTime = 0;
	m_statAlgorithmTime = 0;
	m_statConvergenceFailsCounter = 0;
	m_statErrorTestFailsCounter = 0;
	m_statErrorTestTime = 0;
	m_statStepCounter = 0;
}




void MasterSim::restoreState(double t, const IBK::Path & stateDirectory) {
	// all FMUs must be able to restore state!

	// for all FMU instances:
	// - do normal FMU init
	// - compute FMU-specific state file path
	// - read serialization file and extract meta header
	// - tell FMU to de-serialize state

	// update state of master simulator
}


void MasterSim::storeState(const IBK::Path & stateDirectory) {
	// all FMUs must be able to store state!

	// for all FMU instances:
	// - compute FMU-specific state file path
	// - ask for serialization size and create dummy vector
	// - ask for FMU state and then serialize content of FMU
	// - dump state into file with meta-data header

	// dump state of master simulator
}


void MasterSim::simulate() {
	while (m_t < m_project.m_tEnd.value) {
		// Do an internal step with the selected master algorithm
		doStep();
		++m_statStepCounter;
		// Now the master's internal state has moved to the next time point m_t = m_t + m_h
		// m_h holds the step size used during the last master step

		if (m_t < m_project.m_tEnd.value)
			appendOutputs();// appends outputs (filtering/scheduling is implemented inside OutputWriter class)
	}
	// write final results
	m_outputWriter.m_tLastOutput = -1;  // this ensures that final results are definitely written
	appendOutputs();
}


void MasterSim::doStep() {
	const char * const FUNC_ID = "[MasterSim::doStep]";

	// state of master and fmus at this point:
	// - all FMUs and their outputs correspond to master time t
	// - m_hProposed holds suggested time step size for next step
	// - m_h holds time step size of _last_ completed step

	m_h = m_hProposed; // set proposed time step size

	// if we have error control enabled, store current state of all fmu slaves
	if (m_enableIteration) {
		// request state from all slaves
		storeCurrentSlaveStates(m_iterationStates);
	}

	// step size reduction loop
	while (true) {

		// let master do one step
		m_timer.start();
		AbstractAlgorithm::Result res = m_masterAlgorithm->doStep();
		m_statAlgorithmTime += m_timer.stop()*1e-3;
		switch (res) {
			case AbstractAlgorithm::R_CONVERGED :
				break;

			default : {
				if (!m_enableVariableStepSizes || !m_enableIteration) {
					throw IBK::Exception(IBK::FormatString("Step failure at t=%1, taking step size %2. "
														   "Reduction of time step is not allowed, stopping here.")
										 .arg(m_t).arg(m_h), FUNC_ID);
				}
				if (m_h/2 < m_project.m_hMin.value)
					throw IBK::Exception(IBK::FormatString("Step failure at t=%1, taking step size %2. "
														   "Reducing step would fall below minimum step size of %3.")
										 .arg(m_t).arg(m_h).arg(m_project.m_hMin.value), FUNC_ID);
				m_h /= 2;

				// Reset slaves
				restoreSlaveStates(m_t, m_iterationStates);
				++m_statConvergenceFailsCounter;
				continue; // try again
			}
		}

		// do we have error control enabled
		if (m_project.m_errorControlMode == Project::EM_NONE)
			break; // done with step

		// this handles the step-doubling and error estimation
		// when this function returns with true, m_realytNext (and the other xxxNext) quantities hold the results
		// after the completed half-step which should be aquivalent to not calling doErrorCheck() at all
		m_timer.start();
		bool success = doErrorCheckRichardson();
		m_statErrorTestTime += m_timer.stop()*1e-3;
		if (success)
			break;

	}


	// transfer computed results at end of iteration
	m_realyt.swap(m_realytNext);
	m_intyt.swap(m_intytNext);
	m_boolyt.swap(m_boolytNext);
	m_stringyt.swap(m_stringytNext);

	// advance current master time
	m_t += m_h;

	// adjust step size
	if (m_enableVariableStepSizes) {
		// When not running with error control mode simply increase the step by some factor
		// This could be made dependend on iteration count...
		if (m_project.m_errorControlMode == Project::EM_NONE) {
			// increase time step for next step
			m_hProposed = std::min(m_project.m_hMax.value, 1.2*m_h);
		}

		// adjust step size to not exceed end time point
		if (m_t + m_hProposed > m_project.m_tEnd.value)
			m_hProposed = m_project.m_tEnd.value - m_t;
		// if we fall just a little short of the end time point, increase time step size a little to hit end time point exactly
		if ( m_t + m_hProposed > m_project.m_tEnd.value*0.999999)
			m_hProposed = m_project.m_tEnd.value - m_t;
	}
	IBK::IBK_Message(IBK::FormatString("step = %1, t = %2, h_next = %3, errFails = %4\n").arg(m_statStepCounter, 5, 'f', 0).arg(m_t).arg(m_hProposed).arg(m_statErrorTestFailsCounter),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
}


void MasterSim::appendOutputs() {
	m_timer.start();
	m_outputWriter.appendOutputs(m_t);
	m_statOutputTime += m_timer.stop()*1e-3;
}


void MasterSim::writeMetrics() const {
	const char * const FUNC_ID = "[MasterSim::writeMetrics]";
	IBK::IBK_Message( IBK::FormatString("\nSolver statistics\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("------------------------------------------------------------------------------\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	double wct = m_outputWriter.m_progressFeedback.m_stopWatch.difference()*1e-3; // in seconds
	// determine suitable unit
	std::string ustr = IBK::Time::suitableTimeUnit(wct);
	IBK::IBK_Message( IBK::FormatString("Wall clock time                            = %1\n").arg(IBK::Time::format_time_difference(wct, ustr, true),13),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("------------------------------------------------------------------------------\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("Output writing                             = %1\n").arg(IBK::Time::format_time_difference(m_statOutputTime, ustr, true),13),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("Master-Algorithm                           = %1    %2\n")
		.arg(IBK::Time::format_time_difference(m_statAlgorithmTime, ustr, true),13).arg(m_statStepCounter,6),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("Convergence failures                       =                  %1\n").arg(m_statConvergenceFailsCounter, 6),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("Error test time and failure count          = %1    %2\n")
		.arg(IBK::Time::format_time_difference(m_statErrorTestTime, ustr, true),13).arg(m_statErrorTestFailsCounter,6),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("------------------------------------------------------------------------------\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

	for (unsigned int i=0; i<m_slaves.size(); ++i) {
		std::stringstream strm;
		strm << std::left << std::setw(30) << m_slaves[i]->m_name;
		IBK::IBK_Message( IBK::FormatString("%1      doStep = %2    %3\n").arg(strm.str())
						  .arg(IBK::Time::format_time_difference(m_statSlaveEvalTimes[i], ustr, true),13)
						  .arg(m_statSlaveEvalCounters[i], 6),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		IBK::IBK_Message( IBK::FormatString("                                  getState = %1    %2\n")
						  .arg(IBK::Time::format_time_difference(m_statStoreStateTimes[i], ustr, true),13)
						  .arg(m_statStoreStateCounters[i], 6),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		IBK::IBK_Message( IBK::FormatString("                                  setState = %1    %2\n")
						  .arg(IBK::Time::format_time_difference(m_statRollBackTimes[i], ustr, true),13)
						  .arg(m_statRollBackCounters[i], 6),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	}
	IBK::IBK_Message( IBK::FormatString("------------------------------------------------------------------------------\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
}



// *** PRIVATE FUNCTIONS ***

void MasterSim::importFMUs() {
	const char * const FUNC_ID = "[MasterSimulator::importFMUs]";
	IBK::Path absoluteProjectFilePath = m_args.m_projectFile.parentPath();
	IBK::IBK_Message("\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message("Importing FMUs\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::MessageIndentor indent; (void)indent;

	// collect list of FMU-Dlls to load dynamically
	std::set<IBK::Path> fmuFiles;
	for (unsigned int i=0; i<m_project.m_simulators.size(); ++i) {
		// Using IBK::Path will ensure that the same files though addressed by different
		// relative paths are converted to the same:
		//   project/fmu/myFmu.fmu   and ../project/fmu/myFmu.fmu
		// are the same files.
		IBK::Path p = m_project.m_simulators[i].m_pathToFMU; // may be a relative path
		if (!p.isAbsolute())
			p =  absoluteProjectFilePath / p;
		fmuFiles.insert(p.absolutePath());
	}

	m_fmuManager.m_unzipFMUs = !m_args.flagEnabled("skip-unzip");
	IBK::Path fmuBaseDir = (m_args.m_workingDir / IBK::Path("fmus")).absolutePath();
	if (!fmuBaseDir.exists() && !IBK::Path::makePath(fmuBaseDir))
		throw IBK::Exception(IBK::FormatString("Error creating fmu extraction base directory: '%1'").arg(fmuBaseDir), FUNC_ID);

	// load FMU library for each fmu
	for (std::set<IBK::Path>::const_iterator it = fmuFiles.begin(); it != fmuFiles.end(); ++it) {
		/// \todo check if user has specified extraction path override in project file
		m_fmuManager.importFMU(fmuBaseDir, *it);
	}

	// NOTE: From now on, the FMU instances in m_fmuManager must not be modified anylonger, since
	//       member variables/memory is treated as persistant during lifetime of FMU slaves
}


void MasterSim::checkCapabilities() {
	const char * const FUNC_ID = "[MasterSimulator::checkCapabilities]";
	// depending on master algorithm, an FMU may be required to have certain capabilities

	// if we have maxIters > 1 and an iterative master algorithm, FMUs must be able to reset states
	m_enableIteration = (m_project.m_masterMode >= Project::MM_GAUSS_SEIDEL && m_project.m_maxIterations > 1);

	// for now, iteration also indicates variable time stepping.
	m_enableVariableStepSizes = m_enableIteration;
	// override this setting if an error control model is used
	if (m_project.m_errorControlMode != Project::EM_NONE && !m_enableIteration) {
		throw IBK::Exception("Using error control requires iterative master algorithm.", FUNC_ID);
	}

	if (m_enableVariableStepSizes) {
		// check each FMU for capability flag
		for (unsigned int i=0; i<m_fmuManager.fmus().size(); ++i) {
			const FMU * fmu = m_fmuManager.fmus()[i];
			if (!fmu->m_modelDescription.m_canHandleVariableCommunicationStepSize)
				throw IBK::Exception(IBK::FormatString("FMU '%1' does not provide canHandleVariableCommunicationStepSize capability, "
													   "which is required by time step adjustment algorithm.").arg(fmu->fmuFilePath()),
									 FUNC_ID);
		}
	}

	if (m_enableIteration) {
		// check each FMU for capability flag
		for (unsigned int i=0; i<m_fmuManager.fmus().size(); ++i) {
			const FMU * fmu = m_fmuManager.fmus()[i];
			if (!fmu->m_modelDescription.m_canGetAndSetFMUstate)
				throw IBK::Exception(IBK::FormatString("FMU '%1' does not provide canGetAndSetFMUState capability, "
													   "which is required by master algorithm.").arg(fmu->fmuFilePath()),
									 FUNC_ID);
		}
	}
}


void MasterSim::instatiateSlaves() {
	const char * const FUNC_ID = "[MasterSimulator::instatiateSlaves]";
	IBK::Path absoluteProjectFilePath = m_args.m_projectFile.parentPath();

	// turn on debug logging for high verbosity levels
	Slave::m_useDebugLogging = m_args.m_verbosityLevel > 3;

	// now that all FMUs have been loaded and their functions/symbols imported, we can instantiate the simulator slaves
	std::set<FMU*>	instantiatedFMUs; // set that holds all instantiated slaves, in case an FMU may only be instantiated once

	IBK::IBK_Message("\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message("Instantiating simulation slaves\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	{
		IBK::MessageIndentor indent; (void)indent;
		for (unsigned int i=0; i<m_project.m_simulators.size(); ++i) {
			const Project::SimulatorDef & slaveDef = m_project.m_simulators[i];
			IBK::IBK_Message( IBK::FormatString("%1 (%2)\n").arg(slaveDef.m_name).arg(slaveDef.m_pathToFMU),
							  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			IBK::Path fmuSlavePath = slaveDef.m_pathToFMU;
			if (!fmuSlavePath.isAbsolute())
				fmuSlavePath = absoluteProjectFilePath / fmuSlavePath;
			// search FMU to instantiate
			FMU * fmu = m_fmuManager.fmuByPath(fmuSlavePath.absolutePath());
			// check if we try to instantiate an FMU twice that forbids this
			if (fmu->m_modelDescription.m_canBeInstantiatedOnlyOncePerProcess) {
				if (instantiatedFMUs.find(fmu) != instantiatedFMUs.end())
					throw IBK::Exception(IBK::FormatString("Simulator '%1' attempts to instantiate FMU '%2' a second time, though this FMU "
										 "may only be instantiated once.").arg(slaveDef.m_name).arg(slaveDef.m_pathToFMU), FUNC_ID);
			}
			// remember that this FMU was instantiated
			instantiatedFMUs.insert(fmu);
			// create new simulation slave
			std::auto_ptr<Slave> slave( new Slave(fmu, slaveDef.m_name) );
			try {
				slave->instantiateSlave();
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, IBK::FormatString("Error setting up slave '%1'").arg(slaveDef.m_name), FUNC_ID);
			}
			// store index of slave in global slaves vector
			slave->m_slaveIndex = m_slaves.size();
			// add slave to vector with slaves
			Slave * s = slave.get();
			m_slaves.push_back(slave.release());

			// insert slave into cycle/priority
			if (m_cycles.size() <= slaveDef.m_cycle)
				m_cycles.resize(slaveDef.m_cycle+1);
			m_cycles[slaveDef.m_cycle].m_slaves.push_back(s);
		}
	}

	IBK::IBK_Message("\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message("Cycles\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::MessageIndentor indent2; (void)indent2;

	for (unsigned int i=0; i<m_cycles.size(); ++i) {
		IBK::IBK_Message( IBK::FormatString("Cycle %1:\n").arg(i+1),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		for (unsigned int s=0; s<m_cycles[i].m_slaves.size(); ++s) {
			const Slave * slave = m_cycles[i].m_slaves[s];
			IBK::IBK_Message( IBK::FormatString("  %1 (%2)\n").arg(slave->m_name).arg(slave->fmu()->fmuFilePath().filename()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		}
	}


	unsigned int nSlaves = m_slaves.size();

	// resize vector for iterative states
	m_iterationStates.resize(nSlaves);
	m_errorCheckStates.resize(nSlaves);
	// resize statistics vectors
	m_statRollBackCounters.resize(nSlaves);
	m_statSlaveEvalCounters.resize(nSlaves);
	m_statSlaveEvalTimes.resize(nSlaves);
	m_statRollBackTimes.resize(nSlaves);
	m_statStoreStateCounters.resize(nSlaves);
	m_statStoreStateTimes.resize(nSlaves);

	// initialize vectors
	for (unsigned int i=0; i<nSlaves; ++i) {
		m_iterationStates[i] = NULL;
		m_errorCheckStates[i] = NULL;
		m_statRollBackCounters[i] = 0;
		m_statSlaveEvalCounters[i] = 0;
		m_statSlaveEvalTimes[i] = 0;
		m_statRollBackTimes[i] = 0;
		m_statStoreStateCounters[i] = 0;
		m_statStoreStateTimes[i] = 0;
	}
}


void MasterSim::initMasterAlgorithm() {
	switch (m_project.m_masterMode) {
		case Project::MM_GAUSS_JACOBI : {
			m_masterAlgorithm = new AlgorithmGaussJacobi(this);
		} break;

		case Project::MM_GAUSS_SEIDEL : {
			m_masterAlgorithm = new AlgorithmGaussSeidel(this);
		} break;

		case Project::MM_NEWTON : {
			m_masterAlgorithm = new AlgorithmNewton(this);
		} break;
	}
	// Algorithm specific initialization
	m_masterAlgorithm->init();
}


void MasterSim::setupDefaultParameters() {
	const char * const FUNC_ID = "[MasterSim::setupDefaultParameters]";
	IBK::IBK_Message("\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message("Setting up default parameters\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::MessageIndentor indent; (void)indent;

	// loop over all slaves
	for (unsigned int s=0; s<m_slaves.size(); ++s) {
		// find corresponding parametrization in project file
		const Slave * slave = m_slaves[s];
		Project::SimulatorDef & simDef = const_cast<Project::SimulatorDef &>(m_project.simulatorDefinition(slave->m_name));
		// check if ResultsRootDir parameter is imported by FMU
		const FMU * fmu = slave->fmu();
		for (unsigned int i=0; i<fmu->m_modelDescription.m_variables.size(); ++i) {
			const FMIVariable & fmiVar = fmu->m_modelDescription.m_variables[i];
			if (fmiVar.m_causality != FMIVariable::C_PARAMETER) continue;

			// first string types
			if (fmiVar.m_type == FMIVariable::VT_STRING) {
				if (fmiVar.m_name == "ResultsRootDir") {
					// check if user-defined parameter has already been specified
					if (simDef.m_parameters.find(fmiVar.m_name) == simDef.m_parameters.end()) {
						simDef.m_parameters[fmiVar.m_name] = (m_args.m_workingDir / "slaves" / slave->m_name).absolutePath().str();
						IBK::Path::makePath( IBK::Path(simDef.m_parameters[fmiVar.m_name]));
						IBK::IBK_Message( IBK::FormatString("%1.ResultsRootDir = %2\n").arg(slave->m_name).arg(simDef.m_parameters[fmiVar.m_name]),
							IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
					}
				}
			} // string vars

		} // variables

	} // slaves
}


void MasterSim::initialConditions() {
	const char * const FUNC_ID = "[MasterSim::initialConditions]";

	IBK::IBK_Message("\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message("Setting up experiment\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	// enter initialization mode
	for (unsigned int i=0; i<m_slaves.size(); ++i) {
		Slave * slave = m_slaves[i];
		slave->setupExperiment(m_project.m_relTol, m_t, m_project.m_tEnd.value);
	}

	IBK::IBK_Message("\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message("Computing initial conditions\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::MessageIndentor indent; (void)indent;

	IBK::IBK_Message("Setting default start values for input variables.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	{
		IBK::MessageIndentor indent3; (void)indent3;
		for (unsigned int i=0; i<m_slaves.size(); ++i) {
			Slave * slave = m_slaves[i];
			IBK::IBK_Message( IBK::FormatString("%1\n").arg(slave->m_name), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
			IBK::MessageIndentor indent2; (void)indent2;
			// process all FMI variables and parameters and set their start values
			for (unsigned int v=0; v<slave->fmu()->m_modelDescription.m_variables.size(); ++v) {
				const FMIVariable & var = slave->fmu()->m_modelDescription.m_variables[v];
				if (var.m_causality != FMIVariable::C_INPUT && var.m_causality != FMIVariable::C_PARAMETER)
					continue;

				// get start value
				const std::string & value= var.m_startValue;
				IBK::IBK_Message(IBK::FormatString("(%1)   %2=%3\n")
								 .arg(FMIVariable::varType2String(var.m_type)).arg(var.m_name).arg(value), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DETAILED);
				// set input value in FMU
				slave->setValue(var, value);
			}
		}
	}


	IBK::IBK_Message("Setting user-defined parameters and start values for input variables.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	{
		IBK::MessageIndentor indent3; (void)indent3;

		// loop over all slaves
		for (unsigned int i=0; i<m_slaves.size(); ++i) {
			Slave * slave = m_slaves[i];

			try {
				// set parameters and start values for all slaves
				const Project::SimulatorDef & simDef = m_project.simulatorDefinition(slave->m_name);
				IBK::IBK_Message(IBK::FormatString("%1\n").arg(slave->m_name), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
				IBK::MessageIndentor indent2; (void)indent2;
				for (std::map<std::string, std::string>::const_iterator it = simDef.m_parameters.begin();
					 it != simDef.m_parameters.end(); ++it)
				{
					const std::string & paraName = it->first;
					const std::string & value = it->second;
					// search for FMI variable with this name
					const FMIVariable & var = slave->fmu()->m_modelDescription.variable(paraName);
					IBK::IBK_Message(IBK::FormatString("(%1)   %2=%3\n")
									 .arg(FMIVariable::varType2String(var.m_type)).arg(paraName).arg(value), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DETAILED);
					// set input value in FMU
					slave->setValue(var, value);
				}
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, IBK::FormatString("Error while setting parameter in slave '%1'").arg(slave->m_name), FUNC_ID);
			}
		}
	}

	IBK::IBK_Message("Entering initialization mode.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	// enter initialization mode
	for (unsigned int i=0; i<m_slaves.size(); ++i) {
		Slave * slave = m_slaves[i];
		slave->enterInitializationMode();
	}

	// if enabled, iterate over initial conditions using the selected master algorithm
	// but disable doStep() and get/set state calls within algorithm

	// Do a Gauss-Jacobi Iteration
	for (unsigned int i=0; i<3; ++i) {

		IBK::IBK_Message(IBK::FormatString("Gauss-Jacobi-Iteration, loop #%1.\n").arg(i+1), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DETAILED);
		for (unsigned int j=0; j<m_realyt.size(); ++j) {
			IBK::IBK_Message(IBK::FormatString("  real[%1] = %2.\n").arg(j).arg(m_realyt[j]), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DETAILED);
		}

		// for now just loop over all slaves retrieve outputs
		for (unsigned int i=0; i<m_slaves.size(); ++i) {
			Slave * slave = m_slaves[i];
			// cache outputs
			slave->cacheOutputs();
			// and sync with global variables vector
			syncSlaveOutputs(slave, m_realyt, m_intyt, m_boolyt, m_stringyt, false);
		}
		// and set inputs


		for (unsigned int i=0; i<m_slaves.size(); ++i) {
			Slave * slave = m_slaves[i];
			updateSlaveInputs(slave, m_realyt, m_intyt, m_boolyt, m_stringyt, false);
		}
	}

	IBK::IBK_Message("Leaving initialization mode.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	// exit initialization mode
	for (unsigned int i=0; i<m_slaves.size(); ++i) {
		Slave * slave = m_slaves[i];
		slave->exitInitializationMode();
	}
}


std::pair<const Slave*, const FMIVariable *> MasterSim::variableByName(const std::string & flatVarName) const {
	const char * const FUNC_ID = "[MasterSim::variableByName]";
	std::vector<std::string> tokens;
	// extract slave name for input
	if (IBK::explode_in2(flatVarName, tokens, '.') != 2)
		throw IBK::Exception(IBK::FormatString("Bad format of variable %1").arg(flatVarName), FUNC_ID);

	std::string slaveName = tokens[0];
	std::string varName = tokens[1];
	const Slave * slave = NULL;
	for (unsigned int s=0; s<m_slaves.size(); ++s) {
		if (m_slaves[s]->m_name == slaveName) {
			slave = m_slaves[s];
			break;
		}
	}
	if (slave == NULL)
		throw IBK::Exception(IBK::FormatString("Unknown/undefined slave name %1").arg(slaveName), FUNC_ID);

	// lookup variable in FMU variable list
	for (unsigned int i=0; i<slave->fmu()->m_modelDescription.m_variables.size(); ++i) {
		const FMIVariable & var = slave->fmu()->m_modelDescription.m_variables[i];
		if (var.m_name == varName) {
			return std::make_pair(slave, &var);
		}
	}
	throw IBK::Exception(IBK::FormatString("Unknown/undefined variable name %1 in FMU %2")
						 .arg(varName).arg(slave->fmu()->fmuFilePath()), FUNC_ID);
}


void MasterSim::composeVariableVector() {
	const char * const FUNC_ID = "[MasterSim::composeVariableVector]";
	// process connection graph and find all slaves and their output variables
	IBK::IBK_Message("\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	IBK::IBK_Message("Resolving connection graph and building variable mapping\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	IBK::MessageIndentor indent; (void)indent;
	for (unsigned int i=0; i<m_project.m_graph.size(); ++i) {
		const Project::GraphEdge & edge = m_project.m_graph[i];
		// resolve variable references
		std::pair<const Slave*, const FMIVariable*> inputVarRef = variableByName(edge.m_inputVariableRef);
		std::pair<const Slave*, const FMIVariable*> outputVarRef = variableByName(edge.m_outputVariableRef);

		// check for consistent types
		FMIVariable::VarType t = inputVarRef.second->m_type;
		if (t != outputVarRef.second->m_type)
			throw IBK::Exception( IBK::FormatString("Mismatching types in connection '%1' -> '%2'").arg(edge.m_outputVariableRef).arg(edge.m_inputVariableRef), FUNC_ID);

		IBK::IBK_Message( IBK::FormatString("%1 \t'%2' -> '%3'    (%4)\n").arg(i+1)
			.arg(edge.m_outputVariableRef).arg(edge.m_inputVariableRef).arg(FMIVariable::varType2String(t)), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);

		VariableMapping varMap;
		varMap.m_globalIndex = i;
		varMap.m_inputSlave = const_cast<Slave*>(inputVarRef.first);
		varMap.m_inputValueReference = inputVarRef.second->m_valueReference;
		varMap.m_outputSlave = const_cast<Slave*>(outputVarRef.first);
		varMap.m_outputLocalIndex = varMap.m_outputSlave->fmu()->localOutputIndex(t, outputVarRef.second->m_valueReference);

		// add variable mapping to corresponding vector
		switch (t) {
			case FMIVariable::VT_BOOL	: m_boolVariableMapping.push_back(varMap); break;
			case FMIVariable::VT_INT	: m_intVariableMapping.push_back(varMap); break;
			case FMIVariable::VT_DOUBLE : m_realVariableMapping.push_back(varMap); break;
			case FMIVariable::VT_STRING : m_stringVariableMapping.push_back(varMap); break;
		}
	}

	// need at least one real variable to compute vector norms
	if (m_realVariableMapping.empty())
		throw IBK::Exception("No real variables in connection graph. This is currently considered an error.", FUNC_ID);

	// resize variable vectors
	m_realyt.resize(m_realVariableMapping.size());
	m_realytNext.resize(m_realVariableMapping.size());
	m_realytNextIter.resize(m_realVariableMapping.size());

	m_intyt.resize(m_intVariableMapping.size());
	m_intytNext.resize(m_intVariableMapping.size());
	m_intytNextIter.resize(m_intVariableMapping.size());

	m_boolyt.resize(m_boolVariableMapping.size());
	m_boolytNext.resize(m_boolVariableMapping.size());
	m_boolytNextIter.resize(m_boolVariableMapping.size());

	m_stringyt.resize(m_stringVariableMapping.size());
	m_stringytNext.resize(m_stringVariableMapping.size());
	m_stringytNextIter.resize(m_stringVariableMapping.size());
}


bool MasterSim::doErrorCheckRichardson() {
	const char * const FUNC_ID = "[MasterSim::doErrorCheckRichardson]";

	// when this function is called, we excpect a converged solution
	//
	// - m_h -> holds size of current step
	// - m_realyt -> holds old states at time t
	// - m_realytNext -> holds states at t + h computed by master algorithm
	//
	//   (same for all other data types)

	// we now copy the real vectors to our temporary location
	unsigned int nValues = m_realyt.size();
	if (m_errRealyt.size() != nValues) {
		// resize required memory on first use
		m_errRealytFirst.resize(nValues);

		m_errRealyt.resize(nValues);
		m_errIntyt.resize(m_intyt.size());
		m_errBoolyt.resize(m_boolyt.size());
		m_errStringyt.resize(m_stringyt.size());
	}
	MasterSim::copyVector(m_realytNext, m_errRealytFirst);

	MasterSim::copyVector(m_realyt, m_errRealyt);
	MasterSim::copyVector(m_intyt, m_errIntyt);
	MasterSim::copyVector(m_boolyt, m_errBoolyt);
	std::copy(m_stringyt.begin(), m_stringyt.end(), m_errStringyt.begin());

	// we will now reset the state of all slaves to be back at time t
	restoreSlaveStates(m_t, m_iterationStates);

	// Note: we do not need to sync states with vectors m_realyt and other m_xxxyt since they are still at
	// time level t.


	// set half step size
	m_h /= 2;

	// save iteration states in vector for error states for later roll-back to time t
	m_errTOriginal = m_t;
	m_errorCheckStates.swap(m_iterationStates);  // no copy here, need a swap because we swap back later

	bool failed = false;
	try {

		// if we have an iterative algorithm, store iteration states
		if (m_enableIteration) {
			storeCurrentSlaveStates(m_iterationStates);
		}

		// now ask master algorithm to do a step
		m_timer.start();
		AbstractAlgorithm::Result res = m_masterAlgorithm->doStep();
		m_statAlgorithmTime += m_timer.stop()*1e-3;
		if (res != AbstractAlgorithm::R_CONVERGED) {
			IBK::IBK_Message("First half-step of error test did not converge.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
			failed = true;
		}
		else {
			// complete step: transfer computed results at end of iteration and advance current master time
			m_realyt.swap(m_realytNext);
			m_intyt.swap(m_intytNext);
			m_boolyt.swap(m_boolytNext);
			m_stringyt.swap(m_stringytNext);
			m_t += m_h;

			// if we have an iterative algorithm, store iteration states
			if (m_enableIteration) {
				storeCurrentSlaveStates(m_iterationStates); // mind: errorStates still holds the states at start of error test step
			}
		}

		// take second step
		if (!failed) {
			m_timer.start();
			res = m_masterAlgorithm->doStep();
			m_statAlgorithmTime += m_timer.stop()*1e-3;
			if (res != AbstractAlgorithm::R_CONVERGED) {
				IBK::IBK_Message("Second half-step of error test did not converge.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
				failed = true;
			}
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Error taking half-steps of error test.", FUNC_ID);
	}

	double err = 2; // initialized with failure
	if (!failed) {
		err = 0;
		// compare computed solutions via WRMS Norm of differences
		for (unsigned int i=0; i<nValues; ++i) {
			double errEstimate = (m_realytNext[i] - m_errRealytFirst[i])/2; // Note: mind the division of 2
			// scale the error by tolerances
			double scaledDiff = errEstimate/(std::fabs(m_realytNext[i])*m_project.m_relTol + m_project.m_absTol);
			// sum up error squared
			err += scaledDiff*scaledDiff;
		}
		err = std::sqrt(err/nValues);
	}

	// if error limit has been exceeded, restore master and slave states to last time point
	IBK::IBK_Message(IBK::FormatString("ERR norm  = %1\n").arg(err, 16, 'f', 3), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DEVELOPER);
	if (err > 1) {
		++m_statErrorTestFailsCounter;
		// failure
		IBK::IBK_Message(IBK::FormatString("Error test failed at t=%1 with h=%2, WRMS=%3.\n")
						 .arg(m_errTOriginal).arg(2*m_h).arg(err), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);

		if (m_project.m_errorControlMode == Project::EM_ADAPT_STEP) {
			// restore state to time t

			// roll back slaves
			restoreSlaveStates(m_t, m_errorCheckStates);
			// restore variables from time t
			MasterSim::copyVector(m_errRealyt, m_realyt);
			MasterSim::copyVector(m_errIntyt, m_intyt);
			MasterSim::copyVector(m_errBoolyt, m_boolyt);
			std::copy(m_errStringyt.begin(), m_errStringyt.end(), m_stringyt.begin());

			// reduce step size, but mind factor two, because error step adaptation is based on current m_h = h/2
			m_h = adaptTimeStepBasedOnErrorEstimate(err)*2;

			m_t = m_errTOriginal;
			// swap back iteration and error states
			m_errorCheckStates.swap(m_iterationStates);  // no copy here!

			return false;
		}
	}


	// slaves are now positioned at t + 2 * h2

	// compute new increased time step proposal, but mind factor two, because error step adaptation is based on current m_h = h/2
	m_hProposed = std::min(adaptTimeStepBasedOnErrorEstimate(err)*2, m_project.m_hMax.value);
	m_errorCheckStates.swap(m_iterationStates);  // no copy here!

	// t is at start of the last half-step,
	// and m_h has the half-step size, so that when we complete the
	// step in the calling routined, it appears as if we just had
	// taken a step from t + h/2 to t + h

	return true;
}


bool MasterSim::doErrorCheckWithoutIteration() {

#if 0
	double diffNorm = 2; // initialized with failure
	if (!failed) {
		diffNorm = 0;
		// compare computed solutions via WRMS Norm of differences
		for (unsigned int i=0; i<nValues; ++i) {
			double errEstimate = (m_realytNext[i] - m_errRealytFirst[i])/(1-2);
			// scale the error by tolerances
			double scaledDiff = errEstimate/(std::fabs(m_realytNext[i])*m_project.m_relTol + m_project.m_absTol);
			// sum up error squared
			diffNorm += scaledDiff*scaledDiff;
		}
		diffNorm = std::sqrt(diffNorm/nValues);
	}

	// if error limit has been exceeded, restore master and slave states to last time point
	if (diffNorm > 1) {
		++m_statErrorTestFailsCounter;
		// failure
		IBK::IBK_Message(IBK::FormatString("Error test failed at t=%1 with h=%2, WRMS=%3.\n")
						 .arg(m_errTOriginal).arg(2*m_h).arg(diffNorm), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);

		if (m_project.m_errorControlMode == Project::EM_ADAPT_STEP) {
			// restore state to time t

			// roll back slaves
			restoreSlaveStates(m_t, m_errorCheckStates);
			// sync slave output caches and variables with m_XXXyt variables
			for (unsigned int s=0; s<m_slaves.size(); ++s) {
				Slave * slave = m_slaves[s];
				slave->cacheOutputs();
				syncSlaveOutputs(slave, m_realyt, m_intyt, m_boolyt, m_stringyt);
			}

			// estimate step size:
			// if step size of m_h lead to an error of diffNorm, we want to know the step size that
			// gives a little less than 1 as diffNorm -> assuming linear relation
			// m_hProposed/1 = m_h/diffNorm;

			diffNorm = std::min(diffNorm, 3.0); // limit reduction factor to 3

			m_h /= std::sqrt(diffNorm);

			return false;
		}
	}
#endif
	return true;
}


double MasterSim::adaptTimeStepBasedOnErrorEstimate(double errEstimate) const {
	double MAX_SCALE = 1.5; // upper limit for scaling up time step
	double MIN_SCALE = 0.3; // lower limit for scaling down time step
	double SAFETY = 0.9; // safety factor
	double scale = std::max(MIN_SCALE, std::min(MAX_SCALE, SAFETY/std::sqrt(errEstimate) ) );
	return m_h * scale;
}


void MasterSim::updateSlaveInputs(Slave * slave, const std::vector<double> & realVariables,
								  const std::vector<int> & intVariables,
								  const std::vector<fmi2Boolean> &boolVariables,
								  const std::vector<std::string> & stringVariables,
								  bool realOnly)
{
	IBK_ASSERT(realVariables.size() == m_realVariableMapping.size());
	IBK_ASSERT(intVariables.size() == m_intVariableMapping.size());
	IBK_ASSERT(boolVariables.size() == m_boolVariableMapping.size());
	IBK_ASSERT(stringVariables.size() == m_stringVariableMapping.size());
	// process all connected variables
	for (unsigned int i=0; i<m_realVariableMapping.size(); ++i) {
		VariableMapping & varMap = m_realVariableMapping[i];
		// skip variables that are not outputs of selected slave
		if (varMap.m_inputSlave != slave) continue;
		// set input in slave
		varMap.m_inputSlave->setReal(varMap.m_inputValueReference, realVariables[i]);
	}
	if (!realOnly) {
		for (unsigned int i=0; i<m_intVariableMapping.size(); ++i) {
			VariableMapping & varMap = m_intVariableMapping[i];
			// skip variables that are not outputs of selected slave
			if (varMap.m_inputSlave != slave) continue;
			// set input in slave
			varMap.m_inputSlave->setInteger(varMap.m_inputValueReference, intVariables[i]);
		}
		for (unsigned int i=0; i<m_boolVariableMapping.size(); ++i) {
			VariableMapping & varMap = m_boolVariableMapping[i];
			// skip variables that are not outputs of selected slave
			if (varMap.m_inputSlave != slave) continue;
			// set input in slave
			varMap.m_inputSlave->setBoolean(varMap.m_inputValueReference, boolVariables[i]);
		}
		for (unsigned int i=0; i<m_stringVariableMapping.size(); ++i) {
			VariableMapping & varMap = m_stringVariableMapping[i];
			// skip variables that are not outputs of selected slave
			if (varMap.m_inputSlave != slave) continue;
			// set input in slave
			varMap.m_inputSlave->setString(varMap.m_inputValueReference, stringVariables[i]);
		}
	}
}


void MasterSim::syncSlaveOutputs(const Slave * slave,
								 std::vector<double> & realVariables,
								 std::vector<int> & intVariables,
								 std::vector<fmi2Boolean> &boolVariables,
								 std::vector<std::string> & stringVariables,
								 bool realOnly)
{
	IBK_ASSERT(realVariables.size() == m_realVariableMapping.size());
	IBK_ASSERT(intVariables.size() == m_intVariableMapping.size());
	IBK_ASSERT(boolVariables.size() == m_boolVariableMapping.size());
	IBK_ASSERT(stringVariables.size() == m_stringVariableMapping.size());
	// process all connected variables
	for (unsigned int i=0; i<m_realVariableMapping.size(); ++i) {
		VariableMapping & varMap = m_realVariableMapping[i];
		// skip variables that are not outputs of selected slave
		if (varMap.m_outputSlave != slave) continue;
		// copy local variable to global array
		realVariables[i] = slave->m_doubleOutputs[varMap.m_outputLocalIndex];
	}
	if (!realOnly) {
		for (unsigned int i=0; i<m_intVariableMapping.size(); ++i) {
			VariableMapping & varMap = m_intVariableMapping[i];
			// skip variables that are not outputs of selected slave
			if (varMap.m_outputSlave != slave) continue;
			// copy local variable to global array
			intVariables[i] = slave->m_intOutputs[varMap.m_outputLocalIndex];
		}
		for (unsigned int i=0; i<m_boolVariableMapping.size(); ++i) {
			VariableMapping & varMap = m_boolVariableMapping[i];
			// skip variables that are not outputs of selected slave
			if (varMap.m_outputSlave != slave) continue;
			// copy local variable to global array
			boolVariables[i] = slave->m_boolOutputs[varMap.m_outputLocalIndex];
		}
		for (unsigned int i=0; i<m_stringVariableMapping.size(); ++i) {
			VariableMapping & varMap = m_stringVariableMapping[i];
			// skip variables that are not outputs of selected slave
			if (varMap.m_outputSlave != slave) continue;
			// copy local variable to global array
			stringVariables[i] = slave->m_stringOutputs[varMap.m_outputLocalIndex];
		}
	}
}


void MasterSim::storeCurrentSlaveStates(std::vector<void *> & slaveStates) {
	IBK::StopWatch w;
	for (unsigned int s=0; s<m_slaves.size(); ++s) {
		Slave * slave = m_slaves[s];
		w.start();
		slave->currentState(&slaveStates[s]);
		m_statStoreStateTimes[slave->m_slaveIndex] += 1e-3*w.stop(); // add elapsed time in seconds
		++m_statStoreStateCounters[slave->m_slaveIndex];
	}
}


void MasterSim::restoreSlaveStates(double t, const std::vector<void*> & slaveStates) {
	IBK::StopWatch w;
	for (unsigned int s=0; s<m_slaves.size(); ++s) {
		Slave * slave = m_slaves[s];
		w.start();
		slave->setState(t, slaveStates[slave->m_slaveIndex]);
		m_statRollBackTimes[slave->m_slaveIndex] += 1e-3*w.stop(); // add elapsed time in seconds
		++m_statRollBackCounters[slave->m_slaveIndex];
	}
}

} // namespace MASTER_SIM

