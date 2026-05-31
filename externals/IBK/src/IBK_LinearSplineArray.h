/*	Copyright (c) 2001-2026, Institut für Bauklimatik, TU Dresden, Germany

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

#ifndef IBK_LinearSplineArrayH
#define IBK_LinearSplineArrayH

#include <string>
#include <vector>

namespace IBK {

/*! Stores multiple y-value datasets sharing a single strictly monotonically
	increasing x-axis and provides vectorized linear interpolation.

	A single call to interpolatedValue() performs one binary search on the
	shared x-axis and applies slope-based interpolation to every dataset,
	returning a reference to a pre-allocated internal result buffer.
*/
class LinearSplineArray {
public:

	/*! Sets the shared x-axis and all y-value datasets.
		Throws IBK::Exception if:
		- xvals is empty
		- any yvals[i].size() != xvals.size()
		- xvals is not strictly monotonically increasing
		\param xvals  Strictly monotonically increasing x-axis values.
		\param yvals  Outer index = dataset index, inner index = value index.
	*/
	void setValues(const std::vector<double> & xvals,
				   const std::vector<std::vector<double>> & yvals);

	/*! Combined variant: values[0] = x-axis, values[1..n-1] = y datasets.
		Requires at least 2 elements. Delegates to the two-argument overload.
	*/
	void setValues(const std::vector<std::vector<double>> & values);

	/*! Clears array data (does not reduce vector capacities, though). */
	void clear() { m_x.clear(); m_y.clear(); m_slopes.clear(); m_cachedResult.clear(); }

	/*! Returns true when successfully initialized via setValues(). */
	bool valid() const { return m_valid; }

	/*! Returns true when no data has been set. */
	bool empty() const { return m_x.empty(); }

	/*! Gives access to the raw data.
		This function is a convenience function to manipulate data directly.
		\warning DO NEVER RESIZE THE VECTORS!
	*/
	std::vector<std::vector<double>> y() { return m_y; }

	/*! Returns the number of y-datasets. */
	unsigned int datasetCount() const { return static_cast<unsigned int>(m_y.size()); }

	/*! Returns the number of data points on the shared x-axis. */
	unsigned int size() const { return static_cast<unsigned int>(m_x.size()); }

	/*! Performs vectorized interpolation at position x.
		- One binary search on the shared x-axis locates the interval.
		- Per-dataset: result[k] = m_y[k][i] + m_slopes[k][i] * (x - m_x[i])
		- Constant extrapolation outside the x range.
		- The returned reference is valid until the next call or setValues().
		\note Object must be valid(). In debug builds IBK_ASSERT_X fires if not.
	*/
	const std::vector<double> & interpolatedValue(double x) const;

	/*! Returns a non-interpolated value y at a given point x.
		For $x_i <= x < x_{i+1}$ the values $y_i$ is returned.
	*/
	const std::vector<double> & nonInterpolatedValue(double x) const;

	/*! Returns the global minimum and maximum over all datasets and all x-positions.
		If \a useAbsoluteValues is true, std::abs() is applied to every value before
		comparison, so both minValue and maxValue will be >= 0.
		\a minIndex and \a maxIndex receive the dataset (column) index where the
		respective extremum was found.
		\note Object must be valid().

		\a maxXindex and \a maxXIndex are set to the respective x indexes where the global
		minIndex and maxIndex values where found.
	*/
	void globalMinMaxValues(bool useAbsoluteValues, double & minValue, double & maxValue,
							unsigned int & minIndex, unsigned int & maxIndex,
							unsigned int & minXIndex, unsigned int & maxXIndex) const;

	/*! Returns the minimum and maximum of all datasets at a single x point,
		using non-interpolated (step) lookup via nonInterpolatedValue().
		If \a useAbsoluteValues is true, std::abs() is applied before comparison.
		\a minIndex and \a maxIndex receive the dataset (column) index where the
		respective extremum was found.
		\note Object must be valid().
	*/
	void localMinMaxValues(double x, bool useAbsoluteValues, double & minValue, double & maxValue,
						   unsigned int & minIndex, unsigned int & maxIndex) const;

	/*! Returns a const reference to the shared x-axis vector. */
	const std::vector<double> & x() const { return m_x; }

	/*! Returns a const reference to the y-value matrix (m_y[dataset][point]). */
	const std::vector<std::vector<double>> & y() const { return m_y; }

	/*! Fast O(1) swap of all data with \a other. */
	void swap(LinearSplineArray & other);

private:

	/*! Validates x-axis monotonicity and pre-calculates m_slopes from m_x/m_y.
		Sets m_valid = true on success.
		\param errMsg  Receives error description on failure.
		\return true on success.
	*/
	bool makeSpline(std::string & errMsg);

	/*! Shared x-axis values (strictly monotonically increasing). */
	std::vector<double>				m_x;

	/*! Y-value matrix: column-major format, m_y[dataset][point]. */
	std::vector<std::vector<double>> m_y;

	/*! Pre-calculated slope matrix: m_slopes[dataset][interval].
		m_slopes[k][i] = (m_y[k][i+1] - m_y[k][i]) / (m_x[i+1] - m_x[i]) */
	std::vector<std::vector<double>> m_slopes;

	/*! true after successful makeSpline(). */
	bool							m_valid = false;

	/*! Result cache for interpolatedValue(). Sized to datasetCount() once in
		makeSpline(); never reallocated during queries. */
	mutable std::vector<double>		m_cachedResult;
};

}  // namespace IBK

/*! \file IBK_LinearSplineArray.h
	\brief Contains the declaration of class LinearSplineArray.
*/

#endif // IBK_LinearSplineArrayH
