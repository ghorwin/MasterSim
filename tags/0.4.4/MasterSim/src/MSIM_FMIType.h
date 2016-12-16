#ifndef MSIM_FMITYPE_H
#define MSIM_FMITYPE_H

#include <string>

class TiXmlElement;

namespace MASTER_SIM {

/*! Encapsulates a SimpleType definition in the model description file. */
class FMIType {
public:
	/*! Reads SimpleType element. */
	void read(const TiXmlElement * element);

	/*! Comparison operator to find type definition by declared name. */
	bool operator==(const std::string & name) {
		return m_name == name;
	}

	std::string	m_name;
	std::string	m_unit;
};

} // namespace MASTER_SIM

#endif // MSIM_FMITYPE_H
