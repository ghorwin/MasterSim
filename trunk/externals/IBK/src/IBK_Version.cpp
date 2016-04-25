#include "IBK_Version.h"

#include <cstdlib>

#include "IBK_InputOutput.h"
#include "IBK_assert.h"

namespace IBK {


/*! Mask for the major version number. */
unsigned majorMaskBinary = 0x000000ff;

/*! Mask for the minor version number. */
unsigned int minorMaskBinary = 0x0000ff00;

/*! Mask for the major version number. */
unsigned int majorMaskASCII = 0x0000ff00;

/*! Mask for the minor version number. */
unsigned int minorMaskASCII = 0x000000ff;


/*! Returns the complete version number as unsigned int formated for a binary output little endian. */
unsigned int versionNumberBinary(unsigned int majorNumber, unsigned int minorNumber ) {
	return ( majorNumber | (minorNumber << 8 ) );
}

/*! Extracts major number from version id
	\param version The version number read from an output file.
*/
unsigned int majorFromVersionBinary( unsigned int version ) {
	return ( version & majorMaskBinary );
}

/*! Extracts major number from version id
	\param version The version number read from an output file.
*/
unsigned int minorFromVersionBinary( unsigned int version ){
	return (( version & minorMaskBinary ) >> 8 );
}

/*! Returns the version number as ASCII encoded string */
unsigned int versionNumberASCII(unsigned int majorNumber, unsigned int minorNumber) {
	return ( (majorNumber << 8) | minorNumber );
}

/*! Extracts major number from version id
	\param version The version number read from an output file.
*/
unsigned int majorFromVersionASCII(unsigned int version) {
	return (( version & majorMaskASCII ) >> 8 );
}

/*! Extracts minor number from version id
	\param version The version number read from an output file.
*/
unsigned int minorFromVersionASCII( unsigned int version ) {
	return ( version & minorMaskASCII );
}


void Version::read(std::istream & in, unsigned int magicNumberFirstBinary, unsigned int magicNumberSecondBinary,
				 unsigned int magicNumberFirstASCII, unsigned int magicNumberSecondASCII,
				 bool & isBinary, unsigned int & majorVersion, unsigned int & minorVersion)
{
	const char * const FUNC_ID = "[Version::read]";

	// read first unsigned int from stream
	unsigned int test;
	IBK::read_uint32_binary( in, test);

	// check if this is a binary data file
	if ( test == magicNumberFirstBinary) {

		// read second unsigned int from stream
		IBK::read_uint32_binary( in, test);

		// test magic header
		if (test != magicNumberSecondBinary)
			throw IBK::Exception("Wrong second magic number!", FUNC_ID);

		// we have binary format
		isBinary = true;

		// read file version and split it
		IBK::read_uint32_binary( in, test);
		majorVersion = majorFromVersionBinary( test );
		minorVersion = minorFromVersionBinary( test );
		unsigned int dummy;
		// read last unsigned integer
		IBK::read_uint32_binary( in, dummy);
		IBK_ASSERT(dummy == 0); // safety check

	}
	else if (test == magicNumberFirstASCII) {

		// read second unsigned int from stream
		IBK::read_uint32_binary( in, test);

		// test magic header
		if (test != magicNumberSecondASCII)
			throw IBK::Exception("Wrong second magic number!", FUNC_ID);

		// not a binary file
		isBinary = false;

		// read file version and split it
		IBK::read_uint32_binary( in, test); // 3rd uint
		unsigned int second;
		IBK::read_uint32_binary( in, second); // 4th uint
		unsigned int version = fromASCIIEncoding(test, second);

		majorVersion = majorFromVersionASCII( version );
		minorVersion = minorFromVersionASCII( version );

	}
	else
		throw IBK::Exception("Invalid magic header or unknown file format.", FUNC_ID);


}


void Version::write(std::ostream & out, unsigned int magicNumberFirstBinary, unsigned int magicNumberSecondBinary,
				  unsigned int magicNumberFirstASCII, unsigned int magicNumberSecondASCII,
				  bool isBinary, unsigned int majorVersion, unsigned int minorVersion)
{
	// write magic header
	if (isBinary) {

		IBK::write_uint32_binary( out, magicNumberFirstBinary );
		IBK::write_uint32_binary( out, magicNumberSecondBinary );
		IBK::write_uint32_binary( out, versionNumberBinary(majorVersion, minorVersion) );
		IBK::write_uint32_binary( out, 0 );

	}
	else {

		IBK::write_uint32_binary( out, magicNumberFirstASCII );
		IBK::write_uint32_binary( out, magicNumberSecondASCII );
		unsigned int first, second;
		first = 0; second = 0; // this done to prevent windows environment from complaining
		toASCIIEncoding( versionNumberASCII(majorVersion, minorVersion), first, second );
		IBK::write_uint32_binary( out, first );
		IBK::write_uint32_binary( out, second );
	}
}


/*! Helper struct for functions toASCIIEncoding() and fromASCIIEncoding(). */
union exchangeType {
	unsigned int	dword;
	char			byte[4];
};


void Version::toASCIIEncoding( unsigned int version, unsigned int &first, unsigned int &second ) {


	// split numbers
	char * versionArray = new char[ 2*sizeof(unsigned int) ];

	// extract version numbers
	unsigned int major = majorFromVersionASCII( version );
	unsigned int minor = minorFromVersionASCII( version );

	// initialize
	first = 0;
	second = 0;

	// create hex array to base 10
	unsigned int counter = major;
	unsigned int rest = 0;
	unsigned int position = 0;
	unsigned int spacer = 3;
	while ( counter ) {

		// a mod 10 =>
		// a div a
		std::div_t divresult = std::div (counter,10);
		counter = divresult.quot;
		rest = divresult.rem;

		versionArray[ position ] = '0' + rest;
		++position;
		--spacer;

	}

	while( spacer ){
		versionArray[position] = '0';
		--spacer;
		++position;
	}


	spacer = 3;
	counter = minor;
	while ( counter ) {

		// a mod 10 =>
		// a div a
		std::div_t divresult = std::div (counter,10);
		counter = divresult.quot;
		rest = divresult.rem;

		versionArray[ position ] = '0' + rest;
		++position;
		--spacer;

	}

	while( spacer ){
		versionArray[position] = '0';
		--spacer;
		++position;
	}

	exchangeType temp;
	temp.byte[0] = 32;
	temp.byte[1] = versionArray[2];
	temp.byte[2] = versionArray[1];
	temp.byte[3] = versionArray[0];

	first = temp.dword;

	temp.byte[0] = 46;
	temp.byte[1] = versionArray[5];
	temp.byte[2] = versionArray[4];
	temp.byte[3] = versionArray[3];

	second = temp.dword;

	// clean up
	delete[] versionArray;

}


unsigned int Version::fromASCIIEncoding( unsigned int first, unsigned int second ){

	exchangeType tmp;
	tmp.dword = first;
	unsigned int major = (tmp.byte[1] - '0') * 100 + (tmp.byte[2] - '0') * 10 + (tmp.byte[3] - '0');

	tmp.dword = second;
	unsigned int minor = (tmp.byte[1] - '0') * 100 + (tmp.byte[2] - '0') * 10 + (tmp.byte[3] - '0');

	return ((major << 8) | minor);
}


} // namespace IBK

