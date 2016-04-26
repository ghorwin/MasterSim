#include "MSIM_MasterSim.h"

#include <IBK_Exception.h>

namespace MASTER_SIM {

MasterSimulator::MasterSimulator() :
	m_tCurrent(0)
{
}


void MasterSimulator::instantiateFMUs(const ArgParser &args, const Project & prj) {
	const char * const FUNC_ID = "[MasterSimulator::instantiateFMUs]";

	// create copy of input data (needed for multi-threaded application)
	m_args = args;
	m_project = prj;

	// collect list of FMU-Dlls to load dynamically
	std::set<IBK::Path> fmuFiles;
	for (unsigned int i=0; i<prj.m_simulators.size(); ++i) {
		// Using IBK::Path will ensure that the same files though addressed by different
		// relative paths are converted to the same:
		//   project/fmu/myFmu.fmu   and ../project/fmu/myFmu.fmu
		// are the same files.
		IBK::Path p = prj.m_simulators[i].m_pathToFMU; // may be a relative path
		if (!p.isAbsolute())
			p = args.m_projectFile.parentPath() / p;
		fmuFiles.insert(p.absolutePath());
	}

	m_fmuManager.m_unzipFMUs = !args.flagEnabled("skip-unzip");
	IBK::Path fmuBaseDir = args.m_workingDir / IBK::Path("fmus");
	if (!fmuBaseDir.exists() && !IBK::Path::makePath(fmuBaseDir))
		throw IBK::Exception(IBK::FormatString("Error creating fmu extraction base directory: '%1'").arg(fmuBaseDir), FUNC_ID);

	// load FMU library for each fmu
	for (std::set<IBK::Path>::const_iterator it = fmuFiles.begin(); it != fmuFiles.end(); ++it) {
		/// \todo check if user has specified extraction path override in project file
		m_fmuManager.importFMU(fmuBaseDir, *it);
	}

	m_tStepSize = prj.m_tStepStart;
}


void MasterSimulator::initialize() {
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

	// step size reduction loop



	// ..

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


} // namespace MASTER_SIM

