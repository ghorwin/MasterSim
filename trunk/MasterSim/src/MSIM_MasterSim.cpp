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



	m_tStepSize = prj.m_tStepStart;
}


void MasterSimulator::initialize() {
	m_tLastOutput = -1;
}


void MasterSimulator::restoreState(double t, const IBK::Path & stateDirectory) {

}


void MasterSimulator::doStep() {

	// step size reduction loop



	// ..

	m_tCurrent += m_tStepSize;
}


void MasterSimulator::writeOutputs() {
	if (m_tLastOutput < 0 || m_tLastOutput + m_tOutputStepMin < m_tCurrent) {
		// outputHandler.write()....
	}
}


} // namespace MASTER_SIM

