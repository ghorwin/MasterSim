#ifndef IBK_LogfileH
#define IBK_LogfileH

#include <string>
#include <iosfwd>
#include <fstream>

namespace IBK {

/*! The class Logfile allows the redirection of standard streams into file
	streams.
	This class is a convinient way of using standard streams such as clog or
	cerr into log- or error files. As long as the redirection object exists
	all output to the redirected streams will be written into the files
	instead. With the member function redirect() the file can be changed or
	logging can be turned off (useful if logging can be turned on/off during
	runtime).
*/
class Logfile {
  public:
	/*! Constructor, creates an object for stream redirection
		Creates, overwrites or appends the file 'filename' depending on the
		'filename'. All further output to the standard stream 'original'
		will be written into the logfile instead. This redirection works until
		the destruction of the logfile object or a call to the member function
		redirect().
		\param original  may be either of cout, cerr or clog
		\param filename  filename of the log file (ansi encoded)
		\param ioflags   (optional) open flags, see ios_base::openmode.
						 [default: overwrite]
	*/
	Logfile(std::ostream& original, const std::string& filename,
			std::ios_base::openmode ioflags=std::ios_base::out | std::ios_base::trunc);

	/*! Destructor.
		When the logfile object is destructed the redirection will be removed.
	*/
	~Logfile();

	/*! Removes the stream redirection.
		In combination with the other redirect() member function this can be
		used for turning logging on/off turing runtime.
	*/
	void redirect();

	/*! Redirects the stream into another log file.
		This function is useful for changing the log file of a certain
		redirected stream. Use this function if you want to change the logfile
		name (e.g. if you change the file name daily) but want to keep the
		logfile object. If you turned logging off using the other redirect()
		member function previously this function can be used to turn the logging
		back on.
		\param filename  filename of the logfile
		\param ioflags   (optional) open flags, see ios_base::openmode.
						 [default: overwrite]
	*/
	void redirect(const std::string& filename, std::ios_base::openmode ioflags=std::ios_base::out | std::ios_base::trunc);

private:
	std::basic_ostream<char>*   m_original;      // original buffer
	std::basic_streambuf<char>* m_buffer;        // file stream buffer
	std::basic_ofstream<char>   m_filestream;    // file stream
};

/*! \file IBK_Logfile.h
	\brief Contains the class Logfile for redirecting standard stream output to log files.

	\example Logfile.cpp
	This is an example of how to use the class IBK::Logfile.
*/

} // namespace IBK

#endif // IBK_LogfileH

