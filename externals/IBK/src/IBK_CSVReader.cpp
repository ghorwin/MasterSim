/*	Copyright (c) 2001-2017, Institut für Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, H. Fechner, St. Vogelsang, A. Paepcke, J. Grunewald
	All rights reserved.

	This file is part of the IBK Library.

	Redistribution and use in source and binary forms, with or without modification,
	are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this
	   list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice,
	   this list of conditions and the following disclaimer in the documentation
	   and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its contributors
	   may be used to endorse or promote products derived from this software without
	   specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
	ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
	ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


	This library contains derivative work based on other open-source libraries.
	See OTHER_LICENCES and source code headers for details.

*/

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
#if defined(_WIN32)
	#if defined(_MSC_VER)
			std::ifstream in(filename.wstr().c_str());
	#else
			std::string filenameAnsi = IBK::WstringToANSI(filename.wstr(), false);
			std::ifstream in(filenameAnsi.c_str());
	#endif
#else // _WIN32
			std::ifstream in(filename.c_str());
#endif
		if (!in)
			throw IBK::Exception( IBK::FormatString("File doesn't exist or cannot open/access file."), FUNC_ID);

		std::string line;
		std::getline(in, line);
		IBK::explode(line, m_captions, m_separationCharacter);
		m_nColumns = (unsigned int)m_captions.size();
		m_nRows = 0;
		while (std::getline(in, line)) {
			++m_nRows; // also count empty rows, to get correct line numbers in error messages
			// skip empty rows
			if (line.empty() || line.find_first_not_of("\n\r\t ") == std::string::npos)
				continue;
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
	// store final number of rows
	m_nRows = m_values.size();
}
// ----------------------------------------------------------------------------


} // namespace IBK
