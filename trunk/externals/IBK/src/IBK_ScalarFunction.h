#ifndef IBK_ScalarFunctionH
#define IBK_ScalarFunctionH

#include <functional>
#include <cmath>

namespace IBK {

/*! Abstract base class for the function objects taking one double as argument.
	\todo This class is currently only used by LinearSpline and by some examples of the IBKMK library. Since the
	derivative functionality is not needed by LinearSpline and only by minimization routines, this may be moved to
	the IBKMK library.
*/
class ScalarFunction : public std::unary_function<double, double> {
public:

	ScalarFunction() {}

	/*! Virtual destructor to ensure destruction of data members of derived classes.*/
	virtual ~ScalarFunction() {}

	/*! Actual implementation of the function encapsulated in this function object.
		You have to reimplement this function in your derived class!
	*/
	virtual double operator() (double x) const = 0;

	/*! Derivative of this function, calculated by using a difference quotient.
		Reimplement this function in your derived class if you know the derivative better!
	*/
	virtual double df(double x) const {
		double f1 = operator()(x);
		double h = 1e-8 * std::fabs(x);
		if (h == 0)
			h = 1e-8;
		double xh = x - h;
		h = xh - x;
		double f2 = operator()(xh);
		return (f2-f1)/h;
	}
};

}  // namespace IBK

/*! \file IBK_ScalarFunction.h
	\brief Contains declaration of class ScalarFunction.
*/

#endif // IBK_ScalarFunctionH

