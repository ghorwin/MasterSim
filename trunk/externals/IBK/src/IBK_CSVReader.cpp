#include "IBK_CSVReader.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <iterator>

#include "IBK_configuration.h"
#include "IBK_messages.h"
#include "IBK_StringUtils.h"
#include "IBK_FormatString.h"

using namespace std;

namespace IBK {

void CSVReader::read(const IBK::Path & filename) {
	const char * const FUNC_ID = "[CSVReader::read]";
	try {
#ifdef _MSC_VER
	std::ifstream in(filename.wstr().c_str());
#else // _WIN32
		std::ifstream in(filename.str().c_str());
#endif// _WIN32
		if (!in)
			throw IBK::Exception( IBK::FormatString("File doesn't exist or cannot open/access file."), FUNC_ID);

		std::string line;
		std::getline(in, line);
		IBK::explode(line, m_captions, m_separationCharacter);
		m_nColumns = m_captions.size();
		m_nRows = 0;
		while (std::getline(in, line)) {
			++m_nRows;
			std::vector<std::string> tokens;
			IBK::explode(line, tokens, m_separationCharacter);
			// error: wrong column size
			if(tokens.size() != m_nColumns) {
				throw IBK::Exception(IBK::FormatString("Wrong number of columns in line #%1!")
										.arg(m_nRows+1), FUNC_ID);
			}
			std::vector<double> values(m_nColumns);
			for (unsigned int i=0; i<m_nColumns; ++i) {
				try {
					values[i] = IBK::string2val<double>(tokens[i]);
				}
				catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString("Error reading value in column %1 in line #%2.")
										  .arg(i).arg(m_nRows+1), FUNC_ID);
				}
			}
			m_values.push_back(values);
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading file '%1'.").arg(filename), FUNC_ID);
	}
}
// ----------------------------------------------------------------------------


} // namespace IBK
