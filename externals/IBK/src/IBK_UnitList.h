#ifndef IBK_UnitListH
#define IBK_UnitListH

#include "IBK_ptr_list.h"


#include <string>
#include <vector>
#include <iosfwd>
#include <iostream>

#include "IBK_UnitData.h"

namespace IBK {

class Unit;

/*! The class UnitList is the central class of the unit system.
	There is always a central unit list in the program which will be initialized
	with a default set of units. Use any of the read() functions to replace/add units.
	A unit list can be written to a file using the write function. The member function
	convertible_units() can be used to create a vector of units that can be converted in
	or from each other. The member functions relate_units() and convert() are useful
	if you want to implement a container, such as UnitVector.
*/
class UnitList : public ptr_list<UnitData> {
public:

	/*! Returns a reference to the UnitList singleton. */
	static UnitList&  instance();

	/*! Reads the unit list from the file 'filename'.
		The unit list will be usually read from a file, but can be read from a stream
		(e.g. stringstream) also. Take a look at the example unit list file for info
		about the format.
		If the second parameter is 'true', the unit list will be erased before new
		units are read from the file. Otherwise read units are just added to the list.
		The function throws an exception if a syntax error is encountered in the unit list file.
		If the unit list cannot be found or opened, the function simply returns 'false' and
		does not throw an exception.
		If the unit list can be read without problems the function returns 'true'.
	*/
	bool read_file(const std::string& filename, bool overwrite=false);

	/*! Reads built-in unit list from DEFAULT_UNITS constant defined in IBK_UnitList.cpp. */
	bool read_default();

	/*! Reads the unit list from a stream.
		Works essentially like the function above, only that the unit list data is expected to
		be in the input stream 'stream'.
	*/
	bool read(std::istream& stream, bool overwrite=false);

	/*! Reads the unitlist from a string.
		Works essentially like the function above, only that the unit list data is expected to
		be in the string (wherein each line is separated by a ';').
	*/
	bool read(const std::string& str, bool overwrite=false);

	/*! Writes the unit list into the file 'filename'.
		This function creates a new file with the name 'filename' and writes the unit list into
		that file. Returns 'false' if writing fails.
	*/
	bool write(const std::string& filename) const;

	/*! Creates a list of units, that can be converted in the unit given by 'src'.
		The function clears the vector (first argument) and add all units to it, that
		can be converted into 'src'. If the last (optional) parameter is true, the unit
		'src' will not be contained in the vector (per default the unit 'src' will be
		included in the vector).
		\return Returns 'false' if the vector is empty (meaning no convertible units
				could be found), otherwise 'true'. If the parameter 'remove' is false
				or not given the function will always return true.
	*/
	bool convertible_units(	std::vector<Unit>& list,
							const Unit& src,
							const bool remove=false) const;

	/*! Initialises a list of all known units. */
	void all_units( std::vector<Unit>& list ) const;

	/*! Creates a vector with all base units contained in the unit list. */
	bool base_units(std::vector<Unit>& list);

	/*! Determines the conversion operation and the factor/summand for converting from
		unit 'src' to 'target'.
		Throws an exceptions if the unit 'src' cannot be converted to 'target'.
	*/
	void relate_units(const UnitData* src, const UnitData* target,
					  double& fact, unsigned int& op) const;

	/*! Retrieves a unit data object for a unit by its ID.
		\warning Throws an exception if unit doesn't exist.
	*/
	const UnitData* retrieve(unsigned int unitid) const;

	/*! Retrieves a unit data object for a unit by its name.
		\warning Throws an exception if unit doesn't exist.
	*/
	const UnitData* retrieve(const std::string& str) const;

	/*! Converts the value 'val' from unit 'src' to unit 'target'. */
	void convert(Unit src, Unit target, double& val);

	/*! Converts the value 'val' from unit 'src' to unit 'target'. */
	void convert(const UnitData* src, const UnitData* target, double& val);

	/*! Determines and returns an IBK::Unit that corresponds to the space or/and time integral
		of a given source unit.
	*/
	static IBK::Unit integralQuantity(const IBK::Unit & srcUnit, bool spaceIntegral, bool timeIntegral);

	/*! Convenience function, replaces a unit within a string of format "xxx [<unit>]" with its time integral unit.
		\code
		std::string original = "Some quantity [J/s]";
		std::string integral = replaceUnitWithIntegralUnit(original); // = "Some quantity [J]";
		\endcode
	*/
	static std::string replaceUnitWithIntegralUnit(const std::string & original);

	/*! Operations returned by relate_units(). */
	enum UnitOperations {
		OP_NONE,
		OP_ADD,
		OP_SUB,
		OP_MUL,
		OP_DIV,
		OP_SPECIAL,
		OP_END
	};

private:
	/*! Hide constructor (initializes unit list with default units. */
	UnitList();
	/*! Hide copy constructor. */
	UnitList(const UnitList&);
	/*! Hide assignment operator. */
	const UnitList & operator=(const UnitList&);

	void convert_special(const UnitData* src, const UnitData* target, double& val);

	friend class Unit;
	friend class UnitVector;
};

}  // namespace IBK

/*! \file IBK_UnitList.h
	\brief Contains declaration of class UnitList.
*/

#endif // IBK_UnitListH
