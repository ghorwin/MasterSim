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

#include <iostream>
#include <sstream>
#include <limits>
#include <functional>
#include <stdexcept>
#include <iomanip>
#include <iterator>
#include <ctime>
#include <algorithm>


#include "IBK_LinearSpline.h"

namespace IBK {

LinearSpline::LinearSpline() :
	m_extrapolationMethod(EM_Constant),
	m_xMin(0),
	m_xMax(0),
	m_xStep(0),
	m_xOffset(0),
	m_valid(false)
{
}


unsigned int LinearSpline::size()  const {
	return m_y.size();
}


void LinearSpline::eliminateConsecutive(	const std::vector<double>& tmp_x,
											const std::vector<double>& tmp_y,
											std::vector<double>& tmp2_x,
											std::vector<double>& tmp2_y)
{
	// eliminate consecutive x values (stem from missing accuracy when reading the file)
	tmp2_x.clear();
	tmp2_y.clear();
	tmp2_x.push_back(tmp_x.front());
	tmp2_y.push_back(tmp_y.front());
	unsigned int skipped_values = 0;
	double last_reported_duplicate = std::numeric_limits<double>::infinity();
	for (unsigned int i=1, size = static_cast<unsigned int>(tmp_x.size()); i<size; ++i) {
		if (tmp_x[i] != tmp2_x.back()) {
			tmp2_x.push_back(tmp_x[i]);
			tmp2_y.push_back(tmp_y[i]);
		}
		else {
			if (tmp2_x.back() != last_reported_duplicate) {
				last_reported_duplicate = tmp2_x.back();
			}
			++skipped_values;
		}
	}
	// If the spline ends with a zero slope line sets y value to the original end y value
	if( tmp_x.back() == tmp2_x.back() && tmp_y.back() != tmp2_y.back()) {
		tmp2_y.back() = tmp_y.back();
	}
}


void LinearSpline::setValues(const std::vector<double> & xvals, const std::vector<double> & yvals) {
	if (xvals.size() != yvals.size())
		throw std::runtime_error("X and Y vector size mismatch.");
	if (xvals.empty())
		throw std::runtime_error("Input vectors are empty.");
	std::vector<double> tmp_x;
	std::vector<double> tmp_y;
	eliminateConsecutive(xvals, yvals, tmp_x, tmp_y);
	if (tmp_x.empty())
		throw std::runtime_error("Input vectors are empty.");
	setValues(tmp_x.begin(), tmp_x.end(), tmp_y.begin());
	std::string errstr;
	m_valid = makeSpline(errstr);
}

void LinearSpline::clear() {
	*this = IBK::LinearSpline();
}


void LinearSpline::swap(LinearSpline& spl) {
	m_x.swap(spl.m_x);
	m_y.swap(spl.m_y);
	m_slope.swap(spl.m_slope);
	std::swap(m_valid, spl.m_valid);
	std::swap(m_extrapolationMethod, spl.m_extrapolationMethod);
	std::swap(m_xMin, spl.m_xMin);
	std::swap(m_xMax, spl.m_xMax);
	std::swap(m_xStep, spl.m_xStep);
	std::swap(m_xOffset, spl.m_xOffset);
}


double LinearSpline::value(double x) const {

	if (m_x.size() == 1)
		return m_y[0];
	// x value larger than largest x value?
	if (x > m_x.back()) {
		switch (m_extrapolationMethod) {
			case EM_Constant	: return m_y.back();
			case EM_Linear		: return m_y.back() + m_slope.back()*(x-m_x.back());
		}
	}

	// we use lower bound to find the correct interval
	std::vector<double>::const_iterator it = std::lower_bound(m_x.begin(), m_x.end(), x);
	if (it == m_x.begin()) {
		switch (m_extrapolationMethod) {
			case EM_Constant	: return m_y.front();
			case EM_Linear		: return m_y.front() + m_slope.front()*(x-m_x.front());
		}
	}
	// get the index of the lower limit of the current interval
	unsigned int i = static_cast<unsigned int>(std::distance(m_x.begin(), it) - 1);
#ifdef USE_SLOPE
	return m_y[i] + m_slope[i]*(x - m_x[i]);
#else
	double alpha = (x - m_x[i])/(m_x[i+1]-m_x[i]); // thus must be always between 0 and 1
	return m_y[i]*(1-alpha) + m_y[i+1]*alpha;
#endif
}


double LinearSpline::slope(double x) const {
	if (!m_valid)
		throw std::runtime_error("Linear spline not properly initialized!");
	if (m_x.size() == 1)
		return 0;
	// lower_bound -> i = 0...n   -> subtract 1 to get the slope index
	// max i = n-2 for slope
	size_t i = std::lower_bound(m_x.begin(), m_x.end(), x) - m_x.begin();
	if (i <= 1)					return m_slope.front();
	else if (i < m_x.size()-1)	return m_slope[--i];
	else						return m_slope.back();
}

// *** PRIVATE FUNCTIONS ***

bool LinearSpline::makeSpline(std::string & errMsg) {
	m_valid = false;
	m_slope.clear();
	if (m_x.empty() || m_x.size() != m_y.size()) {
		errMsg = std::string("Invalid vector dimensions.");
		return false;
	}
	if (m_x.size() == 1) {
		// special case, constant spline
		m_valid = true;
		return true;
	}
	// check for strict monotonic increasing values, so if we find any
	// adjacent values that fulfil x[i] >= x[i+1], we have a problem
	std::vector<double>::iterator posIt = std::adjacent_find(m_x.begin(), m_x.end(), std::greater_equal<double>());
	if (posIt != m_x.end()) {
		return false;
	}
	for (unsigned int i=1; i<m_x.size(); ++i)
		m_slope.push_back( (m_y[i] - m_y[i-1])/(m_x[i]-m_x[i-1]) );
	m_valid = true;
	return true;
}


}	// namespace IBK

