#ifndef MSIM_VERSION_H
#define MSIM_VERSION_H

/*	The single place where the MasterSim version number and release date are defined.

	This file contains only preprocessor defines, since it is also included by the
	Windows resource scripts (*.rc) to set the version info of the executables.
	In C++ code use the constants in MSIM_Constants.h instead.

	Release scripts read the version number from the three MSIM_VERSION_xxx defines below,
	so keep the format '#define MSIM_VERSION_xxx <number>'.
*/

#define MSIM_VERSION_MAJOR 0
#define MSIM_VERSION_MINOR 9
#define MSIM_VERSION_PATCH 8

#define MSIM_RELEASE_DATE "December 31st, 2024"

// Stringification helpers; produce a single string literal (e.g. "0.9.8"), which is
// required by the resource compilers (no concatenation of adjacent string literals).
#define MSIM_STR_HELPER(x) #x
#define MSIM_STR(x) MSIM_STR_HELPER(x)

/*! Short version number as string, e.g. "0.9". */
#define MSIM_VERSION_STR MSIM_STR(MSIM_VERSION_MAJOR.MSIM_VERSION_MINOR)
/*! Long version number as string, e.g. "0.9.8". */
#define MSIM_LONG_VERSION_STR MSIM_STR(MSIM_VERSION_MAJOR.MSIM_VERSION_MINOR.MSIM_VERSION_PATCH)

#endif // MSIM_VERSION_H
