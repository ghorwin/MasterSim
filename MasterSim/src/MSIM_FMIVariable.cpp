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
		else if (causality == "parameter")
			m_causality = C_PARAMETER;
		else if (causality == "internal")
			m_causality = C_INTERNAL;
		else
			m_causality = C_OTHER;
		m_causalityString = causality;

		m_variability = ModelDescription::readOptionalAttribute(element, "variability");
		if (m_variability.empty())
			m_variability = "continuous";

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
		else if (child->ValueStr() == "Enumeration") {
			m_type = VT_INT; // treat as int
		}
		else {
			throw IBK::Exception(IBK::FormatString("Unknown or unsupported variable type '%1'").arg(child->ValueStr()), FUNC_ID);
		}
		// if we have causality input or parameter, read start values
		if (m_causality == C_INPUT || m_causality == C_PARAMETER) {
			// work-around/special handling for enumerations without start attribute
			if (child->ValueStr() == "Enumeration") {
				try {
					m_startValue = ModelDescription::readRequiredAttribute(child, "start");
				}
				catch (...) {
					// missing start value for enumerations? choose 0 as index
					IBK_FastMessage(IBK::VL_INFO)(IBK::FormatString("Missing 'start' attribute in variable of type '%1' (this is an error in the modelDescription.xml). Defaulting to 0.")
						.arg(child->ValueStr()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_INFO);
					m_startValue = "0";
				}
			}
			else {
				m_startValue = ModelDescription::readRequiredAttribute(child, "start");
			}
		}
		if (m_causality == C_INTERNAL) {
			m_startValue = ModelDescription::readOptionalAttribute(child, "start");
		}

		if (m_causality != C_OTHER) {
			IBK_FastMessage(IBK::VL_INFO)( IBK::FormatString("%1\n").arg(toString()), IBK::MSG_PROGRESS,
							  FUNC_ID, IBK::VL_INFO);
		}

	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading definition for variable '%1'.").arg(m_name), FUNC_ID);
	}

}


std::string FMIVariable::toString() const {
	std::stringstream strm;

	// compose variable info text:
	//   name (type [unit]), causality, variability
	//      "description"

	strm << m_name << " (" << varType2String(m_type);
	if (m_type == VT_DOUBLE) {
		std::string unit = m_unit;
		if (unit.empty())
			unit = "-";
		strm << " [" << unit << "])";
	}
	else
		strm << ")";

	strm << ", " << m_causalityString;
	if (m_causality == C_INPUT || m_causality == C_INTERNAL || m_causality == C_PARAMETER)
		strm << "(start='" << m_startValue << "')";
	strm << ", " << m_variability;

	if (!m_description.empty())
		strm << ", \"" << m_description << "\"";
	return strm.str();
}


const char *FMIVariable::varType2String(VarType t) {
	switch (t) {
		case VT_BOOL : return "Boolean";
		case VT_INT : return "Integer";
		case VT_DOUBLE : return "Real";
		case VT_STRING : return "String";
	}
	return "undefined";
}

} // namespace MASTER_SIM
