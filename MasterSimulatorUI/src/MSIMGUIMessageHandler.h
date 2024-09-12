#ifndef MSIMGUIMessageHandlerH
#define MSIMGUIMessageHandlerH

#include <IBK_MessageHandler.h>
#include <QObject>

/*! Re-implemented message handler to handle messages from IBK::MessageHandler and direct them to the log widget. */
class MSIMGUIMessageHandler : public QObject, public IBK::MessageHandler  {
	Q_OBJECT
public:
	explicit MSIMGUIMessageHandler(QObject *parent = nullptr);

	/*! msg is an utf8 encoded string */
	virtual void msg(const std::string& msg,
		IBK::msg_type_t t = IBK::MSG_PROGRESS,
		const char * func_id = nullptr,
		int verbose_level = -1);

signals:
	/*! A signal that passes a new message in html format. */
	void newMessage(const QString & message);
	/*! A signal that passes only the last line of the output in plain text format. */
	void lastLineOfMessage(const QString & lastLineOfMessage);

};

#endif // MSIMGUIMessageHandlerH
