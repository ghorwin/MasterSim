#include "MSIM_FMIType.h"

#include <tinyxml.h>

#include <IBK_Exception.h>
#include <IBK_FormatString.h>

#include "MSIM_ModelDescription.h"

namespace MASTER_SIM {


//	Reads a definition of format:
//
//	<SimpleType name="Modelica.SIunits.Temperature">
//		<Real quantity="Temperature" unit="K" min="0" max="373.15" nominal="293.15" />
//
void FMIType::read(const TiXmlElement * element) {
	const char * const FUNC_ID = "[FMIType::read]";
	m_name = ModelDescription::readRequiredAttribute(element, "name");

	try {
		// read child element
		const TiXmlElement * child = element->FirstChild()->ToElement();
		/// \todo should this be an error?
		if (child == NULL)
			return;

		if (child->ValueStr() == "Real") {
			m_unit = ModelDescription::readRequiredAttribute(child, "unit");
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading type definition '%1'").arg(m_name), FUNC_ID);
	}

}

} // namespace MASTER_SIM
