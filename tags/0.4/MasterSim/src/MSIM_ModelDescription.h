#ifndef MSIM_MODELDESCRIPTION_H
#define MSIM_MODELDESCRIPTION_H

#include <IBK_Path.h>

#include "MSIM_FMIVariable.h"
#include "MSIM_FMIType.h"

class TiXmlElement;
class TiXmlDocument;

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
	void read(const IBK::Path & modelDescriptionFilePath);

	/*! Parses model description from an existing TiXmlDocument. */
	void readXMLDoc(TiXmlDocument &doc);

	/*! Returns a variable identified by name. */
	const FMIVariable & variable(const std::string & varName) const;

	/*! Returns a variable identified by value reference. */
	const FMIVariable & variableByRef(unsigned int valueReference) const;

	std::string		m_modelName;
	std::string		m_guid;

	/*! Bit mask of FMU types provided in this FMU (with FMI v2 there can be both ModelExchange and CoSimulation). */
	FMUType			m_fmuType;

	/*! Model identifier for ModelExchange/CoSimulation v1. */
	std::string		m_modelIdentifier;
	/*! Model identifier for ModelExchange v2. */
	std::string		m_meV2ModelIdentifier;
	/*! Model identifier for CoSimulation v2. */
	std::string		m_csV2ModelIdentifier;

	// CoSim Properties

	bool	m_canHandleVariableCommunicationStepSize;
	bool	m_canInterpolateInputs;
	bool	m_canGetAndSetFMUstate;
	bool	m_canSerializeFMUstate;
	bool	m_canBeInstantiatedOnlyOncePerProcess;
	bool	m_providesDirectionalDerivative;

	/*! Vector of SimpleType definitions. */
	std::vector<FMIType>		m_typeDefinitions;

	/*! Vector of variables published by this FMU.
		The ModelVariable index is the index of the variables in this vector + 1.
	*/
	std::vector<FMIVariable>	m_variables;

	/*! Reads an attribute by name from an xml tag. */
	static std::string readRequiredAttribute(const TiXmlElement *xmlElem, const char *attribName);
	/*! Reads an optional attribute by name from an xml tag (if missing, an emptry string is returned). */
	static std::string readOptionalAttribute(const TiXmlElement *xmlElem, const char *attribName);
	/*! Reads a boolean attribute by name from an xml tag and returns its value.
		Function returns false if value is optional (required = false) and not present.
	*/
	static bool readBoolAttribute(const TiXmlElement *xmlElem, const char *attribName, bool required = false);

private:
	/*! Reads CoSimulation element (version 2.0). */
	void readElementCoSimulation(const TiXmlElement * element);
	/*! Reads variables section. */
	void readElementVariables(const TiXmlElement * element);

};

} // namespace MASTER_SIM

#endif // MSIM_MODELDESCRIPTION_H
