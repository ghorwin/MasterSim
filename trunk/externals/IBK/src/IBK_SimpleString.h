#ifndef IBK_SimpleStringH
#define IBK_SimpleStringH

#include <string>

namespace IBK {

/*! Light-weight wrapper class for a C-style string to be used in
	DLL Interfaces.

	The string is implemented using plain old C-types, so binary layout is the same in all libraries, regardless
	of the individual standard library implementation. For example, a string of type SimpleString may be returned
	by a DLL compiled with Borland Builder and used by a library compiled with Visual Studio. It is basically a
	litte more convenient and safe form of passing character arrays of c-strings.
*/
class SimpleString {
public:
	/*! Constructor, creates an empty string. */
	SimpleString()
		: m_buf(NULL), m_len(0)
	{
	}

	/*! Constructor, creates a string as copy of 'src'. */
	SimpleString(const char* src)
		: m_buf(NULL), m_len(0)
	{
		if (src != NULL) {
			std::size_t newlen = strlen(src);
			try
			{
				m_buf = new char[newlen+1];
				std::copy(src, src + newlen, m_buf);
				m_buf[newlen] = '\0';
				m_len = newlen+1;
			}
			catch(...)
			{
				m_buf = 0;
				m_len = 0;
			}
		}
	}

	/*! Constructor, creates a string as copy of 'src'. */
	SimpleString(const std::string & str)
		: m_buf(NULL), m_len(0)
	{
		*this = str;
	}

	/*! Copy constructor */
	SimpleString(const SimpleString& src)
		: m_buf(NULL), m_len(0)
	{
		if (src.m_len > 0)
		{
			m_buf = new char[src.m_len];
			m_len = src.m_len;
			std::copy(src.m_buf, src.m_buf + src.m_len, m_buf);
		}
	}

	/*! Destructor, removes all allocated memory */
	~SimpleString() {
		delete[] m_buf;
	}

	/*! Swaps content of 'src' with content of current object */
	void swap(SimpleString& src) {
		std::swap(m_buf, src.m_buf);
		std::swap(m_len, src.m_len);
	}

	/*! Assignment operator for SimpleString objects */
	SimpleString& operator=(const SimpleString& src) {
		if( this == &src)
			return *this;		// prevent assignment to self
		SimpleString temp(src);
		swap(temp);
		return *this;
	}

	/*! Assignment operator for c-strings */
	SimpleString& operator=(const char* src) {
		SimpleString temp(src);
		swap( temp );
		return *this;
	}

	/*! Assignment operator for std::strings */
	SimpleString& operator=(const std::string& src) {
		SimpleString temp(src.c_str());
		swap( temp );
		return *this;
	}

	/*! Direct read-only access to content. */
	const char* c_str() const {
		return m_buf;
	}

	/*! Clears the string contents. */
	void clear() {
		delete[] m_buf;
		m_buf = 0;
		m_len = 0;
	}

private:

	char* m_buf;
	size_t m_len;
};

} // namespace IBK

/*! \file IBK_SimpleString.h
	\brief Contains the declaration of class SimpleString.
*/


#endif // IBK_SimpleStringH
