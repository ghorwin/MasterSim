#ifndef IBK_MultiSplineH
#define IBK_MultiSplineH

#include "IBK_LinearSpline.h"

#include <vector>

namespace IBK {


/*! Class for using spline values with two definition values (like a plane in 3D space).*/
class MultiSpline {
public:
	/*! Create a non valid object. Cannot be used befor callinf set.*/
	MultiSpline() = default;

	/*! Set the internal database for the spline values.
		\param baseValues A vector of values for the first base (x1).
		\param values Vector of pairs of vectors for the values. The size of the outer vector must be the same as the size of the first base vector (\a basevalues).
		The first part of the pair is the second base (x2) and the second part of the pair the values to be used (y).
		It represents a function like this:   y = f(x1,x2).
	*/
	void set(const std::vector<double> baseValues, const std::vector<std::pair<std::vector<double>,std::vector<double>>>& values);

	/*! It returns the value for the given first and second base values (x1 and x2).
	 *  \param x1 First base (see baseValues in set()).
	 *  \param x2 Second base (see first part of the pair of values in set()).
	*/
	double value(double x1, double x2);

private:

	std::vector<double>				m_baseValues;	///< Vector for first base (x1)
	std::vector<IBK::LinearSpline>	m_values;		///< Vector of spline with second base (x2) and values (y)

	/*! Find out the place of the given x1 value in the first base vector.
	 *  The result show left and right bound of the indices in the vector. The indices can be equel in case the value lays exactly at a vector value.
		Internal function which is used from value().
	*/
	std::pair<size_t,size_t> indexArea(double x1);
};

} // namespace IBK

#endif // IBK_MultiSplineH
