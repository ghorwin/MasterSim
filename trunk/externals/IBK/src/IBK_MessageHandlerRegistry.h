#ifndef IBK_MessageHandlerRegistryH
#define IBK_MessageHandlerRegistryH

#include <string>
#include "IBK_Constants.h"
#include "IBK_MessageHandler.h"

namespace IBK {


/*! This class relays messages (error, debug or progress messages)
	from the IBK library towards the current message handling class.
	You can set a message handler via setMessageHandlerRegistry(). The object
	does not get owned by the MessageHandlerRegistry singleton.
*/
class MessageHandlerRegistry {
public:

	/*! Destructor, reset the default message receiver before exiting. */
	~MessageHandlerRegistry();

	/*! Returns a singleton instance to the current IBK message handler. */
	static MessageHandlerRegistry& instance();

	/*! Relays a message with given type to the message handler.
		\sa IBK_Message.
		*/
	void msg(	const std::string& msg,
				msg_type_t t = MSG_PROGRESS,
				const char * func_id = NULL,
				int verbose_level = VL_ALL)
	{
		m_msgHandler->msg(msg, t, func_id, verbose_level);
	}

	/*! Resets the default message handler. */
	void setDefaultMsgHandler();

	/*! Sets a message handler instance. */
	void setMessageHandler(MessageHandler * handle);

	/*! Returns the message handler instance. */
	MessageHandler * messageHandler() { return m_msgHandler; }

private:
	/*! Singleton - Constructor hidden from public. */
	MessageHandlerRegistry();

	/*! Singleton - Copy constructor hidden and not implemented. */
	MessageHandlerRegistry(const MessageHandlerRegistry&);

	/*! Singleton - Copy assignment operator hidden and not implemented. */
	MessageHandlerRegistry& operator=(const MessageHandlerRegistry&);

	MessageHandler				m_defaultMsgHandler;	///< Default message handler object.
	MessageHandler *			m_msgHandler;			///< Pointer to current message handler object.
};

} // namespace IBK

/*! \file IBK_MessageHandlerRegistry.h
	\brief Contains declaration of class IBK_MessageHandlerRegistry
*/

#endif // IBK_MessageHandlerRegistryH

