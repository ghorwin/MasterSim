#include "MSIM_ErrorControl.h"

namespace MASTER_SIM {

ErrorControl::ErrorControl(MasterSim* master) :
	m_master(master)
{
}

bool ErrorControl::checkLocalError() {

	return true;
}

} // namespace MASTER_SIM
