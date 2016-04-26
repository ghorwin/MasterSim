#ifndef MSIM_FMU_H
#define MSIM_FMU_H

#include <IBK_Path.h>

namespace MASTER_SIM {

class FMUPrivate;

/*! Holds all data about an import FMU. */
class FMU {
public:
	FMU();
	~FMU();

	IBK::Path	m_fmuName;
	IBK::Path	m_fmuUnzipDirectory;

private:
	FMUPrivate	*m_impl;
};

} // namespace MASTER_SIM


#endif // MSIM_FMU_H
