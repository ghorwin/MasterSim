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

#ifndef IBK_LinearSplineH
#define IBK_LinearSplineH

#include <vector>
#include <set>
#include <iosfwd>
#include <string>
#include <iterator>
#include <functional>

namespace IBK {

class ScalarFunction;

/*! This class encapsulates two data vectors containing a lookup table and linear
	interpolation functionality.

	The class LinearSpline can be thought of as a container of a tabulated function
	(containing the data points of that function in separate vectors m_x and m_y).
	In addition to the reading and writing functionality of such a container it
	provides the functionality to linearly interpolate between two data points using
	the member function value(). In order to do that efficiently the slopes between
	the points will be precalculated.

	Alternatively, a linear spline can also be generated with constant-spaced x values.
	In this case, the x-value vector does not exist, but instead the minX, maxX and
	XStep values are set.

	Use any of the setValues() functions to alter the data in the linear spline. Before
	using the linear spline for calculation, call makeSpline(). Only a successful call
	to makeSpline() will allow use of the functions value(), nonInterpolatedValue(),
	slope() and slopes().
*/
class LinearSpline {
public:

	/*! Possible return values of the generate() function. */
	enum SplineGenerationResults {
		SG_Ok,
		SG_MaxPointsExceeded
	};

	/*! Extrapolation technique to be used when querying values outside x range. */
	enum ExtrapolationMethod {
		/*! Use constant extrapolation. */
		EM_Constant,
		/*! Linearly extrapolate using slopes of first or last interval, respectively. */
		EM_Linear
	};

	/*! Default constructor, creates an empty spline. */
	LinearSpline();

	/*! Empties the spline.
		\deprecated Do not use this function anylonger, rather copy a newly created spline over the old one.
	*/
	void clear();

	/*! This generates the slopes-vector and updates the cached value. */
	bool makeSpline(std::string & errMsg);

	/*! Swaps the content of a linear spline with another linear spline quickly */
	void swap(LinearSpline& spl);

	/*! Returns the size of the linear spline. */
	unsigned int size() const;

	/*! Returns whether the spline is empty, that means no data is in the data vectors. */
	bool empty() const { return m_x.empty(); }

	/*! Returns whether the spline contains valid data.
		Only if this function returns true, you may use the functions value(),
		nonInterpolatedValue() and slopes().
	*/
	bool valid() const { return m_valid; }

	/*! Returns an interpolated value y at a given point x.
		If x is outside the range of x value in the spline the first or
		last y value is returned respectively.
	*/
	double value(double x) const;

	/*! Returns a non-interpolated value y at a given point x.
		For $x_i <= x < x_{i+1}$ the values $y_i$ is returned.
	*/
	double nonInterpolatedValue(double x) const;

	/*! Returns the slope at a given point x.
		If x is outside the x-value range the function returns 0.
	*/
	double slope(double x) const;

	/*! Returns a constant reference to a vector holding the slopes. */
	const std::vector<double> & slopes() const { return m_slope; }

	/*! Returns a constant reference to the vector holding the x values. */
	const std::vector<double> & x() const { return m_x; }

	/*! Returns a constant reference to the vector holding the y values. */
	const std::vector<double> & y() const { return m_y; }

	/*! Generic form to set the spline data with values from arbitrary containers.
		The function expects the range starting with firstY to be as long as the range
		firstX...lastX.
		This function performs consistency checks and throws an IBK::Exception if
		the requirements are not met.
		\note		The two different types are necessary in order to allow spline created from
					different container types for x and y values.
		\warning	The caller of the function has to ensure that both ranges have the same length.
	*/
	template <typename InputIteratorX, typename InputIteratorY>
	void setValues(InputIteratorX firstX, InputIteratorX lastX, InputIteratorY firstY) {
		clear();
		while(firstX != lastX) {
			m_x.push_back(*firstX++);
			m_y.push_back(*firstY++);
		}
		m_valid = false; // we default to non-initialized spline
	}

	/*! Sets the new spline values.
		Convenience function (special case of the generic setValues() function) for
		std::vector containers, calls the first setValues() internally.
		\note This function also removes consecutive x-values (x-values with same value)
			  automatically. If you do not want this functionality, use the first setValues()
			  function instead.

		Currently, if the x-values are not monotonically increasing, this function will set the spline state
		to invalid, but not cause an exception.
	*/
	void setValues(const std::vector<double> & xvals, const std::vector<double> & yvals);

	/*! Comparison operator == between two linear splines.
		Two splines are considered equal if the input data vectors m_x and m_y are equal. */
	bool operator==(const LinearSpline& rhs) const {
		return (m_x == rhs.m_x &&
				m_y == rhs.m_y &&
				m_extrapolationMethod == rhs.m_extrapolationMethod);
	}

	/*! Comparison operator != between two linear splines. */
	bool operator!=(const LinearSpline& rhs) const { return !(*this==rhs); }

	/*! The extrapolation method to be used. */
	ExtrapolationMethod	m_extrapolationMethod;


protected:

	/*! Eliminates consecutive x and y values.
		This function creates new vectors from the input vectors that won't contain
		the same x-values.
	*/
	static void eliminateConsecutive(const std::vector<double>& tmp_x,
									 const std::vector<double>& tmp_y,
									 std::vector<double>& tmp2_x,
									 std::vector<double>& tmp2_y);

	/*! For equidistant splines, xMin marks to smallest x-value. */
	double				m_xMin;

	/*! For equidistant splines, xMax marks to largest x-value. */
	double				m_xMax;

	/*! For equidistant splines, xStep marks to step size. */
	double				m_xStep;

	/*! For equidistant splines, xOffset the offset where xStep begins to calculate,
		typically xMin = xOffset. The following equation can be used to
		compute the corresponding x value to y-point i: x_i = xOffset + i*xStep
		\note m_xOffset must be <= xMin
	*/
	double				m_xOffset;

	/*! The x values of the spline data table. */
	std::vector<double>	m_x;

	/*! The y values of the spline data table. */
	std::vector<double>	m_y;

	/*! The pre-calculated slopes (generated).
		The slope m_slope[i] is the slope between points m_x[i] and m_x[i+1].
	*/
	std::vector<double> m_slope;

	/*! Flag indicating whether a spline was successfully initialized using makeSpline(). */
	bool m_valid;

};


}  // namespace IBK

/*! \file IBK_LinearSpline.h
	\brief Contains the declaration of the class LinearSpline, for linearly interpolated data tables.

	\example LinearSpline.cpp
	This is an example of how to use the LinearSpline.
*/

#endif // IBK_LinearSplineH
