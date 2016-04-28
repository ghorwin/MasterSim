#ifndef MSIM_MODELDESCRIPTION_H
#define MSIM_MODELDESCRIPTION_H

#include <IBK_Path.h>

namespace MASTER_SIM {

/*! Implements parsing of modelDescription.xml and stores data from the xml file.
*/
class ModelDescription {
public:

	/*! To mask supported FMU types. */
	enum FMUType {
		ME_v1 = 0,
		CS_v1 = 1,
		ME_v2 = 2,
		CS_v2 = 4
	};

	/*! Constructor, initializes variables and parameter lists. */
	ModelDescription();

	/*! Parses model description. */
	void parseModelDescription(const IBK::Path & modelDescriptionFilePath);

	/*! Bit mask of FMU types provided in this FMU. */
	FMUType	m_availableTypes;

	/*! Model identifier for ModelExchange v1. */
	std::string	m_meV1ModelIdentifier;
	/*! Model identifier for CoSimulation v1. */
	std::string	m_csV1ModelIdentifier;
	/*! Model identifier for ModelExchange v2. */
	std::string	m_meV2ModelIdentifier;
	/*! Model identifier for CoSimulation v2. */
	std::string	m_csV2ModelIdentifier;

};

} // namespace MASTER_SIM

#endif // MSIM_MODELDESCRIPTION_H
