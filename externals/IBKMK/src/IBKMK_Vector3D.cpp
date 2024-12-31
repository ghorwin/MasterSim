/*	IBK Math Kernel Library
	Copyright (c) 2001-today, Institut fuer Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, A. Paepcke, H. Fechner, St. Vogelsang
	All rights reserved.

	This file is part of the IBKMK Library.

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

#include "IBKMK_Vector3D.h"

#include <sstream>

namespace IBKMK {

/*! Converts a vector to a string in format "x y z". */
std::string Vector3D::toString() const {
	std::stringstream strm;
	strm << m_x << " " << m_y << " " << m_z;
	return strm.str();
}

/*! Converts a vector from a string in format "x y z". Throws an exception if parsing of numbers fails. */
Vector3D Vector3D::fromString(const std::string & vecString) {
	FUNCID(Vector3D::fromString);

	std::vector<double> vec;
	try {
		IBK::string2valueVector(vecString, vec);
	} catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Error parsing 3D vector from string '"+ vecString + "'", FUNC_ID);
	}
	if (vec.size() != 3)
		throw IBK::Exception("Size mismatch, expected 3 numbers.", FUNC_ID);
	Vector3D res;
	res.m_x = vec[0];
	res.m_y = vec[1];
	res.m_z = vec[2];
	return res;
}

} // namespace IBKMK

