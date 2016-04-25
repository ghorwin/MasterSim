#include "IBK_configuration.h"

#include <iostream>
#include <iomanip>
#include <ctime>
#include <fstream>
#include <sstream>

#include "IBK_MessageHandlerRegistry.h"

namespace IBK {

MessageHandlerRegistry::MessageHandlerRegistry() {
	m_msgHandler = &m_defaultMsgHandler;
}

MessageHandlerRegistry::~MessageHandlerRegistry() {
	setDefaultMsgHandler();
}

MessageHandlerRegistry& MessageHandlerRegistry::instance() {
	static MessageHandlerRegistry myMsgHandler;
	return myMsgHandler;
}

void MessageHandlerRegistry::setDefaultMsgHandler() {
	m_msgHandler = &m_defaultMsgHandler;
}

void MessageHandlerRegistry::setMessageHandler(MessageHandler *handle) {
	m_msgHandler = handle;
}

} // namespace IBK

