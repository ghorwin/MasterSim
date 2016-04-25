#ifndef IBK_FileUtilsH
#define IBK_FileUtilsH

#include <vector>
#include <string>
#include <cstring>

namespace IBK {

class Path;

/*! Extracts the ID number from a filename.
	\param fname A filename with format "fname_<xxx>.<ext>".
	\return Function returns 0 for invalid filenames or filename with different format.

	Example:
	\code
	string fname = "myfile_17.txt";
	// extract id number
	unsigned int id = extract_ID_from_filename(fname);
	// id is now 17.
	\endcode
*/
unsigned int extract_ID_from_filename(const IBK::Path & fname);


/*! Replaces the ID number in a filename.
	\param fname	A filename with format "fname_<xxx>.<ext>".
	\param newId	New ID for the given filename.
	\return			Returns the new filename with format "fname_<newId>.<ext>".

	Example:
	\code
	std::string fname = "myfile_17.txt";
	unsigned int newId = 100;
	// extract id with
	std::string newFName = replace_ID_in_filename(fname, newId);
	// fname is now "myfile_100.txt".
	\endcode
*/
IBK::Path replace_ID_in_filename( const IBK::Path & fname, const unsigned int newId );

/*! Tries to read N bytes in binary mode from file \a filename.
	This is the checked version.
	\param filename Name of the file.
	\param size Maximum number of bytes for reading.
	\param errmsg Error message.
	\return the vector which could be read.
*/
std::vector<unsigned char> read_some_bytes(const IBK::Path& filename, unsigned int size, std::string& errmsg);

/*! Reads ASCII file into string, not very performant, but simple.
	\param fname Name of the file.
	\return File content as string.
*/
std::string file2String(const IBK::Path & fname);

/*! Converts a sentence of bytes given by \a bytes to the given value.
	No check for vector size take place.
	\param bytes Vector for reading.
	\param value Value to be read from bytes
	\param begin Start position in bytes for reading
	\return position one after the readed value in bytes.
*/
template<typename T>
unsigned int bytes2value(const std::vector<unsigned char>& bytes, T& value, unsigned int begin = 0) {
	std::memcpy((unsigned char*)&value, (&bytes[0]) + begin, sizeof(T));
	return begin + sizeof(T);
}

}  // namespace IBK


/*! \file IBK_FileUtils.h
	\brief Contains helper functions for file access on windows/posix systems.

*/
#endif // IBK_FileUtilsH
