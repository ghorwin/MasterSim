#include "MSIM_ModelDescription.h"

#include <algorithm>

#include <IBK_Exception.h>
#include <IBK_FormatString.h>
#include <IBK_messages.h>

#include <tinyxml.h>

namespace MASTER_SIM {

ModelDescription::ModelDescription() :
	m_canBeInstantiatedOnlyOncePerProcess(false)
{
}


void ModelDescription::read(const IBK::Path & modelDescriptionFilePath) {
	const char * const FUNC_ID = "[ModelDescription::parseModelDescription]";
	// check if file exists
	if (!modelDescriptionFilePath.exists())
		throw IBK::Exception(IBK::FormatString("Missing file '%1'").arg(modelDescriptionFilePath), FUNC_ID);
	try {
		IBK::MessageIndentor indent; (void)indent;
		IBK::IBK_Message(IBK::FormatString("%1\n").arg(modelDescriptionFilePath), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DETAILED);
		TiXmlDocument doc( modelDescriptionFilePath.c_str() );
		if (!doc.LoadFile()) {
			throw IBK::Exception(IBK::FormatString("Error in line %1 of project file:\n%2")
					.arg(doc.ErrorRow()).arg(doc.ErrorDesc()), FUNC_ID);
		}

		readXMLDoc(doc);
	}
	catch ( IBK::Exception & ex) {
		throw IBK::Exception(ex,  IBK::FormatString("Error parsing modelDescription.xml"), FUNC_ID);
	}
}


void ModelDescription::readXMLDoc(TiXmlDocument & doc) {
	const char * const FUNC_ID = "[ModelDescription::readXMLDoc]";
	try {
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

		// read ModelExchange/CoSimulation parameter sections
		if (version == "1.0") {
			// parse 1.0 specific attributes of fmiModelDescription
			m_modelIdentifier = readRequiredAttribute(xmlElem, "modelIdentifier");

			// CoSim Test
			TiXmlElement * element = xmlRoot.FirstChild("Implementation").ToElement();
			if (element != NULL) {
				// readElementImplementation(element);
				IBK::IBK_Message("CoSimulation, version 1\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
				m_fmuType = CS_v1;
			}
			else {
				IBK::IBK_Message("ModelExchange, version 1\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
				m_fmuType = ME_v1;
			}
		}
		else {
			TiXmlElement * element = xmlRoot.FirstChild("CoSimulation").ToElement();
			IBK::IBK_Message("CoSimulation, version 2\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
			if (element!= NULL) {
				readElementCoSimulation(element);
			}
			else
				throw IBK::Exception("Expected CoSimulation FMU.", FUNC_ID);
		}

		// read model variables section
		TiXmlElement * element = xmlRoot.FirstChild("TypeDefinitions").ToElement();
		if (element != NULL) {
			for (TiXmlElement * child = element->FirstChildElement(); child; child = child->NextSiblingElement()) {
				if (child->ValueStr() == "SimpleType") {
					try {
						FMIType t;
						t.read(child);
						m_typeDefinitions.push_back(t);
					}
					catch (IBK::Exception & ex) {
						throw IBK::Exception(ex, IBK::FormatString("Error reading type definition #%1.").arg(m_typeDefinitions.size()), FUNC_ID);
					}
				}
				else {
					/// \todo Is there anything else but simple type yet?
				}
			}
		}
		// read model variables section
		element = xmlRoot.FirstChild("ModelVariables").ToElement();
		if (element == NULL)
			throw IBK::Exception("Missing ModelVariables tag.", FUNC_ID);
		readElementVariables(element);

	}
	catch ( IBK::Exception & ex) {
		throw IBK::Exception(ex,  IBK::FormatString("Error parsing modelDescription.xml"), FUNC_ID);
	}
	catch ( std::exception & ex) {
		throw IBK::Exception( IBK::FormatString("Error parsing modelDescription.xml: %1").arg(ex.what()), FUNC_ID);
	}
}


const FMIVariable & ModelDescription::variable(const std::string & varName) const {
	const char * const FUNC_ID = "[ModelDescription::variableName]";

//	for (const FMIVariable & fmiVar : m_variables) {
	for (unsigned int i=0; i<m_variables.size(); ++i) {
		const FMIVariable & fmiVar = m_variables[i];
		if (fmiVar.m_name == varName)
			return fmiVar;
	}
	throw IBK::Exception(IBK::FormatString("FMIVariable with name '%1' is not exported.").arg(varName), FUNC_ID);
}


const FMIVariable & ModelDescription::variableByRef(FMIVariable::VarType varType, unsigned int valueReference) const {
	const char * const FUNC_ID = "[ModelDescription::variableByRef]";

	const FMIVariable * outputVar = nullptr;
	const FMIVariable * otherVar = nullptr;

	for (const FMIVariable & fmiVar : m_variables) {
		if (fmiVar.m_type == varType && fmiVar.m_valueReference == valueReference) {
			if (fmiVar.m_causality == FMIVariable::C_OUTPUT)
				outputVar = &fmiVar;
			else
				otherVar = &fmiVar;
		}
	}
	if (outputVar != nullptr)
		return *outputVar;

	if (otherVar != nullptr)
		return *otherVar;

	throw IBK::Exception(IBK::FormatString("FMIVariable with value reference '%1' is not exported.").arg(valueReference), FUNC_ID);
}

/*
const FMIVariable & ModelDescription::variableByRef(unsigned int valueReference) const {
	const char * const FUNC_ID = "[ModelDescription::variableByRef]";

	const FMIVariable * outputVar = nullptr;
	const FMIVariable * otherVar = nullptr;

	for (const FMIVariable & fmiVar : m_variables) {
		if (fmiVar.m_valueReference == valueReference) {
			if (fmiVar.m_causality == FMIVariable::C_OUTPUT)
				outputVar = &fmiVar;
			else
				otherVar = &fmiVar;
		}
	}

	if (outputVar != nullptr)
		return *outputVar;

	if (otherVar != nullptr)
		return *otherVar;

	throw IBK::Exception(IBK::FormatString("FMIVariable with value reference '%1' is not exported.").arg(valueReference), FUNC_ID);
}
*/

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


void ModelDescription::readElementVariables(const TiXmlElement * element) {
	const char * const FUNC_ID ="[ModelDescription::readElementVariables]";
	// loop over all elements in this XML element
	IBK::MessageIndentor indent; (void)indent;
	unsigned int varIdx = 0;
	for (const TiXmlElement * e = element->FirstChildElement(); e; e = e->NextSiblingElement()) {
		// get element name
		std::string name = e->Value();
		if (name != "ScalarVariable")
			throw IBK::Exception( IBK::FormatString("Unknown/unsupported element '%1' within ModelVariables tag.").arg(name), FUNC_ID);
		// construct new variable
		FMIVariable var;
		try {
			var.read(e);
			var.m_varIdx = ++varIdx;
			// skip all variables that are not input, output or parameter
//			if (var.m_causality == FMIVariable::C_OTHER)
//				continue;
			// if no variable has been set, yet, and a declared type is given, try to resolve unit
			if (var.m_unit.empty() && !var.m_declaredType.empty())  {
				std::vector<FMIType>::const_iterator it = std::find(m_typeDefinitions.begin(), m_typeDefinitions.end(),
																	var.m_declaredType);
				if (it != m_typeDefinitions.end())
					var.m_unit = it->m_unit;
			}
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error reading ScalarVariable #%1.").arg(m_variables.size()+1), FUNC_ID);
		}
		m_variables.push_back(var);
	}
}


// *** Static Function Implementations ***

std::string ModelDescription::readRequiredAttribute(const TiXmlElement * xmlElem, const char * attribName) {
	const char * attrib = xmlElem->Attribute(attribName);
	if (attrib == NULL)
		throw IBK::Exception( IBK::FormatString("Expected attribute '%1' in element '%2'.").arg(attribName).arg(xmlElem->ValueStr()), "[ModelDescription::readRequiredAttribute]");
	return std::string(attrib);
}


std::string ModelDescription::readOptionalAttribute(const TiXmlElement * xmlElem, const char * attribName) {
	const char * attrib = xmlElem->Attribute(attribName);
	if (attrib == NULL)
		return std::string();
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
