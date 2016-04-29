#include "MSIM_ModelDescription.h"

#include <IBK_Exception.h>
#include <IBK_FormatString.h>
#include <IBK_messages.h>

#include <tinyxml.h>

namespace MASTER_SIM {

ModelDescription::ModelDescription()
{
}


void ModelDescription::parseModelDescription(const IBK::Path & modelDescriptionFilePath) {
	const char * const FUNC_ID = "[ModelDescription::parseModelDescription]";
	// check if file exists
	if (!modelDescriptionFilePath.exists())
		throw IBK::Exception(IBK::FormatString("Missing file '%1'").arg(modelDescriptionFilePath), FUNC_ID);
	try {
		IBK::IBK_Message(IBK::FormatString("  %1\n").arg(modelDescriptionFilePath), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		TiXmlDocument doc( modelDescriptionFilePath.c_str() );
		if (!doc.LoadFile()) {
			throw IBK::Exception(IBK::FormatString("Error in line %1 of project file:\n%2")
					.arg(doc.ErrorRow()).arg(doc.ErrorDesc()), FUNC_ID);
		}

		// we use a handle so that NULL pointer checks are done during the query functions
		TiXmlHandle xmlHandleDoc(&doc);

		TiXmlElement * xmlElem = xmlHandleDoc.FirstChildElement().Element();
		if (!xmlElem)
			return; // empty project, this means we are using only defaults
		if (xmlElem->ValueStr() != "fmiModelDescription")
			throw IBK::Exception("Expected fmiModelDescription as root node in XML file.", FUNC_ID);

		m_fmuType = (FMUType)0;

		// retrieve mandatory attributes for all file versions
		std::string version = readRequiredAttribute(xmlElem, "fmiVersion");
		m_modelName = readRequiredAttribute(xmlElem, "modelName");
		m_guid = readRequiredAttribute(xmlElem, "guid");

		// we read our subsections from this handle
		TiXmlHandle xmlRoot = TiXmlHandle((TiXmlNode*)xmlElem);

		if (version == "1.0") {
			// parse 1.0 specific attributes of fmiModelDescription
			m_modelIdentifier = readRequiredAttribute(xmlElem, "modelIdentifier");

			// CoSim Test
			TiXmlElement * element = xmlRoot.FirstChild("Implementation").ToElement();
			if (element != NULL) {
				// readElementImplementation(element);
				m_fmuType = CS_v1;
			}
			else {
				m_fmuType = ME_v1;
			}
		}
		else {
			TiXmlElement * element = xmlRoot.FirstChild("CoSimulation").ToElement();
			if (element!= NULL) {
				readElementCoSimulation(element);
			}
			else
				throw IBK::Exception("Expected CoSimulation FMU.", FUNC_ID);
		}
	}
	catch( std::exception & ex) {
		throw IBK::Exception( IBK::FormatString("Error parsing modelDescription.xml: %1").arg(ex.what()), FUNC_ID);
	}

}


void ModelDescription::readElementCoSimulation(const TiXmlElement * element) {
	m_csV2ModelIdentifier = readRequiredAttribute(element, "modelIdentifier");
	m_canHandleVariableCommunicationStepSize = readBoolAttribute(element, "canHandleVariableCommunicationStepSize");
	m_canInterpolateInputs = readBoolAttribute(element, "canInterpolateInputs");
	m_canGetAndSetFMUstate = readBoolAttribute(element, "canGetAndSetFMUstate");
	m_canSerializeFMUstate = readBoolAttribute(element, "canSerializeFMUstate");
	m_canBeInstantiatedOnlyOncePerProcess = readBoolAttribute(element, "canBeInstantiatedOnlyOncePerProcess");
	m_providesDirectionalDerivative = readBoolAttribute(element, "providesDirectionalDerivative");
	m_fmuType = (FMUType)(m_fmuType | CS_v2);
}

// *** Static Function Implementations ***

std::string ModelDescription::readRequiredAttribute(const TiXmlElement * xmlElem, const char * attribName) {
	const char * attrib = xmlElem->Attribute(attribName);
	if (attrib == NULL)
		throw IBK::Exception( IBK::FormatString("Expected attribute '%1' in element '%2'.").arg(attribName).arg(xmlElem->ValueStr()), "[ModelDescription::readRequiredAttribute]");
	return std::string(attrib);
}

bool ModelDescription::readBoolAttribute(const TiXmlElement *xmlElem, const char *attribName, bool required) {
	const char * const FUNC_ID = "[ModelDescription::readBoolAttribute]";
	const char * attrib = xmlElem->Attribute(attribName);
	if (attrib == NULL) {
		if (required)
			throw IBK::Exception( IBK::FormatString("Expected attribute '%1' in element '%2'.").arg(attribName).arg(xmlElem->ValueStr()), FUNC_ID);
		else
			return false;
	}
	std::string trueValue = attrib;
	if (trueValue == "true")		return true;
	else if (trueValue == "false")	return false;
	else
		throw IBK::Exception( IBK::FormatString("Invalid value to attribute '%1' in element '%2'.").arg(attribName).arg(xmlElem->ValueStr()), FUNC_ID);
}


} // namespace MASTER_SIM
