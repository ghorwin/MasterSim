#ifndef MSIM_FMUSLAVE_H
#define MSIM_FMUSLAVE_H

#include <IBK_Path.h>

namespace MASTER_SIM {

/*! Holds data from unzipped FMU.
	This is not an instance of an FMU.
*/
class FMUObject {
public:
	FMUObject();

	IBK::Path	m_fmuName;
	IBK::Path	m_fmuUnzipDirectory;
};

} // namespace MASTER_SIM


#endif // MSIM_FMUSLAVE_H
