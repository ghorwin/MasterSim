#ifndef MSIM_SIMULATOR_H
#define MSIM_SIMULATOR_H

#include <IBK_Path.h>

namespace MASTER_SIM {

/*! Holds all information that define a simulator. */
class Simulator {
public:
	/*! C'tor. */
	Simulator();

	/*! Parses content of simulator definition line. */
	void parse(const std::string & simulatorDef);

	/*! Descriptive name. */
	std::string		m_name;
	/*! Unique ID. */
	unsigned int	m_id;
	/*! Path to FMU file. */
	IBK::Path		m_pathToFMU;
};

} // namespace MASTER_SIM


#endif // MSIM_SIMULATOR_H
