#ifndef IBK_QuantityH
#define IBK_QuantityH

#include <string>
#include <vector>

#include "IBK_Unit.h"
#include "IBK_Color.h"

namespace IBK {

/*!	The class Quantity describes a quantity, uniquely identified by its ID name.
	\todo Refactor to have a generic description of a quantity, where type is identified by
		  a generic string, and not a pre-defined Delphin-specific type string.
*/
class Quantity {
public:
	/*! Different types of quantities, recognized by this class. */
	enum type_t {
		STATE,               // Keyword: STATE       'State variable or related quantity'
		FLUX,                // Keyword: FLUX        'Flux between elements'
		BCFLUX,              // Keyword: BCFLUX      'Boundary flux'
		SOURCE,              // Keyword: SOURCE      'Element-based source/sink'
		INTERFACE,           // Keyword: INTERFACE   'Loads on a specific interface (boundary flux inputs)'
		LOAD,                // Keyword: LOAD        'Values of time-value data tables (e.g. climatic data)'
		NUM_TYPES
	};

	/*! Default constructor, creates an undefined quantity. */
	Quantity() : m_type(NUM_TYPES) {}
	/*! Initializing constructor. */
	Quantity(type_t t, const std::string & name);
	/*! Initializing constructor. */
	Quantity(Quantity::type_t t, const std::string & name, const std::string & desc, const IBK::Unit & u, const IBK::Color & color = IBK::Color());

	/*! Resets the member variables. */
	void clear();
	/*! Reads quantity from string (single-line format).
		\code
		// line to parse has format
		STATE Temperature [C]   'Temperature'  {#ff0000}
		// where [C] is the default IO Unit, and {#ff0000} is the hex color code
		\endcode
	*/
	void read(const std::string & line);
	/*! Writes quantity to string (single-line format) and ends line with line break. */
	void write(std::ostream & out, int indent);

	/*! The unique identification string. */
	std::string			m_name;
	/*! The kind of quantity, one from type_t. */
	type_t				m_type;

	/*! Description of the quantity. */
	std::string			m_description;
	/*! Default IO Unit. */
	IBK::Unit			m_unit;
	/*! Color for representation in UI. */
	IBK::Color			m_color;

	/*! Converts enumeration type to corresponding string. */
	static const char * type2string(type_t t);
	/*! Converts type identification string to corresponding enumeration type. */
	static type_t string2type(const std::string & typestr);
};

/*! Comparison operator for Quantities. */
inline bool operator<(const Quantity & lhs, const Quantity & rhs) {
	return (lhs.m_name < rhs.m_name);
}

} // namespace IBK

/*! \file IBK_Quantity.h
	\brief Contains the declaration of class Quantity.
*/

#endif // IBK_QuantityH

