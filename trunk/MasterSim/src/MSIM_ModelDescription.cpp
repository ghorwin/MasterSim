#include "MSIM_ModelDescription.h"

#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

#include <IBK_Exception.h>
#include <IBK_FormatString.h>
#include <IBK_messages.h>

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
		read(modelDescriptionFilePath.str());

		// must be CS v2 -> fix this
		m_csModelIdentifier = getModelIdentifier();
	}
	catch( std::exception & ex) {
		throw IBK::Exception( IBK::FormatString("Error parsing modelDescription.xml: %1").arg(ex.what()), FUNC_ID);
	}

}


} // namespace MASTER_SIM
