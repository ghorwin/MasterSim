#include "MSIM_MasterSim.h"

#include <memory> // for auto_ptr

#include <IBK_Exception.h>
#include <IBK_messages.h>
#include <IBK_StringUtils.h>

#include "MSIM_FMU.h"
#include "MSIM_Slave.h"
#include "MSIM_AlgorithmGaussJacobi.h"
#include "MSIM_AlgorithmGaussSeidel.h"

namespace MASTER_SIM {

MasterSim::MasterSim() :
	m_algorithmGaussJacobi(NULL),
	m_algorithmGaussSeidel(NULL),
	m_tCurrent(0)
{
}


MasterSim::~MasterSim() {
	// clean up master algorithms, wether they are used or not
	delete m_algorithmGaussJacobi;
	delete m_algorithmGaussSeidel;

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

	// collect all output and input variables from all slaves, ordered according to cycles
	composeVariableVector();
}



void MasterSim::initialize() {
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
		// if we have error control enabled, store current state

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
	std::vector<std::string> tokens;
	for (unsigned int i=0; i<m_project.m_graph.size(); ++i) {
		const Project::GraphEdge & edge = m_project.m_graph[i];
		// resolve variable references
		std::pair<const Slave*, const FMIVariable*> inputVarRef = variableByName(edge.m_inputVariableRef);
		std::pair<const Slave*, const FMIVariable*> outputVarRef = variableByName(edge.m_outputVariableRef);

	}



	// loop over all cycles
	for (unsigned int c=0; c<m_cycles.size(); ++c) {
		// loop over all slaves in cycle
		for (unsigned int s=0; s<m_cycles[c].m_slaves.size(); ++s) {
			// get all output variables and check if they are connected in the graph


		}
	}
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

}


void MasterSim::syncSlaveOutputs(const Slave * slave, const std::vector<double> & variables) {

}


void MasterSim::storeCurrentSlaveStates(std::vector<void *> & slaveStates) {
	for (unsigned int i=0; i<m_slaves.size(); ++i) {
		slaveStates[i] = m_slaves[i]->currentState();
	}
}


} // namespace MASTER_SIM

