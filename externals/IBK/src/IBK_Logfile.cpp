#include "IBK_configuration.h"

#include <iostream>
#include <fstream>
#include <streambuf>

#include "IBK_Logfile.h"
#include "IBK_messages.h"
#include "IBK_FormatString.h"

namespace IBK {

// Constructor
Logfile::Logfile(std::ostream& original, const std::string& filename, std::ios_base::openmode ioflags)
	: m_original(NULL), m_buffer(NULL)
{
	m_filestream.open(filename.c_str(), ioflags);
	if (!m_filestream) {
		IBK::IBK_Message( FormatString("Couldn't create file '%1'.") .arg(filename), MSG_ERROR, "[LogFile::LogFile]", 3);
		return;
	}
	m_original=&original;
	m_buffer=original.rdbuf();
	original.rdbuf(m_filestream.rdbuf());
};

// Destructor
Logfile::~Logfile() {
	if ((m_original!=NULL) && (m_buffer!=NULL))
		m_original->rdbuf(m_buffer);
}

// Turns logging off
void Logfile::redirect() {
	m_original->rdbuf(m_buffer);
}

// Turns logging on
void Logfile::redirect(const std::string& filename, std::ios_base::openmode ioflags) {
	// if there is already a file opened, close it first
	if (m_filestream)
		m_filestream.close();
	// open new logfile
	m_filestream.open(filename.c_str(), ioflags);
	if (!m_filestream) {
		IBK::IBK_Message(FormatString("Couldn't create file '%1'.") .arg(filename), MSG_ERROR, "[Logfile::redirect]", 3);
		return;
	}
	if ((m_original!=NULL) && (m_buffer!=NULL))
		m_original->rdbuf(m_filestream.rdbuf());
}

} // namespace IBK

