#include "IBK_Quantity.h"

#include <iostream>
#include <iomanip>
#include <stdexcept>

#include "IBK_Exception.h"
#include "IBK_FormatString.h"

namespace IBK {

Quantity::Quantity(type_t t, const std::string & name) :
	m_name(name),
	m_type(t)
{
}


Quantity::Quantity(Quantity::type_t t, const std::string & name,
				   const std::string & desc,
				   const IBK::Unit & u,
				   const IBK::Color & color) :
	m_name(name),
	m_type(t),
	m_description(desc),
	m_unit(u),
	m_color(color)
{
}


void Quantity::clear() {
	m_type = NUM_TYPES;
	m_name.clear();
	m_description.clear();
	m_unit.clear();
	m_color = IBK::Color();
}


void Quantity::read(const std::string & line) {
	const char * const FUNC_ID = "[QuantityData::read]";

	// parsing line in format: STATE Temperature [C]   'Temperature'  {#ffffff}

	// find description tokens
	std::string::size_type posDescStart = line.find('\'');
	std::string::size_type posDescEnd = line.rfind('\'');
	if (posDescStart != std::string::npos) {
		if (posDescEnd != std::string::npos && posDescEnd != posDescStart) {
			if (posDescStart+1 < posDescEnd)
				m_description = line.substr(posDescStart+1, posDescEnd-posDescStart-1);
		}
		else {
			throw IBK::Exception(IBK::FormatString("Mismatching format in line '%1', only found a single \'.").arg(line), FUNC_ID);
		}
	}

	// find description tokens
	posDescStart = line.find("{#");
	posDescEnd = line.rfind('}');
	if (posDescStart != std::string::npos) {
		if (posDescEnd != std::string::npos && posDescEnd != posDescStart) {
			if (posDescStart+1 < posDescEnd) {
				std::string colName = line.substr(posDescStart+1, posDescEnd-posDescStart-1);
				try {
					m_color = IBK::Color::fromHtml(colName);
				}
				catch (IBK::Exception & ex) {
					throw IBK::Exception(ex, IBK::FormatString("Mismatching color entry in line '%1', expected format '{0xrrggbb}'.").arg(line), FUNC_ID);
				}
			}
		}
		else {
			throw IBK::Exception(IBK::FormatString("Mismatching format in line '%1', only found opening brace {, but not closing brace }.").arg(line), FUNC_ID);
		}
	}


	// find unit tokens
	std::string::size_type posUnitStart = line.find('[');
	std::string::size_type posUnitEnd = line.rfind(']', posDescStart);
	if (posUnitStart == std::string::npos ||
		posUnitEnd == std::string::npos ||
		posUnitStart + 1 == posUnitEnd)
	{
		throw IBK::Exception("Missing or invalid default unit.", FUNC_ID);
	}
	try {
		m_unit = IBK::Unit(line.substr(posUnitStart+1, posUnitEnd-posUnitStart-1));
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Mismatching format in line '%1', expected value unit ID name.").arg(line), FUNC_ID);
	}

	// read type and quantity name tokens
	std::stringstream strm(line.substr(0, posUnitStart));
	std::string typestr;
	if (!(strm >> typestr >> m_name)) {
		throw IBK::Exception("Mismatching format, expected type keyword followed by identification name.", FUNC_ID);
	}
	// try to convert type keyword into type
	try {
		m_type = Quantity::string2type(typestr); // might throw
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Mismatching format in line '%1', expected valid type ID name.").arg(line), FUNC_ID);
	}
}


void Quantity::write(std::ostream & out, int indent) {
	std::string istr(indent, ' ');
	out << istr << std::setw(20) << std::left << Quantity::type2string(m_type)
				<< std::setw(40) << std::left << m_name
				<< std::setw(10) << std::left << ("[" + m_unit.name() + "]");
	if (!m_description.empty()) {
		out << "'" << m_description << "'";
	}
	if (m_color != IBK::Color()) {
		out << "{" << m_color.toHtmlString() << "}";
	}
	out << std::endl;
}



const char *  Quantity::type2string(Quantity::type_t t) {
	switch (t) {
		case STATE		: return "STATE";
		case FLUX		: return "FLUX";
		case BCFLUX		: return "BCFLUX";
		case SOURCE		: return "SOURCE";
		case INTERFACE	: return "INTERFACE";
		case LOAD		: return "LOAD";
		case NUM_TYPES  :
		default			: ;
	}
	throw IBK::Exception( IBK::FormatString("Invalid quantity type index %1.").arg((int)t),
						 "[Quantity::type2string]" );
}


Quantity::type_t Quantity::string2type(const std::string & typestr) {
	for (unsigned int i=0; i<NUM_TYPES; ++i)
		if (type2string((Quantity::type_t)i) == typestr)
			return (Quantity::type_t)i;
	throw IBK::Exception( IBK::FormatString("Cannot convert quantity string '%1' to type.")
						 .arg(typestr),
						 "[Quantity::string2type]"  );
}


} // namespace IBK

