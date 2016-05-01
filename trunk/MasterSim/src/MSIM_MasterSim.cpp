#include "MSIM_MasterSim.h"

#include <IBK_Exception.h>
#include <IBK_messages.h>

#include "MSIM_FMU.h"
#include "MSIM_Slave.h"

namespace MASTER_SIM {

MasterSimulator::MasterSimulator() :
	m_tCurrent(0)
{
}


MasterSimulator::~MasterSimulator() {
	// release allocated memory of slaves
	for (unsigned int i=0; i<m_slaves.size(); ++i)
		delete m_slaves[i];
}


void MasterSimulator::instantiateFMUs(const ArgParser &args, const Project & prj) {
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



void MasterSimulator::initialize() {
	m_tStepSize = m_project.m_tStepStart;
	m_tLastOutput = -1;
}


void MasterSimulator::restoreState(double t, const IBK::Path & stateDirectory) {
	// all FMUs must be able to restore state!

	// for all FMU instances:
	// - do normal FMU init
	// - compute FMU-specific state file path
	// - read serialization file and extract meta header
	// - tell FMU to de-serialize state

	// update state of master simulator
}


void MasterSimulator::storeState(const IBK::Path & stateDirectory) {
	// all FMUs must be able to store state!

	// for all FMU instances:
	// - compute FMU-specific state file path
	// - ask for serialization size and create dummy vector
	// - ask for FMU state and then serialize content of FMU
	// - dump state into file with meta-data header

	// dump state of master simulator
}


void MasterSimulator::simulate() {
	while (m_tCurrent < m_project.m_tEnd) {
		// do an internal step with the selected master algorithm
		// after a successful call to doStep(), the master's internal state has
		// moved to the next time point m_tCurrent
		doStep();

		// handle outputs (filtering/scheduling is implemented inside writeOutputs()).
		writeOutputs();
	}
}



void MasterSimulator::doStep() {

	// state of master and fmus at this point:
	// - all FMUs and their outputs correspond to master time t
	// -

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

	m_tCurrent += m_tStepSize;
}


void MasterSimulator::writeOutputs() {
	// skip output writing, if last output was written within minimum output
	// time step size
	if (m_tLastOutput >= 0 && m_tLastOutput + m_project.m_tOutputStepMin > m_tCurrent)
		return;

	// dump state of master to output files

	// 1. state of input/output variables vector
	// 2. statistics of master / counter variables

}


void MasterSimulator::importFMUs() {
	const char * const FUNC_ID = "[MasterSimulator::importFMUs]";
	IBK::Path absoluteProjectFilePath = m_args.m_projectFile.parentPath();
	IBK::IBK_Message("Importing FMUs.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
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


void MasterSimulator::checkCapabilities() {
	const char * const FUNC_ID = "[MasterSimulator::checkCapabilities]";
	// depending on master algorithm, an FMU may be required to have certain capabilities

	if (m_project.m_masterMode >= Project::MM_GAUSS_SEIDEL_ITERATIVE) {
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


void MasterSimulator::instatiateSlaves() {
	const char * const FUNC_ID = "[MasterSimulator::instatiateSlaves]";
	IBK::Path absoluteProjectFilePath = m_args.m_projectFile.parentPath();

	// now that all FMUs have been loaded and their functions/symbols imported, we can instantiate the simulator slaves
	std::set<FMU*>	instantiatedFMUs; // set that holds all instantiated slaves, in case an FMU may only be instantiated once

	IBK::IBK_Message("Instantiation simulation slaves.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::MessageIndentor indent; (void)indent;
	for (unsigned int i=0; i<m_project.m_simulators.size(); ++i) {
		const Project::SimulatorDef & slaveDef = m_project.m_simulators[i];
		IBK::IBK_Message( IBK::FormatString("%1 (fmu %2)\n").arg(slaveDef.m_name).arg(slaveDef.m_pathToFMU),
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
		// create new simulation slave
		instantiatedFMUs.insert(fmu);
	}
}


bool MasterSimulator::doErrorCheck() {
	/// \todo implement
	return true;
}


} // namespace MASTER_SIM

