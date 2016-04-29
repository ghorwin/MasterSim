#ifndef MSIM_MODELDESCRIPTION_H
#define MSIM_MODELDESCRIPTION_H

#include <IBK_Path.h>

class TiXmlElement;

namespace MASTER_SIM {

/*! Implements parsing of modelDescription.xml and stores data from the xml file.
*/
class ModelDescription {
public:

	/*! To mask supported FMU types. */
	enum FMUType {
		ME_v1 = 1,
		CS_v1 = 2,
		ME_v2 = 4,
		CS_v2 = 8
	};

	/*! Constructor, initializes variables and parameter lists. */
	ModelDescription();

	/*! Parses model description. */
	void parseModelDescription(const IBK::Path & modelDescriptionFilePath);

	std::string		m_modelName;
	std::string		m_guid;

	/*! Bit mask of FMU types provided in this FMU. */
	FMUType	m_fmuType;

	/*! Model identifier for ModelExchange/CoSimulation v1. */
	std::string	m_modelIdentifier;
	/*! Model identifier for ModelExchange v2. */
	std::string	m_meV2ModelIdentifier;
	/*! Model identifier for CoSimulation v2. */
	std::string	m_csV2ModelIdentifier;

	// CoSim V2 Properties

	bool	m_canHandleVariableCommunicationStepSize;
	bool	m_canInterpolateInputs;
	bool	m_canGetAndSetFMUstate;
	bool	m_canSerializeFMUstate;
	bool	m_canBeInstantiatedOnlyOncePerProcess;
	bool	m_providesDirectionalDerivative;

private:
	/*! Reads CoSimulation element (version 2.0). */
	void readElementCoSimulation(const TiXmlElement * element);

	static std::string readRequiredAttribute(const TiXmlElement *xmlElem, const char *attribName);
	static bool readBoolAttribute(const TiXmlElement *xmlElem, const char *attribName, bool required = false);
};

} // namespace MASTER_SIM

#endif // MSIM_MODELDESCRIPTION_H
