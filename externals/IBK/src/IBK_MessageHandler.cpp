#include "IBK_configuration.h"

#include <iostream>
#include <iomanip>
#include <ctime>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "IBK_messages.h"
#include "IBK_MessageHandler.h"
#include "IBK_StringUtils.h"
#include "IBK_Path.h"

namespace IBK {

/*! Helper function for MessageHandler to print out utf8 strings. */
void printUtf8(const std::string & utf8) {
#ifdef _WIN32
	if (GetConsoleOutputCP() == CP_UTF8)
		std::wprintf(L"%S", utf8.c_str());
	else
		std::cout << utf8;
#else
	std::cout << utf8;
#endif
	std::cout.flush();
}

// *** MessageHandler ***

MessageHandler::MessageHandler() :
	m_contextIndentation(40),
	// by default, print all messages within IBK library to console
	m_requestedConsoleVerbosityLevel(VL_DEVELOPER),
	// by default, write only messages up to VL_INFO and including to logfile
	m_requestedLogfileVerbosityLevel(VL_INFO),
	m_logfile(NULL),
	m_timeStampFormat("%Y-%m-%d %H:%M:%S"),
	m_indentation(0)
{
}

MessageHandler::~MessageHandler() {
	closeLogFile();
	// If this message handler instance is currently set in MessageHandlerRegistry
	// replace it with the default message handler
	// so that access violations during cleanup are prevented.
	if (MessageHandlerRegistry::instance().messageHandler() == this) {
		MessageHandlerRegistry::instance().setDefaultMsgHandler();
	}
}


void MessageHandler::setConsoleVerbosityLevel(int verbosity) {
	m_requestedConsoleVerbosityLevel = verbosity;
}

void MessageHandler::setLogfileVerbosityLevel(int verbosity) {
	m_requestedLogfileVerbosityLevel = verbosity;
}


void MessageHandler::setupUtf8Console() {

#if defined(_WIN32) && !defined(__MINGW32__)
	// set font capable of showing unicode characters
	CONSOLE_FONT_INFOEX cfi;
	cfi.cbSize = sizeof cfi;
	cfi.nFont = 0;
	cfi.dwFontSize.X = 0;
	cfi.dwFontSize.Y = 16;
	cfi.FontFamily = FF_DONTCARE;
	cfi.FontWeight = FW_NORMAL;
	std::wcscpy(cfi.FaceName, L"Consolas");
	SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);

	// set console code page to support UTF8
	SetConsoleOutputCP(CP_UTF8);
#endif // _WIN32

}


bool MessageHandler::openLogFile(const std::string& logfile, bool append, std::string & errmsg) {

	closeLogFile();
	if (logfile.empty())
		return true;
	if (append) {
#if defined(_WIN32) && !defined(__MINGW32__)
		m_logfile = new std::ofstream( IBK::Path(logfile).wstr().c_str(), std::ios_base::app);
#else // _WIN32
		m_logfile = new std::ofstream( logfile.c_str(), std::ios_base::app);
#endif // _WIN32
		*m_logfile << "\n----------- continued -------------\n" << std::endl;
	}
	else {
#if defined(_WIN32) && !defined(__MINGW32__)
		m_logfile = new std::ofstream( IBK::Path(logfile).wstr().c_str());
#else // _WIN32
		m_logfile = new std::ofstream( logfile.c_str() );
#endif // _WIN32
	}
	if (!m_logfile->good()) {
		closeLogFile();
		errmsg = "Cannot open/create logfile '"+logfile+"'.";
		return false;
	}
	return true;
}


void MessageHandler::setTimeStampFormat(const std::string & timeStampFormat) {
	m_timeStampFormat = timeStampFormat;
}


void MessageHandler::closeLogFile() {
	if (m_logfile) {
		m_logfile->close();
		delete m_logfile;
		m_logfile = NULL;
	}
}


// implementation of default message handler function
// in this function we don't care about verbosity levels
void MessageHandler::msg(const std::string& msg, msg_type_t t, const char * func_id, int verbose_level) {
	std::string vbStr;
//#define PRINT_VERBOSITY_LEVEL
#ifdef PRINT_VERBOSITY_LEVEL
	std::stringstream strm;
	strm << std::setw(2) << std::right << verbose_level << " ";
	vbStr = strm.str();
#endif

	std::string timeStamp;
	if (!m_timeStampFormat.empty()) {
		std::time_t timeT = std::time(NULL);
		std::tm *currentTimeTM = std::localtime(&timeT);
		std::string buffer(500 + m_timeStampFormat.size(), ' ');

		std::size_t charCount = std::strftime(&buffer[0], (int)buffer.size(), m_timeStampFormat.c_str(), currentTimeTM);
		timeStamp = buffer.substr(0,charCount) + "\t";
	}

	/// \todo add configuration option to enable/disable context printing
	std::string contextStr;
	if (func_id != NULL) {
		std::stringstream strm;
		strm << std::setw(m_contextIndentation) << std::left << std::string(func_id) << "\t";
		contextStr = strm.str();
	}
	else
		contextStr = std::string(m_contextIndentation, ' ')  + "\t";

	std::string messageTypeStr;
	switch (t) {
		case MSG_PROGRESS :
		case MSG_CONTINUED :
			messageTypeStr = "[Progress]\t";
			break;
		case MSG_WARNING :
			messageTypeStr = "[Warning ]\t";
			break;
		case MSG_ERROR :
			messageTypeStr = "[Error   ]\t";
			break;
		case MSG_DEBUG :
			messageTypeStr = "[Debug   ]\t";
			break;
		default :
			messageTypeStr = "[Invalid ]\t";
	}

	std::string istr;
	if (m_indentation > 0)
		istr += std::string(2*m_indentation, ' ');

	std::string indentedMsg;
	for (unsigned int i=0; i<msg.size(); ++i) {
		if (msg[i] == '\n' && i != msg.size()-1)
			indentedMsg += "\n" + timeStamp + messageTypeStr + contextStr + istr;
		else
			indentedMsg += msg[i];
	}

	std::stringstream consoleOut;

	switch (t) {
		case MSG_PROGRESS:
			// write output for all messages to logfile
			// to avoid spamming the output file, do not log anything above detailed level
			if (m_logfile!=NULL && verbose_level != VL_SPECIAL &&
				verbose_level <= m_requestedLogfileVerbosityLevel)
			{
				*m_logfile << timeStamp << messageTypeStr << contextStr << istr << indentedMsg;
			}

			// screen output only if verbosity level is high enough for message verbose_level
			if (verbose_level <= m_requestedConsoleVerbosityLevel) {
				consoleOut << vbStr << istr << msg;
				printUtf8(consoleOut.str());
			}
			break;

		case MSG_CONTINUED:
			if (m_logfile!=NULL && verbose_level != VL_SPECIAL &&
				verbose_level <= m_requestedLogfileVerbosityLevel)
			{
				*m_logfile << msg;
			}
			if (verbose_level <= m_requestedConsoleVerbosityLevel) {
				consoleOut << msg;
				printUtf8(consoleOut.str());
			}
			break;

		case MSG_WARNING :
			if (m_logfile!=NULL && verbose_level != VL_SPECIAL &&
				verbose_level <= m_requestedLogfileVerbosityLevel)
			{
				*m_logfile << timeStamp << messageTypeStr << contextStr << istr << indentedMsg << std::endl;
			}
			if (verbose_level <= m_requestedConsoleVerbosityLevel) {
				IBK::set_console_text_color(IBK::CF_BRIGHT_YELLOW);
				consoleOut << vbStr << istr << msg << std::endl;
				printUtf8(consoleOut.str());
#ifdef _WIN32
				IBK::set_console_text_color(IBK::CF_WHITE);
#else // _WIN32
				IBK::set_console_text_color(IBK::CF_GREY);
#endif // _WIN32
			}
			break;

		case MSG_ERROR :
			if (m_logfile!=NULL) {
				*m_logfile << timeStamp << messageTypeStr << contextStr << istr << indentedMsg << std::endl;
			}
			IBK::set_console_text_color(IBK::CF_BRIGHT_RED);
			consoleOut << vbStr << istr << msg << std::endl;
			printUtf8(consoleOut.str());
#ifdef _WIN32
			IBK::set_console_text_color(IBK::CF_WHITE);
#else // _WIN32
			IBK::set_console_text_color(IBK::CF_GREY);
#endif // _WIN32
			break;

		case MSG_DEBUG :
			if (m_logfile!=NULL) {
				*m_logfile << timeStamp << messageTypeStr << contextStr << istr << indentedMsg << std::endl;
			}
			IBK::set_console_text_color(IBK::CF_BRIGHT_MAGENTA);
			consoleOut << vbStr << istr << msg << std::endl;
			printUtf8(consoleOut.str());
#ifdef _WIN32
			IBK::set_console_text_color(IBK::CF_WHITE);
#else // _WIN32
			IBK::set_console_text_color(IBK::CF_GREY);
#endif // _WIN32
			break;
		default : ; // do nothing
	}

	if (m_logfile != NULL)
		m_logfile->flush();
}

} // namespace IBK

