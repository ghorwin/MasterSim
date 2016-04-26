#include "MSIM_FMU.h"

namespace MASTER_SIM {

/*! Implementation class for the FMU interface, hides all details about
	loading and interfacing an FMU.
*/
class FMUPrivate {

};

// ------------------------------------------------------------------------

FMU::FMU() : m_impl(new FMUPrivate)
{
}

FMU::~FMU() {
	delete m_impl;
}


} // namespace MASTER_SIM
