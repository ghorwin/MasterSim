#ifndef MSIMMessageHandlerH
#define MSIMMessageHandlerH

#include <QObject>
#include <IBK_MessageHandler.h>

class MSIMMessageHandler : public QObject, public IBK::MessageHandler {
	Q_OBJECT
public:
	explicit MSIMMessageHandler(QObject *parent = 0);
	virtual ~MSIMMessageHandler();


	/*! Overloaded to received msg info. */
	virtual void msg(const std::string& msg,
		IBK::msg_type_t t = IBK::MSG_PROGRESS,
		const char * func_id = NULL,
		int verbose_level = -1);

signals:
	/*! Emitted whenever a message was received.
		Shall be connected to the log window to display the message.
	*/
	void msgReceived(int type, QString msg);

};

#endif // MSIMMessageHandlerH
