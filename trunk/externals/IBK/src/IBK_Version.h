#ifndef IBK_VersionH
#define IBK_VersionH

#include <iosfwd>

namespace IBK {

/*! Provides version encoding/decoding used in ASCII and Binary files.

	The following constants need to be defined for each file to uniquely identify type and version of the file:
	- Major file version number, e.g. 3
	- Minor file version number, e.g. 5
	so that the total file version number is '3.5'.

	For unique file identification:
	- The first number of the magic header
	- The second number of the magic header

	Examples:
	\code
	// use case: read header from stream
	std::ifstream in("...");
	IBK::Version::readHeader(in, magicNumberFirstBinary, magicNumberSecondBinary, magicNumberFirstASCII, magicNumberSecondASCII,
							 isBinary, majorVersion, minorVersion);
	// throws Exception if reading fails, otherwise 'in' is positioned after 4-integer header
	// information is stored in arguments isBinary, majorVersion and minorVersion

	// use case: write header to file
	std::ofstream out("...");
	IBK::Version::writeHeader(out, magicNumberFirstBinary, magicNumberSecondBinary, magicNumberFirstASCII, magicNumberSecondASCII,
							  isBinary, majorVersion, minorVersion);
	\endcode

*/
class Version {
public:
	/*! Reads the first 4 integers from input stream and decodes version information.
		The information of the header is stored in the arguments that are passed by reference.
	*/
	static void read(std::istream & in, unsigned int magicNumberFirstBinary, unsigned int magicNumberSecondBinary,
					 unsigned int magicNumberFirstASCII, unsigned int magicNumberSecondASCII,
					 bool & isBinary, unsigned int & majorVersion, unsigned int & minorVersion);

	/*! Encodes and writes a file header the first 4 integers from input stream and decodes version information.
	*/
	static void write(std::ostream & out, unsigned int magicNumberFirstBinary, unsigned int magicNumberSecondBinary,
					  unsigned int magicNumberFirstASCII, unsigned int magicNumberSecondASCII,
					  bool isBinary, unsigned int majorVersion, unsigned int minorVersion);

private:
	/*! Decodes a combined single unsigned integer number into two version numbers.
		\param version	Takes version for encoding in ASCII encoded file.
		\param first	Returns first version number part from an ASCII encoded file.
		\param second	Returns second version number part from an ASCII encoded file.
	*/
	static void toASCIIEncoding( unsigned int version, unsigned int &first, unsigned int &second );

	/*! Encodes two version numbers into a single unsigned integer version number.
		\param first	Takes first version number part from an ASCII encoded file.
		\param second	Takes second version number part from an ASCII encoded file.
		\return			Version in ASCII encoding format.
	*/
	static unsigned int fromASCIIEncoding( unsigned int first, unsigned int second );

}; // Version

} // namespace IBK

/*! \brief Contains the class Version with version encoding/decoding functionality.
	\file IBK_Version.h
*/

#endif // IBK_VersionH
