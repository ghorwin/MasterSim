#include "MSIM_MasterSim.h"

#include <memory> // for auto_ptr
#include <cmath>
#include <fstream>
#include <algorithm> // for min and max

#include <chrono>
#include <thread>

#include <IBK_Exception.h>
#include <IBK_messages.h>
#include <IBK_StringUtils.h>
#include <IBK_assert.h>
#include <IBK_Time.h>

#include "MSIM_Constants.h"
#include "MSIM_FMU.h"
#include "MSIM_AbstractSlave.h"
#include "MSIM_FMUSlave.h"
#include "MSIM_FileReaderSlave.h"
#include "MSIM_AlgorithmGaussJacobi.h"
#include "MSIM_AlgorithmGaussSeidel.h"
#include "MSIM_AlgorithmNewton.h"

namespace MASTER_SIM {

MasterSim::MasterSim() :
	m_masterAlgorithm(NULL),
	m_t(0),
	m_stepStatsOutput(NULL)
{
}


MasterSim::~MasterSim() {
	// clean up master algorithm
	delete m_masterAlgorithm;

	// release allocated memory of slaves (do not unload shared libraries)
	freeSlaves();

	delete m_stepStatsOutput;
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
	const char * const FUNC_ID = "[MasterSim::initialize]";

	// collect output variable references in all fmus
	for (auto fmuptr : m_fmuManager.fmus())
		fmuptr->collectOutputVariableReferences(m_project.m_writeInternalVariables);

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
	m_outputWriter.m_logDir = (m_args.m_workingDir / "log").absolutePath();
	m_outputWriter.m_projectFile = m_args.m_projectFile.str();
	m_outputWriter.setupProgressReport();

	// setup time-stepping variables
	m_h = m_project.m_hStart.value;
	// special handling: when m_h == 0, automatically compute h from simulation duration
	if (m_h == 0) {
		m_h = (m_project.m_tEnd.value - m_project.m_tStart.value)/1000;
		IBK::IBK_Message(IBK::FormatString("\nSetting h_0 = %1 s\n").arg(m_h), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	}

	m_hProposed = m_h;

	// initialize t_errTOrig = t - hLast
	m_errTOriginal = m_t;

	// setup statistics
	m_statOutputTime = 0;
	m_statAlgorithmTime = 0;
	m_statConvergenceFailsCounter = 0;
	m_statErrorTestFailsCounter = 0;
	m_statErrorTestTime = 0;
	m_statStepCounter = 0;
	m_statAlgorithmCallCounter = 0;

	m_acceptedErrRichardson = 1;
	m_acceptedErrSlopeCheck = 1;
}


void MasterSim::openOutputFiles(bool reopen) {
	m_outputWriter.openOutputFiles(reopen);
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
	// write initial statistics and header
	if (m_args.m_verbosityLevel > 1)
		writeStepStatistics();
	// we need to handle rounding errors:
	// Possibility 1)
	//     m_t                    = 0.99999
	//     m_hProposed            = 0.01
	//     m_project.m_tEnd.value = 1.0
	//     -> Do not take the next step, write final output at m_t = 0.999999 which may be rounded to 1.0 in output
	//
	// Possibility 2)
	//     m_t                    = 0.99000000032
	//     m_hProposed            = 0.001
	//     m_project.m_tEnd.value = 1.0
	//     -> Take step, but adjust m_hProposed to hit exactly tEnd
	while (m_t < m_project.m_tEnd.value) {
		double hRemaining = m_project.m_tEnd.value - m_t;
		if (hRemaining < m_hProposed) {
			if (!m_enableVariableStepSizes && hRemaining < 1e-9) {
				break; // do not exceed end time point, can happen due to rounding errors
			}
#ifdef PREVENT_OVERSHOOTING_AT_SIMULATION_END
			m_hProposed = hRemaining;
#endif //  PREVENT_OVERSHOOTING_AT_SIMULATION_END
		}

		// Do an internal step with the selected master algorithm
		doStep();
		++m_statStepCounter;

		if (m_args.m_verbosityLevel > 1)
			writeStepStatistics();

		// Now the master's internal state has moved to the next time point m_t = m_t + m_h
		// m_h holds the step size used during the last master step

		if (m_t < m_project.m_tEnd.value)
			appendOutputs();// appends outputs (filtering/scheduling is implemented inside OutputWriter class)
	}
	// write final results

	// ensure, that final results are definitely written, but only if the last output time is not already
	// close enough to the final results
	if (m_project.m_tEnd.value - m_outputWriter.m_tLastOutput > 1e-8)
		m_outputWriter.m_tLastOutput = -1;  // this ensures that
	appendOutputs();
}


void MasterSim::doStep() {
	const char * const FUNC_ID = "[MasterSim::doStep]";

	// state of master and fmus at this point:
	// - all FMUs and their outputs correspond to master time t
	// - m_hProposed holds suggested time step size for next step
	// - m_h holds time step size of _last_ completed step

	m_h = m_hProposed; // set proposed time step size

	// ensure that we do not exceed simulation end time point
	if (m_enableVariableStepSizes) {
		// slightly enlarge step if we miss end time only by a fraction
		if (m_t + m_h + 1e-10 > m_project.m_tEnd.value) {
			m_h = m_project.m_tEnd.value - m_t;
			IBK::IBK_Message(IBK::FormatString("Adjusting h='%1' to hit the end time point exactly.\n").arg(m_h),
							 IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		}
	}

	// if we have error control enabled or use iteration in the master algorithm, store current state of all fmu slaves
	if (m_enableIteration || m_useErrorTestWithVariableStepSizes) {
		// request state from all slaves
		storeCurrentSlaveStates(m_iterationStates);
	}

	// step size reduction loop
	while (true) {

		// let master do one step
		m_timer.start();
		AbstractAlgorithm::Result res = m_masterAlgorithm->doStep();
		// states:
		//   m_XXXyt     -> still values at time point t
		//   m_XXXytNext -> values at time point t + h
		++m_statAlgorithmCallCounter;
		m_statAlgorithmTime += m_timer.stop()*1e-3;
		switch (res) {
			case AbstractAlgorithm::R_CONVERGED :
				break;

			case AbstractAlgorithm::R_ITERATION_LIMIT_EXCEEDED :
				if (!m_enableVariableStepSizes) {
					IBK_FastMessage(IBK::VL_INFO)(IBK::FormatString("Step failure at t=%1, taking step size %2. "
														"Reduction of time step is not enabled, continuing with "
														"potentially inaccurate values.")
														 .arg(m_t).arg(m_h),IBK::MSG_WARNING, FUNC_ID, IBK::VL_INFO);
					break;
				}
				/* fall through */

			default : {
				if (m_h/5 < m_project.m_hMin.value)
					throw IBK::Exception(IBK::FormatString("Step failure at t=%1, taking step size %2. "
														   "Reducing step would fall below minimum step size of %3.")
										 .arg(m_t).arg(m_h).arg(m_project.m_hMin.value), FUNC_ID);
				m_h /= 5;

				// Reset slaves
				restoreSlaveStates(m_t, m_iterationStates);
				++m_statConvergenceFailsCounter;
				continue; // try again
			}
		}

		// do we have error control enabled
		if (m_project.m_errorControlMode == Project::EM_NONE)
			break; // done with step

		if (m_project.m_errorControlMode == Project::EM_CHECK) {
			doErrorCheckWithoutIteration(); // no time step adjustment yet
			break; // done with step
		}
		else {

			// this handles the step-doubling and error estimation
			// when this function returns with true, m_realytNext (and the other xxxNext) quantities hold the results
			// after the completed half-step which should be aquivalent to not calling doErrorCheck() at all
			m_timer.start();
			bool success = doErrorCheckRichardson();
			m_statErrorTestTime += m_timer.stop()*1e-3;
			if (success)
				break;
		}
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
			m_hProposed = std::min(m_project.m_hMax.value, 2*m_h);
		}

		// adjust step size to not exceed end time point
		if (m_t + m_hProposed > m_project.m_tEnd.value)
			m_hProposed = m_project.m_tEnd.value - m_t;
		// if we fall just a little short of the end time point, increase time step size a little to hit end time point exactly
		if ( m_t + m_hProposed > m_project.m_tEnd.value*0.999999)
			m_hProposed = m_project.m_tEnd.value - m_t;
	}
	IBK_FastMessage(IBK::VL_DETAILED)(IBK::FormatString("MASTER: step = %1, t = %2, h_next = %3, errFails = %4\n").arg(m_statStepCounter, 5, 'f', 0).arg(m_t).arg(m_hProposed).arg(m_statErrorTestFailsCounter),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DETAILED);
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

	unsigned int maIters, maFMUErrs, maLimitExceeded;
	m_masterAlgorithm->stats(maIters, maLimitExceeded, maFMUErrs);

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
	IBK::IBK_Message( IBK::FormatString("Convergence iteration limit exceeded       =                  %1\n").arg(maLimitExceeded, 6),
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

	// create summary.txt file
	IBK::Path summaryFilePath = m_args.m_workingDir / "log/summary.txt";
	std::ofstream sumFile(summaryFilePath.c_str());
	sumFile << "WallClockTime=" << wct << std::endl;
	sumFile << "FrameworkTimeWriteOutputs=" << m_statOutputTime << std::endl;
	sumFile << "MasterAlgorithmSteps=" << m_statStepCounter << std::endl;
	sumFile << "MasterAlgorithmTime=" << m_statAlgorithmTime << std::endl;
	sumFile << "ConvergenceFails=" << m_statConvergenceFailsCounter << std::endl;
	sumFile << "ConvergenceIterLimitExceeded=" << maLimitExceeded << std::endl;
	sumFile << "ErrorTestFails=" << m_statErrorTestFailsCounter << std::endl;
	sumFile << "ErrorTestTime=" << m_statErrorTestTime << std::endl;
	for (unsigned int i=0; i<m_slaves.size(); ++i) {
		sumFile << "Slave["<<i+1<< "]Time=" << m_statSlaveEvalTimes[i] + m_statStoreStateTimes[i] + m_statRollBackTimes[i] << std::endl;
	}
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

		// only import *.fmu files
		if (IBK::string_nocase_compare(it->extension(), "fmu") )
			m_fmuManager.importFMU(fmuBaseDir, *it);
	}

	// NOTE: From now on, the FMU instances in m_fmuManager must not be modified anylonger, since
	//       member variables/memory is treated as persistant during lifetime of FMU slaves
}


void MasterSim::checkCapabilities() {
	const char * const FUNC_ID = "[MasterSimulator::checkCapabilities]";
	// depending on master algorithm, an FMU may be required to have certain capabilities

	IBK::IBK_Message("\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message("Checking capabilities of FMUs\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::MessageIndentor indent; (void)indent;

	// if we have maxIters > 1 and an iterative master algorithm, FMUs must be able to reset states
	if (m_project.m_maxIterations > 1) {
		IBK::IBK_Message("Iteration enabled because maximum number of iterations > 1.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		m_enableIteration = true;
	}
	else {
		IBK::IBK_Message("Iteration disabled (maximum number of iterations == 1).\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		m_enableIteration = false;
	}

	m_enableVariableStepSizes = m_project.m_adjustStepSize;

	// override this setting if an error control model is used
	if (m_project.m_errorControlMode == Project::EM_STEP_DOUBLING) {
		// must have time step adjustment enabled
		if (!m_enableVariableStepSizes)
			throw IBK::Exception("Using error control with time step adjustment requires time step adjustment flag (adjustStepSize) to be enabled.", FUNC_ID);
		// iteration is enabled, so that FMU state is stored and restored
		m_useErrorTestWithVariableStepSizes = true;
	}
	else {
		m_useErrorTestWithVariableStepSizes = false;
	}

	if (m_enableVariableStepSizes) {
		if (!m_enableIteration && !m_useErrorTestWithVariableStepSizes) {
			throw IBK::Exception("Using variable step sizes without iteration or error control with time step adjustment "
				"is not possible (adjustStepSize flag must be off).", FUNC_ID);
		}
		IBK::IBK_Message("Time step adjustment enabled.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		// check each FMU for capability flag
		for (unsigned int i=0; i<m_fmuManager.fmus().size(); ++i) {
			const FMU * fmu = m_fmuManager.fmus()[i];
			if (!fmu->m_modelDescription.m_canHandleVariableCommunicationStepSize)
				throw IBK::Exception(IBK::FormatString("FMU '%1' does not provide canHandleVariableCommunicationStepSize capability, "
													   "which is required by time step adjustment algorithm.").arg(fmu->fmuFilePath()),
									 FUNC_ID);
		}
	}

	if (m_enableIteration || m_useErrorTestWithVariableStepSizes) {
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
	FMUSlave::m_useDebugLogging = m_args.m_verbosityLevel > 3;

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

			std::unique_ptr<AbstractSlave> slave;

			// is this an fmu we load?
			if (IBK::string_nocase_compare(fmuSlavePath.extension(), "fmu")) {

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
				slave.reset( new FMUSlave(fmu, slaveDef.m_name) );
			}
			else if (IBK::string_nocase_compare(fmuSlavePath.extension(), "tsv") ||
					 IBK::string_nocase_compare(fmuSlavePath.extension(), "csv"))
			{
				// create new file reader slave
				slave.reset( new FileReaderSlave(fmuSlavePath, slaveDef.m_name) );
			}
			else {
				throw IBK::Exception(IBK::FormatString("Unrecognized extension in simulation file path '%1'.").arg(slaveDef.m_pathToFMU),
									 FUNC_ID);
			}

			try {
				slave->instantiate();
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, IBK::FormatString("Error setting up slave '%1'").arg(slaveDef.m_name), FUNC_ID);
			}
			// store index of slave in global slaves vector
			slave->m_slaveIndex = (unsigned int)m_slaves.size();
			// add slave to vector with slaves
			AbstractSlave * s = slave.get();
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
			const AbstractSlave * slave = m_cycles[i].m_slaves[s];
			IBK::IBK_Message( IBK::FormatString("  %1 (%2)\n").arg(slave->m_name).arg(slave->m_filepath.filename()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		}
	}


	unsigned int nSlaves = (unsigned int)m_slaves.size();

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
		const FMUSlave * slave = dynamic_cast<const FMUSlave *>(m_slaves[s]);
		if (slave == nullptr)
			continue;
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
		AbstractSlave * slave = m_slaves[i];
		slave->setupExperiment(m_project.m_relTol, m_t, m_project.m_tEnd.value);
	}

	IBK::IBK_Message("\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message("Computing initial conditions\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::MessageIndentor indent; (void)indent;

	IBK::IBK_Message("Setting default start values for input variables.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	{
		IBK::MessageIndentor indent3; (void)indent3;
		for (unsigned int i=0; i<m_slaves.size(); ++i) {
			FMUSlave * slave = dynamic_cast<FMUSlave *>(m_slaves[i]);
			if (slave == nullptr)
				continue;
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
			FMUSlave * slave = dynamic_cast<FMUSlave *>(m_slaves[i]);
			if (slave == nullptr)
				continue;

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
		AbstractSlave * slave = m_slaves[i];
		slave->enterInitializationMode();
	}

	// if enabled, iterate over initial conditions using the selected master algorithm
	// but disable doStep() and get/set state calls within algorithm

	// Do a Gauss-Jacobi Iteration
	for (unsigned int i=0; i<3; ++i) {

		IBK::IBK_Message(IBK::FormatString("Gauss-Jacobi-Iteration, loop #%1.\n").arg(i+1), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DETAILED);
		for (unsigned int j=0; j<m_realyt.size(); ++j) {
			IBK::IBK_Message(IBK::FormatString("  real[%1] = %2\n").arg(j).arg(m_realyt[j]), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DETAILED);
		}

		// for now just loop over all slaves retrieve outputs
		for (unsigned int i=0; i<m_slaves.size(); ++i) {
			AbstractSlave * slave = m_slaves[i];
			// cache outputs
			slave->cacheOutputs();
			// and sync with global variables vector
			syncSlaveOutputs(slave, m_realyt, m_intyt, m_boolyt, m_stringyt, false);
		}
		// and set inputs


		for (unsigned int i=0; i<m_slaves.size(); ++i) {
			AbstractSlave * slave = m_slaves[i];
			updateSlaveInputs(slave, m_realyt, m_intyt, m_boolyt, m_stringyt, false);
		}
	}

	IBK::IBK_Message("Leaving initialization mode.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	// exit initialization mode
	for (unsigned int i=0; i<m_slaves.size(); ++i) {
		AbstractSlave * slave = m_slaves[i];
		slave->exitInitializationMode();
	}
}


std::pair<const AbstractSlave *, const FMIVariable *> MasterSim::variableByName(const std::string & flatVarName) const {
	const char * const FUNC_ID = "[MasterSim::variableByName]";
	std::vector<std::string> tokens;
	// extract slave name for input
	if (IBK::explode_in2(flatVarName, tokens, '.') != 2)
		throw IBK::Exception(IBK::FormatString("Bad format of variable '%1'").arg(flatVarName), FUNC_ID);

	std::string slaveName = tokens[0];
	std::string varName = tokens[1];
	const AbstractSlave * slave = NULL;
	for (unsigned int s=0; s<m_slaves.size(); ++s) {
		if (m_slaves[s]->m_name == slaveName) {
			slave = m_slaves[s];
			break;
		}
	}
	if (slave == nullptr)
		throw IBK::Exception(IBK::FormatString("Unknown/undefined slave name '%1'").arg(slaveName), FUNC_ID);

	const FMUSlave * fmuSlave = dynamic_cast<const FMUSlave*>(slave);
	if (fmuSlave == nullptr)
		throw IBK::Exception(IBK::FormatString("Slave '%1' is not an FMU-based simulation slave.")
							 .arg(slaveName), FUNC_ID);
	// lookup variable in FMU variable list
	for (unsigned int i=0; i<fmuSlave->fmu()->m_modelDescription.m_variables.size(); ++i) {
		const FMIVariable & var = fmuSlave->fmu()->m_modelDescription.m_variables[i];
		if (var.m_name == varName) {
			return std::make_pair(slave, &var);
		}
	}
	throw IBK::Exception(IBK::FormatString("Unknown/undefined variable name '%1' in FMU '%2'")
						 .arg(varName).arg(slave->m_filepath), FUNC_ID);
}


AbstractSlave * MasterSim::splitFlatVariableName(const std::string & flatVarName, std::string & varname) const {
	const char * const FUNC_ID = "[MasterSim::slavePathByName]";
	std::size_t pos = flatVarName.find(".");
	if (pos == std::string::npos)
		throw IBK::Exception( IBK::FormatString("Invalid variable name '%1', missing slave name prefix.").arg(flatVarName), FUNC_ID);
	std::string slaveName = flatVarName.substr(0, pos);
	// lookup simulator path
	AbstractSlave * slave = NULL;
	for (unsigned int s=0; s<m_slaves.size(); ++s) {
		if (m_slaves[s]->m_name == slaveName) {
			slave = m_slaves[s];
			break;
		}
	}
	if (slave == nullptr)
		throw IBK::Exception(IBK::FormatString("Unknown/undefined slave name '%1', used in variable '%2'").arg(slaveName).arg(flatVarName), FUNC_ID);
	varname = flatVarName.substr(pos+1);
	return slave;
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
		std::pair<const AbstractSlave*, const FMIVariable*> inputVarRef = variableByName(edge.m_inputVariableRef);
		FMIVariable::VarType t = inputVarRef.second->m_type;

		IBK::IBK_Message( IBK::FormatString("%1 \t'%2' -> '%3'    (%4)\n").arg(i+1)
			.arg(edge.m_outputVariableRef).arg(edge.m_inputVariableRef).arg(FMIVariable::varType2String(t)), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);

		VariableMapping varMap;
		varMap.m_globalIndex = i;
		varMap.m_inputSlave = const_cast<AbstractSlave*>(inputVarRef.first);
		varMap.m_inputValueReference = inputVarRef.second->m_valueReference;

		// Variable may be generated by an FMU slave or a file reader slave, distinguish behavior
		// if the fmu file path ends with "fmu" or not.
		// Since we don't know if the output comes from an FMU, we cannot use variableByName(), but must determine the file path first

		std::string varName;
		AbstractSlave * slave = splitFlatVariableName(edge.m_outputVariableRef, varName);
		IBK::Path filePath = slave->m_filepath;
		// if not *.fmu, must be a tsv/csv file
		if (!IBK::string_nocase_compare(filePath.extension(),"fmu")) {

			FileReaderSlave * fileReaderSlave = dynamic_cast<FileReaderSlave *>(slave);
			IBK_ASSERT(fileReaderSlave != nullptr);

			// compose absolute path
			if (!filePath.isAbsolute()) {
				IBK::Path absoluteProjectFilePath = m_args.m_projectFile.parentPath();
				filePath = absoluteProjectFilePath / filePath;
			}

			// search for a variable name
			int colIndex = -1;
			for (unsigned int i=0; i<slave->m_doubleVarNames.size(); ++i) {
				if (varName == fileReaderSlave->m_doubleVarNames[i]) {
					colIndex = i;
					break;
				}
			}
			if (colIndex == -1)
				throw IBK::Exception(IBK::FormatString("Unknown/undefined variable name '%1' in file '%2'").arg(varName).arg(filePath), FUNC_ID);

			// store values
			varMap.m_outputSlave = slave;
			varMap.m_outputLocalIndex = colIndex;
		}
		else {
			std::pair<const AbstractSlave*, const FMIVariable*> outputVarRef = variableByName(edge.m_outputVariableRef);

			// check for consistent types
			if (t != outputVarRef.second->m_type)
				throw IBK::Exception( IBK::FormatString("Mismatching types in connection '%1' -> '%2'").arg(edge.m_outputVariableRef).arg(edge.m_inputVariableRef), FUNC_ID);

			varMap.m_outputSlave = const_cast<AbstractSlave*>(outputVarRef.first);
			FMUSlave * fmuSlave = dynamic_cast<FMUSlave*>(varMap.m_outputSlave);
			varMap.m_outputLocalIndex = fmuSlave->fmu()->localOutputIndex(t, outputVarRef.second->m_valueReference);
		}

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
		IBK::IBK_Message("No real variables in connection graph. This may be an error since FMUs won't communicate with each other.", IBK::MSG_WARNING, FUNC_ID);

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

	// for the error test we resize vectors - for the 3-step history test m_errXXXXyt hold the values t - h_last
	// for the Richardson/Step-Doubling test the values hold the temporary values during evaluation of the half-steps
	m_errRealytFirst.resize(m_realyt.size());
	m_errRealyt.resize(m_realyt.size());
	m_errIntyt.resize(m_intyt.size());
	m_errBoolyt.resize(m_boolyt.size());
	m_errStringyt.resize(m_stringyt.size());
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
	MasterSim::copyVector(m_realytNext, m_errRealytFirst);

	MasterSim::copyVector(m_realyt, m_errRealyt);
	MasterSim::copyVector(m_intyt, m_errIntyt);
	MasterSim::copyVector(m_boolyt, m_errBoolyt);
	std::copy(m_stringyt.begin(), m_stringyt.end(), m_errStringyt.begin());
	// m_errXXXyt variables now hold copy of y(t) values which will be restored when error test is done

	// we will now reset the state of all slaves to be back at time t
	restoreSlaveStates(m_t, m_iterationStates);

	// Note: we do not need to sync states with vectors m_realyt and other m_xxxyt since they are still at
	// time level t.

	// save iteration states in vector for error states for later roll-back to time t
	double hOriginal = m_h;
	m_errTOriginal = m_t;
	m_errorCheckStates.swap(m_iterationStates);  // no copy here, need a swap because we swap back later

	// set half step size
	m_h /= 2;

	bool failed = false;
	try {

		// if we have an iterative algorithm, store iteration states
		if (m_enableIteration) {
			storeCurrentSlaveStates(m_iterationStates);
		}

		// now ask master algorithm to do a step
		m_timer.start();
		AbstractAlgorithm::Result res = m_masterAlgorithm->doStep();
		++m_statAlgorithmCallCounter;
		m_statAlgorithmTime += m_timer.stop()*1e-3;
		if (res != AbstractAlgorithm::R_CONVERGED) {
			IBK_FastMessage(IBK::VL_INFO)("ERROR_TEST: First half-step of error test did not converge.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
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
			++m_statAlgorithmCallCounter;
			m_statAlgorithmTime += m_timer.stop()*1e-3;
			if (res != AbstractAlgorithm::R_CONVERGED) {
				IBK_FastMessage(IBK::VL_INFO)("ERROR_TEST: Second half-step of error test did not converge.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
				failed = true;
			}
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "ERROR_TEST: Error taking half-steps of error test.", FUNC_ID);
	}

	// initialized with failure
	double err = 25; // 0.9/sqrt(25) < 0.2 which is the current minimum for time step reduction
	double errSlopes = 0;

	if (!failed) {
		err = 0;
		errSlopes = 0;
		// compare computed solutions via WRMS Norm of differences
		unsigned int nValues = (unsigned int)m_realytNext.size();
		for (unsigned int i=0; i<nValues; ++i) {

			double errEstimate = (m_realytNext[i] - m_errRealytFirst[i]);
			// scale the error by tolerances
			double scaledDiff = errEstimate/(std::fabs(m_realytNext[i])*m_project.m_relTol + m_project.m_absTol);
			// sum up error squared
			err += scaledDiff*scaledDiff;

			// y(t)     = m_errRealyt
			// y(t+h/2) = m_realyt[i]
			// y(t+h)   = m_errRealytFirst[i]

			double slope_full = (m_errRealytFirst[i] - m_errRealyt[i])/hOriginal;
			double slope_lastHalf = (m_errRealytFirst[i] - m_realyt[i])/m_h;

			// errorSlope = h * (slope(t...t+h) - slope(t...t+h/2))
			double errSlope = m_h * (slope_full - slope_lastHalf);

			// scale the error by tolerances
			scaledDiff = errSlope/(std::fabs(m_realytNext[i])*m_project.m_relTol + m_project.m_absTol);
			// sum up error squared
			errSlopes += scaledDiff*scaledDiff;

		}
		err = std::sqrt(err/nValues);
		errSlopes = std::sqrt(errSlopes/nValues);
		m_acceptedErrRichardson = err;
		m_acceptedErrSlopeCheck = errSlopes;
	}

	// if error limit has been exceeded, restore master and slave states to last time point
	IBK_FastMessage(IBK::VL_DEVELOPER)(IBK::FormatString("ERROR_TEST: ERR norm  = %1, ERR_slope norm  = %2\n")
									   .arg(err, 16, 'f', 3).arg(errSlopes, 16, 'f', 3), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DEVELOPER);

	err = std::max(err, errSlopes);
	if (err > 1) {
		++m_statErrorTestFailsCounter;
		if (failed) {
			IBK_FastMessage(IBK::VL_INFO)(IBK::FormatString("ERROR_TEST: Error test failed at t=%1 with h=%2 due to convergence error.\n")
							 .arg(m_errTOriginal).arg(2*m_h), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		}
		else {
			// failure by error test
			IBK_FastMessage(IBK::VL_INFO)(IBK::FormatString("ERROR_TEST: Error test failed at t=%1 with h=%2, WRMS=%3.\n")
							 .arg(m_errTOriginal).arg(2*m_h).arg(err), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		}
		// reduce step size, but only, if we are not yet at the minimum allowed step size
		if (m_project.m_errorControlMode == Project::EM_STEP_DOUBLING) {
			if (hOriginal > m_project.m_hMin.value) {
				// restore state to time t

				// roll back slaves
				restoreSlaveStates(m_t, m_errorCheckStates);
				// restore variables from time t
				MasterSim::copyVector(m_errRealyt, m_realyt);
				MasterSim::copyVector(m_errIntyt, m_intyt);
				MasterSim::copyVector(m_errBoolyt, m_boolyt);
				std::copy(m_errStringyt.begin(), m_errStringyt.end(), m_stringyt.begin());

				// reduce step size; factor two is already applied within function, because error step adaptation is based on current m_h = h/2
				m_h = adaptTimeStepBasedOnErrorEstimate(err);

				m_t = m_errTOriginal;
				// swap back iteration and error states
				m_errorCheckStates.swap(m_iterationStates);  // no copy here!

				return false; // redo step with smaller step size
			}
			else {
				m_hProposed = hOriginal; // continue with minimum step size
			}
		}
	}
	else {
		// compute new increased time step proposal; factor two is already applied within function, because error step adaptation is based on current m_h = h/2
		m_hProposed = std::min(adaptTimeStepBasedOnErrorEstimate(err), m_project.m_hMax.value);
	}

	// slaves are now positioned at t + 2 * h2

	m_errorCheckStates.swap(m_iterationStates);  // no copy here!

	// t is at start of the last half-step,
	// and m_h has the half-step size, so that when we complete the
	// step in the calling routine, it appears as if we just had
	// taken a step from t + h/2 to t + h

	// Mind: for the step statistics we have to double m_h again, otherwise the
	//       output will show only half-step sizes even though we always completed the full interval

	return true;
}


bool MasterSim::doErrorCheckWithoutIteration() {
	const char * const FUNC_ID = "[MasterSim::doErrorCheckWithoutIteration]";

	// when this function is called, we excpect a converged solution
	//
	// - m_h -> holds size of current step
	// - m_realyt -> holds old states at time t
	// - m_realytNext -> holds states at t + h computed by master algorithm
	// - m_errRealyt -> holds states at t + h_last computed by master algorithm
	//
	//   (same for all other data types)

	// first step? do nothing
	bool res = true;
	if (m_errTOriginal != m_t) {
		// determine difference in slopes between t + h, t - h_last, t + h and t

		double err = 0;
		double errSlopes = 0;

		// compare computed solutions via WRMS Norm of differences
		unsigned int nValues = (unsigned int)m_realytNext.size();
		for (unsigned int i=0; i<nValues; ++i) {

			double errEstimate = (m_realytNext[i] - m_errRealyt[i]);
			// scale the error by tolerances
			double scaledDiff = errEstimate/(std::fabs(m_realytNext[i])*m_project.m_relTol + m_project.m_absTol);
			// sum up error squared
			err += scaledDiff*scaledDiff;

			// y(t)     = m_errRealyt
			// y(t+h/2) = m_realyt[i]
			// y(t+h)   = m_realytNext[i]

			double slope_full = (m_realytNext[i] - m_errRealyt[i])/(m_t + m_h - m_errTOriginal);
			double slope_lastHalf = (m_realytNext[i] - m_realyt[i])/m_h;

			// errorSlope = h * (slope(t...t+h) - slope(t...t+h/2))
			double errSlope = m_h * (slope_full - slope_lastHalf);

			// scale the error by tolerances
			scaledDiff = errSlope/(std::fabs(m_realytNext[i])*m_project.m_relTol + m_project.m_absTol);
			// sum up error squared
			errSlopes += scaledDiff*scaledDiff;
		}
		err = std::sqrt(err/nValues);
		errSlopes = std::sqrt(errSlopes/nValues);

		err = std::max(err, errSlopes);
		if (err > 1) {
			++m_statErrorTestFailsCounter;
			// failure by error test
			IBK_FastMessage(IBK::VL_INFO)(IBK::FormatString("ERROR_TEST: Error test failed at t=%1 with h=%2, WRMS=%3.\n")
							 .arg(m_errTOriginal).arg(2*m_h).arg(err), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
			res = false;
		}
		m_acceptedErrRichardson = err;
		m_acceptedErrSlopeCheck = errSlopes;
	}

	// update state
	copyVector(m_realyt, m_errRealyt);
	return res;
}


double MasterSim::adaptTimeStepBasedOnErrorEstimate(double errEstimate) const {
	const char * const FUNC_ID = "[MasterSim::adaptTimeStepBasedOnErrorEstimate]";
	double MAX_SCALE = 2; // upper limit for scaling up time step
	double MIN_SCALE = 0.2; // lower limit for scaling down time step
	double SAFETY = 0.9; // safety factor
	double scale = std::max(MIN_SCALE, std::min(MAX_SCALE, SAFETY/std::sqrt(errEstimate) ) );
	double hNew = m_h * scale * 2; // use factor two because errEstimate is based on half steps
	// check for falling below time step limit
	if (hNew < m_project.m_hMin.value) {
		IBK_FastMessage(IBK::VL_INFO)(IBK::FormatString("MASTER: Step failure at t=%1, taking step size %2. "
			"Reducing step would fall below minimum step size of %3, continuing with minimum time step size.\n")
			.arg(m_t).arg(m_h).arg(m_project.m_hMin.value), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		hNew = m_project.m_hMin.value;
	}
	return hNew;
}


void MasterSim::updateSlaveInputs(AbstractSlave * slave, const std::vector<double> & realVariables,
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
		// skip variables that are not inputs to selected slave
		if (varMap.m_inputSlave == NULL || varMap.m_inputSlave != slave) continue;
		// set input in slave
		varMap.m_inputSlave->setReal(varMap.m_inputValueReference, realVariables[i]);
	}
	if (!realOnly) {
		for (unsigned int i=0; i<m_intVariableMapping.size(); ++i) {
			VariableMapping & varMap = m_intVariableMapping[i];
			// skip variables that are not inputs to selected slave
			if (varMap.m_inputSlave == NULL || varMap.m_inputSlave != slave) continue;
			// set input in slave
			varMap.m_inputSlave->setInteger(varMap.m_inputValueReference, intVariables[i]);
		}
		for (unsigned int i=0; i<m_boolVariableMapping.size(); ++i) {
			VariableMapping & varMap = m_boolVariableMapping[i];
			// skip variables that are not inputs to selected slave
			if (varMap.m_inputSlave == NULL || varMap.m_inputSlave != slave) continue;
			// set input in slave
			varMap.m_inputSlave->setBoolean(varMap.m_inputValueReference, boolVariables[i]);
		}
		for (unsigned int i=0; i<m_stringVariableMapping.size(); ++i) {
			VariableMapping & varMap = m_stringVariableMapping[i];
			// skip variables that are not inputs to selected slave
			if (varMap.m_inputSlave == NULL || varMap.m_inputSlave != slave) continue;
			// set input in slave
			varMap.m_inputSlave->setString(varMap.m_inputValueReference, stringVariables[i]);
		}
	}
}


void MasterSim::syncSlaveOutputs(const AbstractSlave * slave,
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
		AbstractSlave * slave = m_slaves[s];
		w.start();
		slave->currentState(&slaveStates[s]);
		m_statStoreStateTimes[slave->m_slaveIndex] += 1e-3*w.stop(); // add elapsed time in seconds
		++m_statStoreStateCounters[slave->m_slaveIndex];
	}
}


void MasterSim::restoreSlaveStates(double t, const std::vector<void*> & slaveStates) {
	IBK::StopWatch w;
	for (unsigned int s=0; s<m_slaves.size(); ++s) {
		AbstractSlave * slave = m_slaves[s];
		w.start();
		slave->setState(t, slaveStates[slave->m_slaveIndex]);
		m_statRollBackTimes[slave->m_slaveIndex] += 1e-3*w.stop(); // add elapsed time in seconds
		++m_statRollBackCounters[slave->m_slaveIndex];
	}
}


void MasterSim::writeStepStatistics() {
	// if log file hasn't been created yet, initialize log file now
	if (m_stepStatsOutput == NULL) {
		std::string statsFile = (m_outputWriter.m_logDir / "stepstats.tsv").str();
		m_stepStatsOutput = new std::ofstream(statsFile.c_str());
		std::ostream & out = *m_stepStatsOutput;
		out << std::setw(14) << std::left << "Time [s]" << '\t'
			   << std::setw(10) << std::left << "Steps" << '\t'
			   << std::setw(14) << std::left << "StepSize [s]" << '\t'
			   << std::setw(18) << std::left << "AlgorithmCalls" << '\t'
			   << std::setw(18) << std::left << "ErrorTestFails" << '\t'
			   << std::setw(18) << std::left << "ConvergenceFails" << '\t'
			   << std::setw(12) << std::left << "Iterations" << '\t'
			   << std::setw(18) << std::left << "IterLimitExceeded" << '\t'
			   << std::setw(12) << std::left << "FMUErrors" << '\t'
			   << std::setw(18) << std::left << "ErrorNormRichardson" << '\t'
			   << std::setw(18) << std::left << "ErrorNormSlopeCheck"
			   << std::endl;
		return; // first call only writes header
	}
	std::ostream & out = *m_stepStatsOutput;
	unsigned int maIters, maFMUErrs, maLimitExceeded;
	m_masterAlgorithm->stats(maIters, maLimitExceeded, maFMUErrs);
	double h = m_h;
	if (m_project.m_errorControlMode == Project::EM_STEP_DOUBLING)
		h *= 2;
	out << std::setw(14) << std::left << m_t << '\t'
		   << std::setw(10) << std::left << m_statStepCounter << '\t'
		   << std::setw(14) << std::left << h << '\t'
		   << std::setw(18) << std::left << m_statAlgorithmCallCounter << '\t'
		   << std::setw(18) << std::left << m_statErrorTestFailsCounter << '\t'
		   << std::setw(18) << std::left << m_statConvergenceFailsCounter << '\t'
		   << std::setw(12) << std::left << maIters << '\t'
		   << std::setw(18) << std::left << maLimitExceeded << '\t'
		   << std::setw(12) << std::left << maFMUErrs << '\t'
		   << std::setw(18) << std::left << m_acceptedErrRichardson << '\t'
		   << std::setw(18) << std::left << m_acceptedErrSlopeCheck << '\t'
		   << std::endl;
}


void MasterSim::freeSlaves() {
	// wait a few seconds for the openMP threadpool spin to shut down before deleting the slaves
	// see https://stackoverflow.com/questions/34439956/vc-crash-when-freeing-a-dll-built-with-openmp
	if (!m_slaves.empty()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		for (unsigned int i=0; i<m_slaves.size(); ++i)
			delete m_slaves[i];
		m_slaves.clear();
	}
}

} // namespace MASTER_SIM

