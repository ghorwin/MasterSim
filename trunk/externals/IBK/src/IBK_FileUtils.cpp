/*	IBK library
	Copyright (c) 2001-2016, Institut fuer Bauklimatik, TU Dresden, Germany

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

	This library contains derivative work based on other open-source libraries,
	see LICENSE and OTHER_LICENSES files.
*/

#include "IBK_FileUtils.h"

// for windows systems the windows header should be included as early as possible
#if defined(_WIN32)
	#include <Windows.h>
	#include <Shlobj.h>
	#include <direct.h>
#endif

#include <fstream>

#include "IBK_assert.h"
#include "IBK_Path.h"


namespace IBK {

unsigned int extract_ID_from_filename(const IBK::Path & fname) {
	IBK::Path fname_wo_ext = fname.withoutExtension();
	std::string::size_type pos_id = fname_wo_ext.str().rfind("_");
	if (pos_id == std::string::npos)
		return 0; // ID not found, return invalid ID
	std::string idstr = fname_wo_ext.str().substr(pos_id+1);
	std::stringstream strm(idstr);
	unsigned int id;
	if (strm >> id)
		return id;
	else
		return 0; // misformed/invalid ID
}

IBK::Path replace_ID_in_filename( const IBK::Path & fname, const unsigned int newId ) {

	std::string::size_type pos_id = fname.str().rfind("_");
	std::string::size_type pos_ext = fname.str().rfind(".");

	if ( pos_ext == std::string::npos)
		return IBK::Path(); // no extension found, return invalid path

	IBK_ASSERT(pos_ext > 0); // string must not start with .

	std::stringstream strm;
	if ( pos_id == std::string::npos ) {

		// compose new ID by inserting ID into name
		strm << fname.withoutExtension().str() << "_" << newId << "." << fname.extension();
	}
	else {
		std::string name = fname.str().substr(0, pos_id);
		strm << name << "_" << newId << "." << fname.extension();
	}
	return IBK::Path(strm.str());
}


std::vector<unsigned char> read_some_bytes(const IBK::Path& filename, unsigned int size, std::string& errmsg) {
	errmsg.clear();
	std::ifstream in;
	in.rdbuf()->pubsetbuf(0, 0);
#ifdef _MSC_VER
	in.open(filename.wstr().c_str(), std::ios_base::binary);
#else // _WIN32
	in.open(filename.str().c_str(), std::ios_base::binary);
#endif
	std::vector<unsigned char> result;
	if (!in) {
		errmsg = "Cannot open file";
		return result;
	}
	result.resize(size);
	std::streamsize resSize = in.readsome(reinterpret_cast<char*>(&result[0]), size);
	if (resSize < static_cast<std::streamsize>(size)) {
		result.erase( result.begin() + (int)resSize, result.end());
	}
	return result;
}


std::string file2String(const IBK::Path & fname) {
	std::ifstream in;
#if defined(_WIN32) && !defined(__MINGW32__)
	in.open(fname.wstr().c_str());
#else
	in.open(fname.c_str());
#endif // _WIN32
	std::stringstream strm;
	strm << in.rdbuf();
	return strm.str();
}


}  // namespace IBK

