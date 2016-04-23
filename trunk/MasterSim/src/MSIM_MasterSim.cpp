#include "MSIM_MasterSim.h"

namespace MASTER_SIM {

MasterSimulator::MasterSimulator() {
}




void MasterSimulator::instantiateFMUs(const ArgParser &args, const Project & prj) {
	// create copy of input data (needed for multi-threaded application)
	m_args = args;
	m_project = prj;

	// collect list of FMU-Dlls to load dynamically
	std::set<IBK::Path> fmuFiles;
	for (unsigned int i=0; i<prj.m_simulators.size(); ++i) {
		fmuFiles.insert(prj.m_simulators[i].m_pathToFMU);
	}

	//

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

