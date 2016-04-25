#include "IBK_configuration.h"

#include <sstream>
#include <limits>

#include "IBK_Color.h"
#include "IBK_StringUtils.h"
#include "IBK_Exception.h"
#include "IBK_assert.h"

using namespace std;

namespace IBK {

Color Color::fromTColor(int tcolor) {
	IBK_ASSERT(tcolor >= 0);
	unsigned int alpha = 255;
	unsigned int blue = (tcolor & 0xFF0000) >> 16;
	unsigned int green = (tcolor & 0xFF00) >> 8;
	unsigned int red = tcolor & 0xFF;
	return Color(red, green, blue, alpha);
}

Color Color::fromQRgb(unsigned int qrgb) {
	unsigned int alpha = (qrgb & 0xFF000000) >> 24;
	unsigned int red = (qrgb & 0xFF0000) >> 16;
	unsigned int green = (qrgb & 0xFF00) >> 8;
	unsigned int blue = qrgb & 0xFF;
	return Color(red, green, blue, alpha);
}

Color Color::fromHtml(const std::string& html) {
	const char * const FUNC_ID = "[Color::fromHtml]";
	if( html[0] != '#') {
		std::stringstream tstr(html);
		unsigned int icolor;
		if( tstr >> icolor) {
			if( icolor > static_cast<unsigned int>(std::numeric_limits<int>::max()))
				return Color::fromQRgb(icolor);
			return Color::fromTColor(static_cast<int>(icolor));
		}
		throw IBK::Exception("Cannot convert string into a color.", FUNC_ID);
	}
	std::stringstream tstr(html.substr(1));
	unsigned int uitemp;
	if( !(tstr >> std::hex >> uitemp))
		throw IBK::Exception("Cannot convert string into a color.", FUNC_ID);
	return Color::fromQRgb(uitemp);
}

void Color::read(const std::string& data) {
	const char * const FUNC_ID = "[Color::read]";
	std::stringstream in(data);
	std::string line, keyword, value;
	try {
		// now read all the parameters
		while (getline(in, line)) {
			if (!IBK::extract_keyword(line, keyword, value))
				throw IBK::Exception( "Keyword expected" , FUNC_ID);
			else {
				// make it possible to read old style keywords
				if ( keyword == "COLOUR" || keyword == "COLOR") {
					try {
						*this = Color::fromHtml(value);
						// no transperant colors are allowed
						if ( m_alpha == 0 ) {
							m_alpha = 255;
						}
					}
					catch(std::exception&) {
						throw IBK::Exception( "Invalid parameter for keyword COLOUR.", FUNC_ID);
					}
				} // if ( keyword == "COLOUR" || keyword == "COLOR") {
			}
		}
	}
	catch (const IBK::Exception & ex) {
		throw IBK::Exception(ex, string("Error reading colour from data:\n") + data, FUNC_ID);
	}
}


void Color::write(std::ostream& out, unsigned int indent) const {
	std::string istr(indent, ' ');
	try {
		out << istr << std::setw(24) << std::left << "COLOUR" << " = " << toHtmlString() << '\n';
	}
	catch (const IBK::Exception & ex) {
		throw IBK::Exception(ex, string("Error writing IBK::Color to output stream."), "[Color::write]");
	}
}


unsigned int Color::toQRgb() const {

	unsigned int alpha = m_alpha << 24;
	unsigned int red = m_red << 16;
	unsigned int green = m_green << 8;
	unsigned int res = alpha + red + green + m_blue;

	return res;
}

int Color::toTColor() const {

	unsigned int alpha = m_alpha << 24;
	unsigned int blue = m_blue << 16;
	unsigned int green = m_green << 8;
	unsigned int res = alpha + blue + green + m_red;
	return res;
}

std::string Color::toHtmlString() const {
	unsigned int res = toQRgb();
	std::stringstream cstr;
	cstr << std::hex << res;
	std::string tscol("#");
	tscol += cstr.str();
	return tscol;
}

}  // namespace IBK

