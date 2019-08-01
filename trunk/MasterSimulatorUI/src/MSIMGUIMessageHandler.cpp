#include "MSIMGUIMessageHandler.h"

MSIMGUIMessageHandler::MSIMGUIMessageHandler(QObject *parent) :
	QObject(parent)
{
}

void MSIMGUIMessageHandler::msg(const std::string& msg,
	IBK::msg_type_t t,
	const char * func_id,
	int verbose_level)
{
	// call default implementation
	IBK::MessageHandler::msg(msg, t, func_id, verbose_level);

	if ((t == IBK::MSG_PROGRESS || t == IBK::MSG_CONTINUED || t == IBK::MSG_WARNING) &&
		verbose_level > m_requestedConsoleVerbosityLevel)
	{
		return; // skip messages with too high verbosity
	}

	// compose message for log window
	QString msgString = QString::fromUtf8(msg.c_str());
	if (msgString.endsWith('\n'))
		msgString.chop(1);
	if (msgString.endsWith("\r\n"))
		msgString.chop(2);

	std::string istr;
	if (m_indentation > 0)
		istr += std::string(2*m_indentation, ' ');

	msgString = QString::fromStdString(istr) + msgString;

	// remember last line as plain text
	QString lastLine = msgString.split('\n').last();

	// replace all spaces by hard spaces
	msgString = msgString.replace(" ", "&nbsp;");
	// replace all < and >
	msgString = msgString.replace("<", "&lt;");
	msgString = msgString.replace(">", "&gt;");
	// replace all line breaks by <br>
	msgString = msgString.replace("\n", "<br>");

	QString html = QString("<span style=\"white-space:pre; color:%2\">%1</span>").arg(msgString);
	switch (t) {
		case IBK::MSG_PROGRESS :
		case IBK::MSG_CONTINUED :
			html = html.arg("#000000");
			break;
		case IBK::MSG_WARNING :
			html = html.arg("#aa8800");
			break;
		case IBK::MSG_ERROR :
			html = html.arg("#bb0000");
			break;
		case IBK::MSG_DEBUG :
			html = html.arg("#808000");
			break;
	}

	emit newMessage(html);
	emit lastLineOfMessage(lastLine);
}

