#include "MSIMMessageHandler.h"
#include "MSIMConversion.h"

MSIMMessageHandler::MSIMMessageHandler(QObject *parent) :
	QObject(parent)
{
}


MSIMMessageHandler::~MSIMMessageHandler() {
}


void MSIMMessageHandler::msg(const std::string& msg,
	IBK::msg_type_t t,
	const char * func_id,
	int verbose_level)
{
	IBK::MessageHandler::msg(msg, t, func_id, verbose_level);
	if (verbose_level > m_requestedConsoleVerbosityLevel)
		return;

	emit msgReceived(t, utf82QString(msg).trimmed());
}

