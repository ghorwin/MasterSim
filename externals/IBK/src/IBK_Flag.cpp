#include "IBK_configuration.h"

#include "IBK_Flag.h"

#include <stdexcept>
#include <iostream>
#include <iomanip>

#include "IBK_StringUtils.h"
#include "IBK_FormatString.h"
#include "IBK_Exception.h"

namespace IBK {

void Flag::set(const std::string& n, const std::string& s) {
	std::string lowerCaseState = IBK::tolower_string(s);
	m_name = n;
	if (lowerCaseState == "1" || lowerCaseState == "yes" || lowerCaseState == "true") {
		m_state = true;
	}
	else if (lowerCaseState == "0" || lowerCaseState == "no" || lowerCaseState == "false") {
		m_state = false;
	}
	else
		throw IBK::Exception( IBK::FormatString("Invalid value '%2' for flag '%1'!").arg(m_name).arg(s),"[Flag::set]" );
}



void Flag::read(std::istream& in) {
	std::string name_;
	std::string state_;
	in >> name_ >> state_;
	if (!in)
		throw IBK::Exception( "Error reading flag!","[Flag::read]");
	if (state_ == "1" || string_nocase_compare(state_, "yes") || string_nocase_compare(state_, "true") ) {
		m_state = true;
	}
	else if (state_ == "0" || string_nocase_compare(state_, "no") || string_nocase_compare(state_, "false") ){
		m_state = false;
	}
	else
		throw IBK::Exception( IBK::FormatString("Invalid value for flag '%1' !") .arg(name_),"[Flag::read]" );
}



void Flag::write(std::ostream& out, unsigned int indent, unsigned int width) const {
	out << std::string(indent, ' ');
	out << std::setw(width) << std::left << m_name << " = ";
	if (m_state) 	out << "yes";
	else		out << "no";
}



bool Flag::operator!=(const Flag & other) const {
	if (m_state != other.m_state) return true;
	if (m_name != other.m_name) return true;
	return false; // this and other hold the same data
}


} // namespace IBK
