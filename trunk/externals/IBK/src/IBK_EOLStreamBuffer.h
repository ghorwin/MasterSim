#ifndef IBK_EOLStreamBufferH
#define IBK_EOLStreamBufferH

#include <streambuf>

namespace IBK {

/*! \brief Stream buffer class for filtering different EOL signs.
	This buffer can be used as buffer for input streams in order to convert different EndOfLine sequences
	into one usable ('\n').
	This is useful if one want read text files created at WindowsOS on a MacOS system.
	\code
	// create filestream and open file
	std::ifstream fin(fileName.c_str());
	// set eolfilter as new streambuffer
	IBK::EOLStreamBuffer eolfilter(in);
	// use filestream like before
	std::string text1, text2;
	in << text1 << text2;
	\endcode
	The stream buffer read directly from input device (no additional buffer).
*/
class EOLStreamBuffer : public std::streambuf {
public:
	/*! Standard constructor.
		\param in Input stream to be used
		It stores the old stream buffer and install itself as new streambuffer for in.
	*/
	explicit EOLStreamBuffer( std::ios& in );

	/*! Destructor.
		Sets the old streambuffer, stored in constructor, back to the stream.
	*/
	~EOLStreamBuffer();

protected:
	/*! New underflow function will be called in case of input buffer underflow.
		It makes the necessary conversion. The read pointer will not set back.
	*/
	virtual int_type underflow();

	/*! New uflow function will be called if the stream fetches a new character.
		It makes the necessary conversion.
		It calls underflow in case of a buffer underflow and sets the buffer read pointer to next position.
	*/
	virtual int_type uflow();

private:
	/*! Private copy constructor in order to prevent copy.*/
	EOLStreamBuffer(const EOLStreamBuffer& src);

	/*! Private copy assignment operator in order to prevent copy.*/
	EOLStreamBuffer& operator=(const EOLStreamBuffer&);

	/*! Typedef to identify base class type
		\todo Remove this type and use std::streambuf unless needed.
	*/
	typedef std::streambuf base_type;

	std::ios*		m_in;		///< Reference to input stream.
	std::streambuf* m_buf;		///< Pointer to original buffer from input stream.
	int_type		m_lastc1;	///< Last read char in underflow, initialized to 0 on construction.
	int_type		m_lastc2;	///< Last read char in uflow, initialized to 0 on construction.
};

} // namespace IBK

/*! \file IBK_EOLStreamBuffer.h
	\brief Contains declaration of class EOLStreamBuffer.
*/

#endif // IBK_EOLStreamBufferH
