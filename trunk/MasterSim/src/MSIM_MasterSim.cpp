#include "MSIM_MasterSim.h"

#include <memory> // for auto_ptr
#include <cmath>

#include <IBK_Exception.h>
#include <IBK_messages.h>
#include <IBK_StringUtils.h>
#include <IBK_assert.h>
#include <IBK_Time.h>

#include "MSIM_FMU.h"
#include "MSIM_Slave.h"
#include "MSIM_AlgorithmGaussJacobi.h"
#include "MSIM_AlgorithmGaussSeidel.h"
#include "MSIM_AlgorithmNewton.h"

namespace MASTER_SIM {

MasterSim::MasterSim() :
	m_masterAlgorithm(NULL),
	m_tCurrent(0)
{
}


MasterSim::~MasterSim() {
	// clean up master algorithm
	delete m_masterAlgorithm;

	// release allocated memory of slaves
	for (unsigned int i=0; i<m_slaves.size(); ++i)
		delete m_slaves[i];
}


void MasterSim::instantiateFMUs(const ArgParser &args, const Project & prj) {
	//const char * const FUNC_ID = "[MasterSimulator::instantiateFMUs]";

	// create copy of input data (needed for multi-threaded application)
	m_args = args;
	m_project = prj;

	// import all FMUs
	importFMUs();

	// check required capabilities of FMUs, this depends on the selected master algorithm
	checkCapabilities();

	// instantiate all slaves
	instatiateSlaves();
}


void MasterSim::initialize() {
	// collect all output and input variables from all slaves, ordered according to cycles
	composeVariableVector();

	// select master algorithm
	initMasterAlgorithm();

	// compute initial conditions (enter and exit initialization mode)
	initialConditions();

	// setup output writer
	m_outputWriter.m_project = &m_project; // Note: persistant pointer, must be valid for lifetime of output writer
	m_outputWriter.m_slaves = m_slaves; // copy pointer as persistant pointers, must not modify m_slaves vector
	m_outputWriter.m_resultsDir = (m_args.m_workingDir / "results").absolutePath();
	m_outputWriter.m_logDir = (m_args.m_workingDir / "logs").absolutePath();
	m_outputWriter.m_projectFile = m_args.m_projectFile.str();
	m_outputWriter.setupProgressReport();

	// setup time-stepping variables
	m_tStepSize = m_project.m_tStepStart;
	m_tStepSizeProposed = m_tStepSize;

	// setup statistics
	m_statOutputTime = 0;
	m_statAlgorithmTime = 0;
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
	while (m_tCurrent < m_project.m_tEnd) {
		// do an internal step with the selected master algorithm
		// after a successful call to doStep(), the master's internal state has
		// moved to the next time point m_tCurrent
		doStep();

		// handle outputs (filtering/scheduling is implemented inside writeOutputs()).
		if (m_tCurrent < m_project.m_tEnd) {
			m_timer.start();
			appendOutputs();
			m_statOutputTime += m_timer.stop()*1e-3;
		}
	}
	// write final results
	m_outputWriter.m_tLastOutput = -1;  // this ensures that final results are definitely written
	appendOutputs();
}



void MasterSim::doStep() {
	const char * const FUNC_ID = "[MasterSim::doStep]";


	// state of master and fmus at this point:
	// - all FMUs and their outputs correspond to master time t
	// - m_tStepSizeProposed holds suggested time step size for next step
	// - m_tStepSize holds time step size of _last_ completed step

	m_tStepSize = m_tStepSizeProposed; // set proposed time step size

	// step size reduction loop
	while (true) {
		// if we have error control enabled, store current state of master and all fmus


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
										 .arg(m_tCurrent).arg(m_tStepSize), FUNC_ID);
				}
				if (m_tStepSize/2 < m_project.m_tStepMin)
					throw IBK::Exception(IBK::FormatString("Step failure at t=%1, taking step size %2. "
														   "Reducing step would fall below minimum step size of %3.")
										 .arg(m_tCurrent).arg(m_tStepSize).arg(m_project.m_tStepMin), FUNC_ID);
				m_tStepSize /= 2;

				/// \todo Reset slaves
				continue; // try again
			}
		}

		// do we have error control enabled
		if (m_project.m_errorControlMode == Project::EM_NONE)
			break; // done with step

		if (doErrorCheck())
			break;

		// reduce step size and roll back to
		// ...
	}

	// transfer computed results at end of iteration
	m_realyt.swap(m_realytNext);
	m_intyt.swap(m_intytNext);
	m_boolyt.swap(m_boolytNext);
	m_stringyt.swap(m_stringytNext);

	// advance current master time
	m_tCurrent += m_tStepSize;

	// adjust step size
	if (m_enableVariableStepSizes) {
		/// \todo Compute new time step to be used in next step.
		///		  Also consider event indicators here.
		m_tStepSizeProposed = m_tStepSize;

		// adjust step size to not exceed end time point
		if (m_tCurrent + m_tStepSizeProposed > m_project.m_tEnd)
			m_tStepSizeProposed = m_project.m_tEnd - m_tCurrent;
		// if we fall just a little short of the end time point, increase time step size a little to hit end time point exactly
		if ( m_tCurrent + m_tStepSizeProposed > m_project.m_tEnd*0.999999)
			m_tStepSizeProposed = m_project.m_tEnd - m_tCurrent;
	}
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
	IBK::IBK_Message( IBK::FormatString("Output writing                             = %1\n")
		.arg(IBK::Time::format_time_difference(m_statOutputTime, ustr, true),13),
						IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("Time spend in algorithm                    = %1\n")
		.arg(IBK::Time::format_time_difference(m_statAlgorithmTime, ustr, true),13),
						IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("------------------------------------------------------------------------------\n"), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

	for (unsigned int i=0; i<m_slaves.size(); ++i) {
		std::stringstream strm;
		strm << std::left << std::setw(30) << m_slaves[i]->m_name;
		IBK::IBK_Message( IBK::FormatString("%1      doStep = %2    %3\n").arg(strm.str())
						  .arg(IBK::Time::format_time_difference(m_statSlaveEvalTimes[i], ustr, true),13)
						  .arg(m_statSlaveEvalCounters[i], 6),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		IBK::IBK_Message( IBK::FormatString("%1    setState = %2    %3\n").arg(strm.str())
						  .arg(IBK::Time::format_time_difference(m_statSlaveEvalTimes[i], ustr, true),13)
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

	/// \todo Make this is project file setting.
	m_enableVariableStepSizes = (m_project.m_errorControlMode == Project::EM_ADAPT_STEP);

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

	// if we have maxIters > 1 and an iterative master algorithm, FMUs must be able to reset states
	m_enableIteration = (m_project.m_masterMode >= Project::MM_GAUSS_SEIDEL && m_project.m_maxIterations > 1);

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

	// now that all FMUs have been loaded and their functions/symbols imported, we can instantiate the simulator slaves
	std::set<FMU*>	instantiatedFMUs; // set that holds all instantiated slaves, in case an FMU may only be instantiated once

	IBK::IBK_Message("\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message("Instantiating simulation slaves\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
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

	unsigned int nSlaves = m_slaves.size();

	// resize vector for iterative states
	m_iterationStates.resize(nSlaves);
	// resize statistics vectors
	m_statRollBackCounters.resize(nSlaves);
	m_statSlaveEvalCounters.resize(nSlaves);
	m_statSlaveEvalTimes.resize(nSlaves);
	m_statRollBackTimes.resize(nSlaves);

	// initialize vectors
	for (unsigned int i=0; i<nSlaves; ++i) {
		m_iterationStates[i] = NULL;
		m_statRollBackCounters[i] = 0;
		m_statSlaveEvalCounters[i] = 0;
		m_statSlaveEvalTimes[i] = 0;
		m_statRollBackTimes[i] = 0;
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
}


void MasterSim::initialConditions() {
	const char * const FUNC_ID = "[MasterSim::initialConditions]";

	IBK::IBK_Message("\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message("Setting up experiment\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	// enter initialization mode
	for (unsigned int i=0; i<m_slaves.size(); ++i) {
		Slave * slave = m_slaves[i];
		slave->setupExperiment(m_project.m_relTol, m_tCurrent, m_project.m_tEnd);
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


	IBK::IBK_Message("Setting used-defined parameters and start values for input variables.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
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

	// for now just loop over all slaves retrieve outputs
	for (unsigned int i=0; i<m_slaves.size(); ++i) {
		Slave * slave = m_slaves[i];
		// cache outputs
		slave->cacheOutputs();
		// and sync with global variables vector
		syncSlaveOutputs(slave, m_realyt, m_intyt, m_boolyt, m_stringyt);
	}
	// and set inputs

	for (unsigned int i=0; i<m_slaves.size(); ++i) {
		Slave * slave = m_slaves[i];
		updateSlaveInputs(slave, m_realyt, m_intyt, m_boolyt, m_stringyt);
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


bool MasterSim::doErrorCheck() {

	/// \todo implement
	return true;
}


bool MasterSim::doConvergenceTest() {
	const char * const FUNC_ID = "[MasterSim::doConvergenceTest]";

	// compare all state-based values: int, bool and string
	for (unsigned int i=0; i<m_intytNextIter.size(); ++i) {
		if (m_intytNext[i] != m_intytNextIter[i])
			return false;
	}

	for (unsigned int i=0; i<m_boolytNextIter.size(); ++i) {
		if (m_boolytNext[i] != m_boolytNextIter[i])
			return false;
	}

	for (unsigned int i=0; i<m_stringytNextIter.size(); ++i) {
		if (m_stringytNext[i] != m_stringytNextIter[i])
			return false;
	}

	// WRMS norm of real values
	double norm = 0;
	for (unsigned i=0; i<m_realytNextIter.size(); ++i) {
		double diff = m_realytNextIter[i] - m_realytNext[i];
		double absValue = std::fabs(m_realytNextIter[i]);
		double weight = absValue*m_project.m_relTol + m_project.m_absTol;
		diff /= weight;
		norm += diff*diff;
	}

	norm = std::sqrt(norm);
	if (norm > 1) {
		IBK::IBK_Message(IBK::FormatString("WRMS norm = %1 : ").arg(norm, 12, 'f', 0), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DEVELOPER);
		return false;
	}


	return true;
}


void MasterSim::updateSlaveInputs(Slave * slave, const std::vector<double> & realVariables,
								  const std::vector<int> & intVariables,
								  const std::vector<fmi2Boolean> &boolVariables,
								  const std::vector<std::string> & stringVariables)
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


void MasterSim::syncSlaveOutputs(const Slave * slave,
								 std::vector<double> & realVariables,
								 std::vector<int> & intVariables,
								 std::vector<fmi2Boolean> &boolVariables,
								 std::vector<std::string> & stringVariables)
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


void MasterSim::storeCurrentSlaveStates(std::vector<void *> & slaveStates) {
	for (unsigned int i=0; i<m_slaves.size(); ++i) {
		m_slaves[i]->currentState(&slaveStates[i]);
	}
}


} // namespace MASTER_SIM

