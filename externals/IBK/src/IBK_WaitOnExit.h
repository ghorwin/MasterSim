#ifndef IBK_WaitOnExitH
#define IBK_WaitOnExitH

#include "IBK_configuration.h"

namespace IBK {

/*! This is a simple class that calls the system command "pause" on windows
	systems to prevent automatic closing of console windows when run from
	Windows Explorer and similar.
	\code
	int main(int argc, char * argv[]) {
		IBK::WaitOnExit waitObj;

		// ... code ...
	} // at closing scope waitObj gets destroyed and calls system("pause") on Windows systems
	\endcode
*/
class WaitOnExit {
public:
	/*! Constructor.
		\param wait Optional argument that can be used to control the behavior of the object
					upon destruction.
	*/
	WaitOnExit(bool wait = true) : m_wait(wait) {}
#ifdef WIN32
	/*! Destructor, only on Windows systems. */
	~WaitOnExit() {
		if (m_wait) system("pause");
	}
#endif // WIN32

	/*! If true, the "pause" command is issued on destruction of this object, if false, nothing happens. */
	bool m_wait;
};

}  // namespace IBK

/*! \file IBK_WaitOnExit.h
	\brief Contains the declaration of the class WaitOnExit.
*/

#endif // IBK_WaitOnExitH
