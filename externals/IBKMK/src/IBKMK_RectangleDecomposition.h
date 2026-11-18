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

#ifndef IBKMK_RectDecomposition_H
#define IBKMK_RectDecomposition_H

#include <set>
#include <vector>
#include <string>

namespace IBKMK {

struct Rectangle {
	int			m_x1;
	int			m_y1;
	int			m_x2;
	int			m_y2;
	std::string	m_name;
};

/*!
 * This algorithm implements the Graph-Based Decomposition (page 6 of appended source).
 * input takes a set of x/y coordinates of elements contained by a shape.
 * output returns a set of corner elements ((x_min, y_min), (x_max, y_max)) of the rectangles.
 *
 *  https://library.utia.cas.cz/separaty/2012/ZOI/suk-rectangular%20decomposition%20of%20binary%20images.pdf
*/
void rectDecomposition(std::set<std::pair<unsigned int,unsigned int>> & input,
					   std::set<std::pair<std::pair<unsigned int,unsigned int>,std::pair<unsigned int, unsigned int>>> & output);

/*!
 * This algorithm implements a simple greedy decomposition algorithm.
 * \param input Takes a vector of vector (grid) of strings. The string is the name or content of the cell
 * \return A vector of rectangles.
 *
 *
*/
std::vector<Rectangle> rectDecomposition_Greedy(const std::vector<std::vector<std::string>>& input);

} // namespace IBKMK

#endif // IBKMK_RectDecomposition_H
