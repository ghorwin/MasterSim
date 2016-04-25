#include "IBK_configuration.h"

#include <cmath>
#include <sstream>
#include <stdexcept>
#include <limits>

#include "IBK_math.h"
#include "IBK_Exception.h"
#include "IBK_FormatString.h"

namespace IBK {


#ifdef IBK_ENABLE_SAFE_MATH

const double POS_INF =	1e30;
const double NEAR_ZERO = 1e-10;
const double FUZZY_REL_TOLERANCE = 1e-12;

// Safe variant for std::pow function taking a double exponent
double f_pow(double base, double e) {
	// detect negative base error
	if (base < 0)
		throw IBK::Exception(IBK::FormatString("Negative base in power function, base = %1").arg(base), "[IBK::f_pow]");
	// detect overflow and throw exception
	if (e > 50)
		throw IBK::Exception(IBK::FormatString("Exponent too large in power function, exponent = %1").arg(e), "[IBK::f_pow]");
	return std::pow(base, e);
}

// Safe variant for std::pow function taking an integer exponent
double f_pow(double base, int e) {
	// detect negative base error
	if (base < 0)
		throw IBK::Exception(IBK::FormatString("Negative base in power function, base = %1.").arg(base), "[IBK::f_pow]");
	// detect overflow and throw exception
	if (e > 50)
		throw IBK::Exception(IBK::FormatString("Exponent too large in power function, exponent = %1.").arg(base), "[IBK::f_pow]");
	return std::pow(base, e);
}

// Safe variant for std::exp function
double f_exp(double x) {
	// TODO : add range check
	return std::exp(x);
}

// Safe variant for std::pow(10,x) function
double f_pow10(double e) {

	if (e > 50)
		throw IBK::Exception(IBK::FormatString("Exponent too large in power function, exponent = %1").arg(e), "[IBK::f_pow10]");

// fast pow10 function is not available on MINGW-GCC and APPLE MacOS 10.5 or non GNUC compilers
#if defined(__MINGW32__) || defined(__APPLE__) || !defined(__GNUC__)
	return std::pow(10,e);
#else
	return pow10(e);
#endif
}

// Safe variant for std::log function
double f_log(double x) {
	if (x<=0)
		throw IBK::Exception(IBK::FormatString("Negative or zero value in log function, x = %1").arg(x), "[IBK::f_log]");
	return std::log(x);
}

// Safe variant for std::log10 function
double f_log10(double x) {
	if (x<=0)
		throw IBK::Exception(IBK::FormatString("Negative or zero value in log function, x = %1").arg(x), "[IBK::f_log10]");
	return std::log10(x);
}

// Safe variant for std::sqrt function.
double f_sqrt(double x) {
	if (x<0)
		throw IBK::Exception(IBK::FormatString("Negative value in sqrt function, x = %1").arg(x), "[IBK::f_sqrt]");
	return std::sqrt(x);
}

#endif // IBK_ENABLE_SAFE_MATH

double scale(double x) {
#ifdef USE_COSINE
	return 0.5 - 0.5*std::cos(x*3.141592654);
#else //USE_COSINE
	double xinv = 1-x;
	// alternatively, you can also add an exponent to xinv
	return 1 + xinv*xinv*(-3 + 2*xinv);
#endif // USE_COSINE
}
// -----------------------------------------------------------------------------

double scale2(double x, double epsilon) {
	if (x <= 0)
		return 0;

	if (x >= epsilon)
		return 1;

	return 0.5 - 0.5 * std::cos(x * 3.141592654 / epsilon);
}

double scale2(double x) {
	return scale2(x, 1.0);
}
// -----------------------------------------------------------------------------


double error_function(double x) {
	// Implementation based on approximation,
	// see Abramowitz and Steger, 1964, Handbook of mathematical functions, Dover Publications, IBC, 1046p.
	double ra = std::fabs(x);
	double sign = ra/x;
	double b = 1;
	if (ra <= 0.5) {
		x = ra;
		double ex = std::exp(-x*x);
		double T = 1.0/(1.0 + 0.47047*x);
		b = 1.0 - ex*(T*(0.3480242 - T*(0.0958798 - T*0.7478556) ) );
	}
	return sign*b;
}
// -----------------------------------------------------------------------------


void min_max_values(const std::vector<double> & vec, double & minVal, double & maxVal) {
	minVal = std::numeric_limits<double>::max();
	maxVal = 0;
	for (unsigned int i=0; i<vec.size(); ++i) {
		if (minVal > vec[i])
			minVal = vec[i];
		if (maxVal < vec[i])
			maxVal = vec[i];
	}
}
// -----------------------------------------------------------------------------


} // namespace IBK
