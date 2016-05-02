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
		m_valueReference = IBK::string2val<int>(ModelDescription::readRequiredAttribute(element, "valueReference"));
		const char * attrib = element->Attribute("description");
		if (attrib != NULL)
			m_description = std::string(attrib);
		std::string variability = ModelDescription::readRequiredAttribute(element, "variability");
		/// \todo variability is currently ignored, all variables are treated as continuous or discrete

		// read child element
		const TiXmlElement * child = element->FirstChild()->ToElement();
		if (child == NULL)
			throw IBK::Exception("Missing variable type declaration.", FUNC_ID);
		if (child->ValueStr() == "Real") {
			m_type = VT_DOUBLE;
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
		std::string causality = ModelDescription::readRequiredAttribute(element, "causality");
		if (causality == "output")
			m_causality = C_OUTPUT;
		else if (causality == "input")
			m_causality = C_INPUT;
		else if (causality == "parameter")
			m_causality = C_PARAMETER;
		else
			m_causality = C_OTHER;

		if (m_causality != C_OTHER) {
			IBK::IBK_Message( IBK::FormatString("%1 (%2, %3)\n").arg(m_name).arg(child->ValueStr()).arg(causality), IBK::MSG_PROGRESS,
							  FUNC_ID, IBK::VL_INFO);
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading definition for variable '%1'").arg(m_name), FUNC_ID);
	}

}

} // namespace MASTER_SIM
