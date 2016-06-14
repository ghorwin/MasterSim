#include "MSIM_FMIVariable.h"

#include <tinyxml.h>

#include <IBK_StringUtils.h>
#include <IBK_Exception.h>
#include <IBK_messages.h>

#include "MSIM_ModelDescription.h"

namespace MASTER_SIM {

void FMIVariable::read(const TiXmlElement * element) {
	const char * const FUNC_ID = "[FMIVariable::read]";
	m_name = ModelDescription::readRequiredAttribute(element, "name");

	// read value reference, description and type
	try {
		m_valueReference = IBK::string2val<unsigned int>(ModelDescription::readRequiredAttribute(element, "valueReference"));
		const char * attrib = element->Attribute("description");
		if (attrib != NULL)
			m_description = std::string(attrib);

		std::string causality = ModelDescription::readOptionalAttribute(element, "causality");
		if (causality == "output")
			m_causality = C_OUTPUT;
		else if (causality == "input")
			m_causality = C_INPUT;
		else if (causality == "parameter") {
			m_causality = C_PARAMETER;
			std::string variability = ModelDescription::readRequiredAttribute(element, "variability");
			/// \todo variability is currently ignored, all variables are treated as continuous or discrete
		}
		else
			m_causality = C_OTHER;

		// read child element
		const TiXmlElement * child = element->FirstChild()->ToElement();
		if (child == NULL)
			throw IBK::Exception("Missing variable type declaration.", FUNC_ID);
		if (child->ValueStr() == "Real") {
			m_type = VT_DOUBLE;
			// try to read unit
			attrib = child->Attribute("unit");
			if (attrib != NULL)
				m_unit = attrib;
			attrib = child->Attribute("declaredType");
			if (attrib != NULL)
				m_declaredType = attrib;
		}
		else if (child->ValueStr() == "Integer") {
			m_type = VT_INT;
		}
		else if (child->ValueStr() == "String") {
			m_type = VT_STRING;
		}
		else if (child->ValueStr() == "Boolean") {
			m_type = VT_BOOL;
		}
		// if we have causality input or parameter, read start element
		if (m_causality == C_INPUT || m_causality == C_PARAMETER) {
			m_startValue = ModelDescription::readRequiredAttribute(child, "start");
		}

		if (m_causality != C_OTHER) {
			IBK::IBK_Message( IBK::FormatString("%1 (%2, %3)\n").arg(m_name).arg(child->ValueStr()).arg(causality), IBK::MSG_PROGRESS,
							  FUNC_ID, IBK::VL_INFO);
		}

	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading definition for variable '%1'").arg(m_name), FUNC_ID);
	}

}


const char *FMIVariable::varType2String(VarType t) {
	switch (t) {
		case VT_BOOL : return "Boolean";
		case VT_INT : return "Integer";
		case VT_DOUBLE : return "Real";
		case VT_STRING : return "String";
		default: return "undefined";
	}
}

} // namespace MASTER_SIM
