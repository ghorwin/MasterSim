#ifndef IBK_ParameterH
#define IBK_ParameterH

#include <string>
#include "IBK_Unit.h"
#include "IBK_math.h"
#include "IBK_assert.h"

namespace IBK {

/*! The class Parameter represents a physical parameter consisting of a descriptive
	keyword, a value and unit used for input/output. Parameters can be used in
	calculation programs for any kind of input parameters. The advantage of the
	use of a parameter is, that the input data can be given in a unit that differs
	from the SI base unit used for calculation.
	When created or set, a parameter always converts the value given in the IO unit
	into the internal SI base unit which can be retrieved via the 'value' member
	variable. The only exception is, when the parameter has been created with an
	'undefined' unit (unit id = 0).
	The member function get_value() can be used to retrieve the parameters value in
	the required unit.
*/
class Parameter {
public:
	/*! Default constructor, creates an empty parameter with undefined unit. */
	Parameter() : name(""), value(0), IO_unit(0) {}

	/*! Constructor to initialise the parameter with name, value and IO unit.
		When you create a parameter like this the value 'val' will be converted into
		the base unit of 'unit', if available. For example when passing val=20 and
		mm as unit the value of the parameter will be 0.02 afterwards.
	*/
	Parameter(const std::string& str, double val, const Unit unit=Unit(0));

	/*! Function is just for convenience and works essentially like the function above.
		If the unit is invalid, the function throws an exception.
	*/
	Parameter(const std::string& str, double val, const std::string& unit_str);

	/*! Sets value and unit with conversion.
		The value 'val' will be converted from 'unit' in the base unit of 'unit'.
	*/
	bool set(double val, Unit unit, std::string * errmsg = NULL);

	/*! Sets name, value and unit with conversion.
		The value 'val' will be converted from 'unit' in the base unit of 'unit'.
	*/
	bool set(const std::string& str, double val, Unit unit, std::string * errmsg = NULL);

	/*! Sets name and value without conversion.
		The value 'val' will not be converted and the unit will be set to 'undefined'.
	*/
	void set(const std::string& str, double val);

	/*! Sets name, value and unit with conversion.
		The value 'val' will be converted from unit 'ustr' in the base unit of 'ustr'.
	*/
	bool set(const std::string& name_, double val, const std::string& ustr, std::string * errmsg = NULL) { return set(name_, val, Unit(ustr), errmsg); }

	/*! Sets the parameter by name and value, whereas valstr is composed of the actual value and the IO_unit.
		The value will be converted in the corresponding base unit.
	*/
	bool set(const std::string& str, const std::string& valstr, std::string * errmsg = NULL);

	/*! Returns the value of the parameter converted into its IO unit. */
	double get_value() const { return get_value(IO_unit); }

	/*! Returns the value of the parameter converted into 'unit'.
		The function throws an IBK::Exception if the units cannot be related.
	*/
	double get_value(Unit unit) const;

	/*! Returns the value of the parameter converted into unit 'ustr'.
		The function throws a IBK::Exception exception if the units cannot be related.
	*/
	double get_value(const std::string& ustr) const { return get_value(Unit(ustr)); }

	/*! Returns the parameter fully formatted as std::string.
		The parameter is converted into the IO_unit.
		\param without_name If false, the parameter string is of format "<name> = <value> <unit>", otherwise of
			format "<value> <unit>".
	*/
	std::string toString(bool without_name = true) const;

	/*! Returns the parameter fully formatted as std::string.
		\param u The output unit.
		\param without_name If false, the parameter string is of format "<name> = <value> <unit>", otherwise of
			format "<value> <unit>".
	*/
	std::string toString(const IBK::Unit & u, bool without_name = true) const;

	/*! Reads a parameter from the stream 'in'.
		The format for reading is:
		\code
		<name> <value> <unit>
		\endcode
		When a parameter should be read without a name/keyword,
		pass 'true' as second parameter.
		The value that has been read will be converted from the IO-unit to
		the base unit.
		\param in       The input stream.
		\param no_name (optional) Flag that tells the read function to read only the value and unit.
	*/
	void read(std::istream& in, bool no_name=false);

	/*! Writes a parameter into the output stream.
		The output format is: name value unit <br>
		If the parameter hasn't got a name, only the value and the unit is written.
		The value will be first converted into the IO unit and then written.
		The optional parameters can be used to specify the layout.
		\param out          The output stream.
		\param indent       (optional) Number of spaces, that should be put in front.
		\param paramWidth   (optional) Width of the parameter name (exclusive indentation),
									   a " = " is always appended
		\param writeName    (optional) Should the name (keyword) be written or not.
	*/
	void write(std::ostream& out, unsigned int indent=0, unsigned int paramWidth=0, bool writeName=true) const;

	/*! Reads a parameter from the stream 'in'.
		It uses the binary representation.
		\param in       The input stream.
	*/
	void readBinary(std::istream& in);

	/*! Writes a parameter into the output stream.
		It uses the binary representation.
		\param out          The output stream.
	*/
	void writeBinary(std::ostream& out) const;

	/*! Returns the current base unit of the parameter. */
	Unit unit() const { return Unit(IO_unit.base_id()); }

	/*! Clears the parameter (name and IO_unit are set to empty strings). */
	void clear();

	/*! Returns true if the parameter is empty (a parameter is empty when it does not
		have a name).
	*/
	bool empty() const { return name.empty(); }

	/*! Compares this instance and another by content and returns true if they differ. */
	bool operator!=(const Parameter& other) const {
		if (IO_unit != other.IO_unit)
				return true;

		if (!near_equal(value, other.value))
				return true;

		if (name != other.name)
				return true;

		return false;
	}

	/*! Compares this instance and another by content and returns true if are the same. */
	bool operator==(const Parameter& other) const {
			return !operator!=(other);
	}

	/*! Comparison function, compares two parameters with 5 digits accuracy.
		If you need a different level of accuracy, compare the values of the parameters
		directly.
		Requires the IO_units of this and the other parameter to have the same
		base unit.
	*/
	bool equalTo(const Parameter& other) {
		IBK_ASSERT(IO_unit.base_id() == other.IO_unit.base_id());
		// compare with 5 digits accuracy
		return IBK::nearly_equal<5>(value, other.value);
	}

	/*! This method is used to test lower limits of a physical parameter.
		In case of an error this method throws an exception. Errors are missmatching parameter names,
		missmatching base units, and comparsion operation missmatches.
		\param parameterLowerBound Unit, parameter name and value to be tested against.
		\param isLessEqual If set to true comparison operator is <= is used, if false < is applied.
		\return True is returned if parameter name and unit can be matched, as well as the value test is valid as well.
	*/
	void checkLowerBound (	const IBK::Parameter& parameterLowerBound,
							bool isLessEqual ) const;

	/*! This method is used to test lower limits of a physical parameter.
		In case of an error this method throws an exception. Errors are missmatching parameter names,
		missmatching base units, and comparsion operation missmatches.
		\param parameterUpperBound Unit, parameter name and value to be tested against.
		\param isGreaterEqual If set to true comparison operator is >= is used, if false > is applied.
	*/
	void checkUpperBound (	const IBK::Parameter& parameterUpperBound,
							bool isGreaterEqual ) const;


	// ****** member variables *************************************************

	/*! The descriptive keyword/name of the parameter. */
	std::string 	name;
	/*! The value of the parameter for calculations (always in SI base unit). */
	double 			value;
	/*! Input and output unit for the parameter. */
	Unit 			IO_unit;

private:

	/*! Operators used by private test routine. */
	enum oper_t {
		OP_LT,	///< less then
		OP_LE,	///< less equal then
		OP_GE,	///< greater equal then
		OP_GT,	///< greater then
	};


	/*! Test routine for code reuse by \sa checkLowerBound and \sa checkUpperBound. */
	void test( Parameter val, oper_t op ) const;

};

/*! "Less then" operator, returns true if value of left parameter is less than value of right parameter.
	Requires the IO_units of this and the other parameter to have the same
	base unit.
*/
inline bool operator<(const Parameter& lhs, const Parameter& rhs) {
	IBK_ASSERT(lhs.IO_unit.base_id() == rhs.IO_unit.base_id());
	return lhs.value < rhs.value;
}

}  // namespace IBK

/*! \file IBK_Parameter.h
	\brief Contains the declaration of the class Parameter, a class for a physical parameter
		   with name, value and assiciated Input/Output unit.
*/

#endif // IBK_ParameterH

