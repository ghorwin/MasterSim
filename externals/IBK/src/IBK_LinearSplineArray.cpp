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

#include <algorithm>
#include <cmath>
#include <functional>
#include <iterator>

#include "IBK_LinearSplineArray.h"
#include "IBK_Exception.h"
#include "IBK_FormatString.h"
#include "IBK_assert.h"

namespace IBK {

void LinearSplineArray::setValues(const std::vector<double> & xvals,
								  const std::vector<std::vector<double>> & yvals)
{
	FUNCID(LinearSplineArray::setValues);
	if (xvals.empty())
		throw IBK::Exception("X-axis vector is empty.", FUNC_ID);
	if (yvals.empty())
		throw IBK::Exception("Y-value dataset list is empty.", FUNC_ID);
	for (unsigned int i = 0; i < yvals.size(); ++i) {
		if (yvals[i].size() != xvals.size())
			throw IBK::Exception(IBK::FormatString("Size mismatch in dataset %1: "
				"expected %2 values, got %3.")
				.arg(i).arg((unsigned int)xvals.size()).arg((unsigned int)yvals[i].size()),
				FUNC_ID);
	}
	m_x = xvals;
	m_y = yvals;
	m_valid = false;
	std::string errMsg;
	if (!makeSpline(errMsg))
		throw IBK::Exception(IBK::FormatString("Error generating spline: %1").arg(errMsg), FUNC_ID);
}


void LinearSplineArray::setValues(const std::vector<std::vector<double>> & values) {
	FUNCID(LinearSplineArray::setValues);
	if (values.size() < 2)
		throw IBK::Exception("Need at least 2 vectors (x-axis + one y-dataset).", FUNC_ID);
	const std::vector<double> & xvals = values[0];
	std::vector<std::vector<double>> yvals;
	yvals.reserve(values.size() - 1);
	for (unsigned int i = 1; i < values.size(); ++i)
		yvals.push_back(values[i]);
	try {
		setValues(xvals, yvals);
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Error setting values from combined vector.", FUNC_ID);
	}
}


const std::vector<double> & LinearSplineArray::interpolatedValue(double x) const {
	IBK_ASSERT_X(m_valid, "LinearSplineArray not properly initialized. Call setValues() first!");

	const unsigned int d = static_cast<unsigned int>(m_y.size());

	// special case: single point
	if (m_x.size() == 1) {
		for (unsigned int k = 0; k < d; ++k)
			m_cachedResult[k] = m_y[k][0];
		return m_cachedResult;
	}

	// constant extrapolation above
	if (x > m_x.back()) {
		for (unsigned int k = 0; k < d; ++k)
			m_cachedResult[k] = m_y[k].back();
		return m_cachedResult;
	}

	// binary search for interval
	std::vector<double>::const_iterator it = std::lower_bound(m_x.begin(), m_x.end(), x);

	// constant extrapolation below
	if (it == m_x.begin()) {
		for (unsigned int k = 0; k < d; ++k)
			m_cachedResult[k] = m_y[k].front();
		return m_cachedResult;
	}

	// interpolate
	unsigned int i = static_cast<unsigned int>(std::distance(m_x.begin(), it) - 1);
	double dx = x - m_x[i];
	for (unsigned int k = 0; k < d; ++k)
		m_cachedResult[k] = m_y[k][i] + m_slopes[k][i] * dx;
	return m_cachedResult;
}


const std::vector<double> &LinearSplineArray::nonInterpolatedValue(double x) const {
	IBK_ASSERT_X( m_valid, "Linear spline not properly initialized. Call makeSpline() first! nonInterpolatedValue()" );

	const unsigned int d = static_cast<unsigned int>(m_y.size());
	if (m_x.size() == 1) {
		for (unsigned int k = 0; k < d; ++k)
			m_cachedResult[k] = m_y[k][0];
		return m_cachedResult;
	}
	// x value larger than largest x value?
	if (x > m_x.back()) {
		for (unsigned int k = 0; k < d; ++k)
			m_cachedResult[k] = m_y[k].back();
		return m_cachedResult;
	}
	// we use lower bound to find the correct interval
	std::vector<double>::const_iterator it = std::lower_bound(m_x.begin(), m_x.end(), x);
	if (it == m_x.begin()) {
		for (unsigned int k = 0; k < d; ++k)
			m_cachedResult[k] = m_y[k].front();
		return m_cachedResult;
	}
	// get the index of the lower limit of the current interval
	unsigned int i = static_cast<unsigned int>(std::distance(m_x.begin(), it) - 1);
	// special case: x is a saltus of the non-interpolated spline
	// (than equals x the element m_x[i+1])
	if ( i < m_x.size() - 1 && x == m_x[i+1] ) {
		for (unsigned int k = 0; k < d; ++k)
			m_cachedResult[k] = m_y[k][i+1];
		return m_cachedResult;
	}

	for (unsigned int k = 0; k < d; ++k)
		m_cachedResult[k] = m_y[k][i];
	return m_cachedResult;
}


void LinearSplineArray::globalMinMaxValues(bool useAbsoluteValues, double & minValue, double & maxValue,
										   unsigned int & minIndex, unsigned int & maxIndex,
										   unsigned int & minXIndex, unsigned int & maxXIndex) const
{
	IBK_ASSERT_X(m_valid, "LinearSplineArray not properly initialized. Call setValues() first!");

	const unsigned int d = static_cast<unsigned int>(m_y.size());
	const unsigned int n = static_cast<unsigned int>(m_x.size());

	double firstVal = useAbsoluteValues ? std::abs(m_y[0][0]) : m_y[0][0];
	minValue = firstVal;
	maxValue = firstVal;
	minIndex = 0;
	maxIndex = 0;
	minXIndex = 0;
	maxXIndex = 0;

	for (unsigned int k = 0; k < d; ++k) {
		for (unsigned int i = 0; i < n; ++i) {
			double v = useAbsoluteValues ? std::abs(m_y[k][i]) : m_y[k][i];
			if (v < minValue) { minValue = v; minIndex = k; minXIndex=i; }
			if (v > maxValue) { maxValue = v; maxIndex = k; maxXIndex=i; }
		}
	}
}


void LinearSplineArray::localMinMaxValues(double x, bool useAbsoluteValues,
										  double & minValue, double & maxValue,
										  unsigned int & minIndex, unsigned int & maxIndex) const
{
	IBK_ASSERT_X(m_valid, "LinearSplineArray not properly initialized. Call setValues() first!");

	const std::vector<double> & vals = nonInterpolatedValue(x);

	double firstVal = useAbsoluteValues ? std::abs(vals[0]) : vals[0];
	minValue = firstVal;
	maxValue = firstVal;
	minIndex = 0;
	maxIndex = 0;

	for (unsigned int k = 1; k < vals.size(); ++k) {
		double v = useAbsoluteValues ? std::abs(vals[k]) : vals[k];
		if (v < minValue) { minValue = v; minIndex = k; }
		if (v > maxValue) { maxValue = v; maxIndex = k; }
	}
}

void LinearSplineArray::swap(LinearSplineArray & other) {
	m_x.swap(other.m_x);
	m_y.swap(other.m_y);
	m_slopes.swap(other.m_slopes);
	m_cachedResult.swap(other.m_cachedResult);
	std::swap(m_valid, other.m_valid);
}


bool LinearSplineArray::makeSpline(std::string & errMsg) {
	m_valid = false;
	m_slopes.clear();

	if (m_x.empty()) {
		errMsg = "X-axis vector is empty.";
		return false;
	}

	const unsigned int d = static_cast<unsigned int>(m_y.size());

	// single point: no slopes needed
	if (m_x.size() == 1) {
		m_slopes.resize(d);
		m_cachedResult.assign(d, 0.0);
		m_valid = true;
		return true;
	}

	// check for strict monotonic increasing values
	std::vector<double>::iterator posIt = std::adjacent_find(m_x.begin(), m_x.end(),
															  std::greater_equal<double>());
	if (posIt != m_x.end()) {
		errMsg = IBK::FormatString("X values are not strictly monotonic increasing "
			"(at position/index %1).").arg((unsigned int)(posIt - m_x.begin())).str();
		return false;
	}

	// pre-compute slopes for each dataset
	unsigned int intervals = static_cast<unsigned int>(m_x.size()) - 1;
	m_slopes.resize(d);
	for (unsigned int k = 0; k < d; ++k) {
		m_slopes[k].resize(intervals);
		for (unsigned int i = 0; i < intervals; ++i)
			m_slopes[k][i] = (m_y[k][i+1] - m_y[k][i]) / (m_x[i+1] - m_x[i]);
	}

	// size the result cache once; never reallocated during queries
	m_cachedResult.assign(d, 0.0);
	m_valid = true;
	return true;
}

}  // namespace IBK
