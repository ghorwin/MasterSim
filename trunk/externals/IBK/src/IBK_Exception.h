#ifndef IBK_ExceptionH
#define IBK_ExceptionH

#include <exception>
#include <string>
#include <list>
#include "IBK_FormatString.h"

namespace IBK {

/*! This is the general exception class used when exceptions are throwns from IBK classes and functions.
	Use this class instead of the standard library exception classes.

	When throwing an exception from within a function, pass a message text and an information about
	the location. The following code shows an example for an exception throw from within a function
	splineCheck().
	\code
	if (spline.empty())
		throw IBK::Exception("Spline must not be empty!", "splineCheck()");
	\endcode

	If you catch and re-throw an exception, also pass the old exception to build up the exception stack.
	\code
	try {
		// some code
		splineCheck();
	}
	catch (const IBK::Exception & ex) {
		throw IBK::Exception(ex, "Error initializing module!", "checkAll()");
	}
	\endcode

	You can access the exception list by using the member function msgs() and process
	the individual messages stored in the msg_info struct.
*/
class Exception : public std::exception {
public:
	/*! Struct containing message info. */
	struct MsgInfo {
		/*! Convenience constructor, creates a message info struct.
			\param	w	Error message text.
			\param	loc Location string of the routine throwing the exception.
		*/
		MsgInfo(const std::string & w, const std::string & loc) : what(w), location(loc) {}
		std::string	what;		///< The message text.
		std::string	location;	///< The location.
	};

	/*! Destructor declaration needed because std::~exception() has throw specifier. */
	~Exception() throw();

	/*! Default constructor. */
	Exception();
	/*! Constructor for creating a new exception.
		\param	what	Error message text.
		\param	loc		Location string of the routine throwing the exception.
	*/
	Exception(const std::string& what, const std::string& loc);
	/*! Constructor for creating a new exception.
		\param	what	Formatted error message text including specific information of
						the function call.
		\param	loc		Location string of the routine throwing the exception.
	*/
	Exception(const IBK::FormatString& what, const std::string& loc);
	/*! Constructor for re-throwing an exception.
		\param	old		Previously thrown exception that currently was caught.
		\param	what	Additional error message text.
		\param	loc		Location string of the routine throwing the exception.
	*/
	Exception(const Exception & old, const std::string& what, const std::string& loc);
	/*! Constructor for re-throwing an exception.
		\param	old		Previously thrown exception that currently was caught.
		\param	what	Additional formatted error message text including specific information of
						the function call.
		\param	loc		Location string of the routine throwing the exception.
	*/
	Exception(const Exception & old, const IBK::FormatString& what, const std::string& loc);
	/*! Constructor for re-throwing an std::exception.
		\param	old		Previously thrown standard exception that currently was caught.
		\param	what	Additional formatted error message text including specific information of
						the function call.
		\param	loc		Location string of the routine throwing the exception.
	*/
	Exception(const std::exception & old, const IBK::FormatString& what, const std::string& loc);

	/*! Accesses the top-most (last) exception message.
		\return Error message string.
	*/
	const char* what() 	const throw();
	/*! Accesses the top-most (last) exception location.
		\return Location string.
	*/
	const char* location() const;

	/*! Accesses the stack of messages from all caught exceptions.
		\return current message stack.
	*/
	const std::list<MsgInfo> & msgs() const { return msgs_; }

	/*! Writes the content of the message list as IBK::MSG_ERROR message to IBK::IBK_Message(). */
	void writeMsgStackToError() const;

	/*! Exports the content of the message list as line-end separated error list.
		\return current message stack as a superset string.
	*/
	std::string msgStack() const;

private:
	/*! The message list. */
	std::list<MsgInfo> msgs_;
};

} // namespace IBK

/*! \file IBK_Exception.h
	\brief Contains declaration of class Exception.
*/

#endif // IBK_ExceptionH
