#ifndef MSIM_MODELDESCRIPTION_H
#define MSIM_MODELDESCRIPTION_H

#include <IBK_Path.h>

//#include <boost/property_tree/ptree.hpp>
#include "ModelDescription.h"

namespace MASTER_SIM {

/*! Implements parsing of modelDescription.xml and stores data from the xml file.
*/
class ModelDescription : public ModelDescriptionAIT {
public:
	/*! Constructor, initializes variables and parameter lists. */
	ModelDescription();

	/*! Parses model description. */
	void parseModelDescription(const IBK::Path & modelDescriptionFilePath);

	/*! Data structure that holds the content of the FMU file. */
//	boost::property_tree::ptree		m_propertyTree;

	/*! Model identifier for CoSimulation. */
	std::string	m_csModelIdentifier;

};

} // namespace MASTER_SIM

#endif // MSIM_MODELDESCRIPTION_H
