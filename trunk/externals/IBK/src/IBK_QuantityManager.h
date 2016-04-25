#ifndef IBK_QuantityManagerH
#define IBK_QuantityManagerH

#include <string>
#include <vector>
#include <iosfwd>
#include <map>
#include <set>

#include "IBK_Quantity.h"
#include "IBK_Path.h"

namespace IBK {

/*!	The class QuantityManager manages definitions of physical quantities.

	A list of known quantities can be read from file/string similarly to the unit list
	and then the properties of the different quantities can be retrieved.
	The Quantity manager provides conversion functionality between Quantities ID strings,
	and enumeration values of a defined subset of quantities.

	A quantity is uniquely identified through its type and ID-name.

	The QuantityManager combines the functionality of a manager for all quantities and
	for a subset of quantities actually used in any application.
	\code
	// define quantities (can be done in a text file, descriptions are optional)
	std::string quantities =
		"STATE Moisture mass density [kg/m3] 'Total mass of liquid water and water vapor'\n"
		"STATE Temperature           [C]     'Some temperature'\n"
		"STATE Pressure              [Pa]\n"
		"FLUX HeatFluxDensity        [W/m2]\n";
	// populate quantity manager with quantities
	QuantityManager quantMan;
	quantMan.read(quantities);
	\endcode
*/
class QuantityManager {
public:
	/*! Constructor. */
	QuantityManager();

	/*! Reads list of quantities from string (newline separated entries).
		Previously existing quantities are replaced.
	*/
	void read(const std::string & data);

	/*! Writes list of quantities to output stream. */
	void write(std::ostream & out);

	/*! Convenience function, opens file and reads quantities from file. */
	void readFromFile(const IBK::Path & fname);

	/*! Convenience function, opens file for writing and writes quantities to file. */
	void writeToFile(const IBK::Path & fname);

	/*! Resets the member variables. */
	void clear();

	/*! Returns the index of the quantity in the global list of quantities.
		Indicies are invalidated with each call to read() or clear().
		\return Returns an index if quantity name was found, otherwise -1.
	*/
	int index(const std::string & quantityName) const;

	/*! Returns the index of the quantity in the global list of quantities.
		Indicies are invalidated with each call to read() or clear().
		\return Returns an index if quantity was found, otherwise -1.
	*/
	int index(const Quantity & quantity) const {
		return index(quantity.m_name);
	}

	/*! Returns the quantity definition data for the global index.
		Throws an exception when quantity index is invalid.
	*/
	const Quantity & quantity(unsigned int globalIndex) const;

	/*! Returns the quantity definition data for the unique ID name.
		Throws an exception when such a quantity is not defined.
	*/
	const Quantity & quantity(const std::string & quantityName) const {
		return quantity(index(quantityName));
	}

	/*! Generates set with subset of quantities that all belong to requested type. */
	std::set<Quantity> quantitiesOfType(IBK::Quantity::type_t t) const;

	/*! Gives read-only access to list of quantities. */
	const std::vector<Quantity> & quantities() const { return m_quantities; }

private:
	/*! Holds all quantities (optimized for index access using global index). */
	std::vector<Quantity>						m_quantities;

	/*! Map for connecting unique quantity names to global ids (for reverse lookup). */
	std::map<std::string, unsigned int>			m_globalIndexMap;
};

} // namespace IBK

/*! \file IBK_QuantityManager.h
	\brief Contains the declaration of class QuantityManager.
*/

#endif // IBK_QuantityManagerH


