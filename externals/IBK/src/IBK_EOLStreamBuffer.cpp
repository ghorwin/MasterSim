#include "IBK_EOLStreamBuffer.h"

#include <iostream>

namespace IBK {

EOLStreamBuffer::EOLStreamBuffer( std::ios& in )
	: base_type(),
	  m_in(&in),
	  m_buf( in.rdbuf() ),
	  m_lastc1(),
	  m_lastc2()
{
	in.rdbuf(this);
}

EOLStreamBuffer::~EOLStreamBuffer() {
	m_in->rdbuf(m_buf);
}

EOLStreamBuffer::int_type EOLStreamBuffer::underflow() {
	for ( int_type c = m_buf->sgetc();;c = m_buf->sgetc()) {
		if ( traits_type::eq_int_type( c, traits_type::eof()))
			return traits_type::eof();
		if ( c == 0x1e) {
			if(m_lastc1 == c) {
				m_lastc1 = 0;
				continue;
			}
			else {
				m_lastc1 = c;
				return '\n';
			}
		}
		// \r\n -> skip character
		if (c == '\n' && m_lastc1 == '\r' ) {
			m_lastc1 = 0;
			continue;
		}
		// \n\r -> skip character
		if (c == '\r' && m_lastc1 == '\n' ) {
			m_lastc1 = 0;
			continue;
		}
		// \n or \r -> always return \n
		if (c == '\r' || c == '\n' ) {
			m_lastc1 = c;
			return'\n';
		}
		else {
			m_lastc1 = 0;
			return c;
		}
	}
}

EOLStreamBuffer::int_type EOLStreamBuffer::uflow() {
	for ( int_type c = m_buf->sbumpc();;c = m_buf->sbumpc()) {
		if ( traits_type::eq_int_type( c, traits_type::eof()))
			return traits_type::eof();
		if ( c == 0x1e) {
			if(m_lastc1 == c) {
				m_lastc1 = 0;
				continue;
			}
			else {
				m_lastc1 = c;
				return '\n';
			}
		}
		if (c == '\n' && m_lastc1 == '\r' ) {
			m_lastc1 = 0;
			continue;
		}
		if (c == '\r' && m_lastc1 == '\n' ) {
			m_lastc1 = 0;
			continue;
		}
		if (c == '\r' || c == '\n' ) {
			m_lastc1 = c;
			return'\n';
		}
		else {
			m_lastc1 = 0;
			return c;
		}
	}
}

} // namespace IBK
