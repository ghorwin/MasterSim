#include "IBK_configuration.h"

#include "IBK_Exception.h"
#include "IBK_messages.h"

namespace IBK {

Exception::Exception()
{
}

Exception::~Exception() throw() {
}

Exception::Exception(const std::string& what, const std::string& loc) {
	msgs_.push_back( IBK::Exception::MsgInfo(what, loc) );
}

Exception::Exception(const IBK::FormatString& what, const std::string& loc) {
	msgs_.push_back( IBK::Exception::MsgInfo(what.str(), loc) );
}

Exception::Exception(const Exception & old, const std::string& what, const std::string& loc) :
	msgs_(old.msgs_)
{
	msgs_.push_back( IBK::Exception::MsgInfo(what, loc) );
}

Exception::Exception(const Exception & old, const IBK::FormatString& what, const std::string& loc) :
	msgs_(old.msgs_)
{
	msgs_.push_back( IBK::Exception::MsgInfo(what.str(), loc) );
}

Exception::Exception(const std::exception & old, const IBK::FormatString& what, const std::string& loc){
	msgs_.push_back( IBK::Exception::MsgInfo( old.what(), loc ) );
	msgs_.push_back( IBK::Exception::MsgInfo(what.str(), loc) );
}


const char* Exception::what() 	const throw() {
	if (msgs_.empty()) return "";
	else return msgs_.back().what.c_str();
}


const char* Exception::location() const {
	if (msgs_.empty()) return "";
	else return msgs_.back().location.c_str();
}

void Exception::writeMsgStackToError() const {
	for (std::list<MsgInfo>::const_iterator it = msgs_.begin();
		it != msgs_.end(); ++it)
	{
		IBK::IBK_Message(it->what, MSG_ERROR, it->location.c_str(), 1);
	}
}

std::string Exception::msgStack() const {
	std::string allMsgs;
	for (std::list<MsgInfo>::const_iterator it = msgs_.begin();
		it != msgs_.end(); ++it)
	{
		allMsgs += std::string(it->what);
		std::list<MsgInfo>::const_iterator itNext = it;
		if (++itNext != msgs_.end())
			allMsgs +=" \n";
	}
	return allMsgs;
}

} // namespace IBK

