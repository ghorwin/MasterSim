#ifndef IBK_openMPH
#define IBK_openMPH

#if defined(_OPENMP)

// we have openmp
#include <omp.h>


#else

// no openmp, disable warnings

#	if defined(__GNUC__)

#pragma GCC diagnostic ignored "-Wunknown-pragmas"

#	 elif  defined(__ICC) || defined(__INTEL_COMPILER)

// icc warning disabling works with icc version 14.0 - what is the switch for this?
#pragma warning ( disable : 3180 )

#	elif defined(_MSC_VER)

#pragma warning (disable : 4068 ) /* disable unknown pragma warnings */

#	endif


#endif // _OPENMP

/*! \file IBK_openMP.h
	\brief Replacement include file for OpenMP-based library code that includes omp.h and also sets defines to disable
			compiler warnings if not compiling with OpenMP support.
*/

#endif // IBK_openMPH
