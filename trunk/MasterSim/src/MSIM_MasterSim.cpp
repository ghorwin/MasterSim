#include "MSIM_MasterSim.h"

#include <memory> // for auto_ptr

#include <IBK_Exception.h>
#include <IBK_messages.h>
#include <IBK_StringUtils.h>
#include <IBK_assert.h>

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

	// compute initial conditions
	initialConditions();

	// setup time-stepping variables
	m_tStepSize = m_project.m_tStepStart;
	m_tStepSizeProposed = m_tStepSize;
	m_tLastOutput = -1;
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
		writeOutputs();
	}
}



void MasterSim::doStep() {

	// state of master and fmus at this point:
	// - all FMUs and their outputs correspond to master time t
	// -

	m_tStepSize = m_tStepSizeProposed; // set proposed time step size

	// step size reduction loop
	while (true) {
		// if we have error control enabled, store current state of master and all fmus

		// let master do one step


		// do we have error control enabled
		if (m_project.m_errorControlMode == Project::EM_NONE)
			break; // done with step

		if (doErrorCheck())
			break;

		// reduce step size and roll back to
		// ...
	}

	/// \todo Compute new time step to be used in next step.
	///		  Also consider event indicators here.
	m_tStepSizeProposed = m_tStepSize; // no time step adjustment yet!

	// advance current master time
	m_tCurrent += m_tStepSize;
}


void MasterSim::writeOutputs() {
	// skip output writing, if last output was written within minimum output
	// time step size
	if (m_tLastOutput >= 0 && m_tLastOutput + m_project.m_tOutputStepMin > m_tCurrent)
		return;

	// dump state of master to output files

	// 1. state of input/output variables vector
	// 2. statistics of master / counter variables

}


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
	if (m_project.m_masterMode >= Project::MM_GAUSS_SEIDEL && m_project.m_maxIterations > 1) {
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
		// set global index
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
	IBK::IBK_Message("Computing initial conditions\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	// loop over all slaves
	for (unsigned int i=0; i<m_slaves.size(); ++i) {
		Slave * slave = m_slaves[i];

		// set parameters and start values for all slaves

	}

	// enter initialization mode
	for (unsigned int i=0; i<m_slaves.size(); ++i) {
		Slave * slave = m_slaves[i];
		slave->enterInitializationMode();
	}

	// if enabled, iterate over initial conditions using the selected master algorithm
	// but disable doStep() and get/set state calls within algorithm

	// for now just loop over all slaves
	for (unsigned int i=0; i<m_slaves.size(); ++i) {
		Slave * slave = m_slaves[i];
		// cache outputs
		slave->cacheOutputs();
		// and sync with global variables vector
		syncSlaveOutputs(slave, m_realyt); // for now just real variables
	}

	// exit initialization mode
	// enter initialization mode
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
	std::vector<std::string> tokens;
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
			case FMIVariable::VT_BOOL	: break;
			case FMIVariable::VT_INT	: break;
			case FMIVariable::VT_DOUBLE : m_realVariableMapping.push_back(varMap); break;
			case FMIVariable::VT_STRING : break;
		}
	}

	// resize variable vectors
	m_realyt.resize(m_realVariableMapping.size());
	m_realytNext.resize(m_realVariableMapping.size());
	m_realytNextIter.resize(m_realVariableMapping.size());
}


bool MasterSim::doErrorCheck() {
	/// \todo implement
	return true;
}


bool MasterSim::doConvergenceTest() {
	/// \todo implement
	return true;
}


void MasterSim::updateSlaveInputs(Slave * slave, const std::vector<double> & variables) {
	IBK_ASSERT(variables.size() == m_realVariableMapping.size());
	// process all connected variables
	for (unsigned int i=0; i<m_realVariableMapping.size(); ++i) {
		VariableMapping & varMap = m_realVariableMapping[i];
		// skip variables that are not outputs of selected slave
		if (varMap.m_outputSlave != slave) continue;
		// set input in slave
		varMap.m_inputSlave->setReal(varMap.m_inputValueReference, variables[i]);
	}
}


void MasterSim::syncSlaveOutputs(const Slave * slave, std::vector<double> & variables) {
	IBK_ASSERT(variables.size() == m_realVariableMapping.size());
	// process all connected variables
	for (unsigned int i=0; i<m_realVariableMapping.size(); ++i) {
		VariableMapping & varMap = m_realVariableMapping[i];
		// skip variables that are not outputs of selected slave
		if (varMap.m_outputSlave != slave) continue;
		// copy local variable to global array
		variables[i] = slave->m_doubleOutputs[varMap.m_outputLocalIndex];
	}
}


void MasterSim::storeCurrentSlaveStates(std::vector<void *> & slaveStates) {
	for (unsigned int i=0; i<m_slaves.size(); ++i) {
		slaveStates[i] = m_slaves[i]->currentState();
	}
}


} // namespace MASTER_SIM

