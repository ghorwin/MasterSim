#ifndef IBK_FlagH
#define IBK_FlagH

#include <string>
#include <iosfwd>

namespace IBK {

/*! The class Flag represents a named boolean flag.
	If the name of a flag is empty, the default assumption is that the flag is not specified.
	Use this convention to decide whether is flag is to be written to output/input file or not.
*/
class Flag {
public:
	/*! Default constructor, creates a disabled (unnamed) flag with state 'false'. */
	Flag() : m_state(false) {}

	/*! Constructor to initialise the flag.	*/
	Flag(const std::string& n, bool s) : m_name(n), m_state(s) {}

	/*! Sets the flag state and whether it is enabled or not. */
	void set(const std::string& n, bool s ) { m_name = n; m_state = s; }

	/*! Checking convenience function that compares the given state as string and throws an exception
		if an invalid value is passed.
		Valid strings are:
		* "0" or "1"
		* "true" or "false"
		* "yes" or "no"
		The argument string is converted to lower-case before running the comparison.
		If an invalid state string is passed, the function throws an IBK::Exception.
	*/
	void set(const std::string& n, const std::string& s);

	/*! Reads a flag from the stream 'in'.
		The format for reading is: name state<br>
		\param in       The input stream.
	*/
	void read(std::istream& in);

	/*! Writes a flag into the output stream.
		The output format is: name state<br>
		The optional parameters can be used to specify the layout.
		\param out          The output stream.
		\param indent       (optional) Number of spaces, that should be put in front.
		\param width        (optional) Width of the flag name (exclusive indentation),
									   a " = " is always appended
	*/
	void write(std::ostream& out, unsigned int indent=0, unsigned int width=0) const;

	/*! Clears the flag. */
	void clear() { m_name.clear(); m_state = false; }

	/*! Returns true, if the flag is set, and the state is true. */
	bool isEnabled() const { return !m_name.empty() && m_state; }

	/*! Returns the name of a flag. If name is empty flag is not defined and thus invalid. */
	const std::string & name() const { return m_name; }

	/*! Compares this instance with another by value and returns true if they differ. */
	bool operator!=(const Flag & other) const;

	/*! Compares this instance with another by value and returns true if they are the same. */
	bool operator==(const Flag & other) const { return ! operator!=(other); }


	// ****** member variables *************************************************
protected:

	/*! The descriptive keyword/name of the parameter. */
	std::string 	m_name;

	/*! The state of the flag. */
	bool 			m_state;
};

} // namespace IBK

/*! \file IBK_Flag.h
	\brief Contains the declaration of the class Flag, a class for a named boolean.
*/

#endif // IBK_FlagH
