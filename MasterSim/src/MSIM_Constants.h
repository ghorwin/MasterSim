#ifndef MSIM_CONSTANTS_H
#define MSIM_CONSTANTS_H

namespace MASTER_SIM {

/*! Short version number (major.minor). */
extern const char * const VERSION;

/*! Long Version number. */
extern const char * const LONG_VERSION;

/*! Program name, also used as registry/settings key for saving the user
	defined settings and the MRU list. */
extern const char * const PROGRAM_NAME;

/*! Indentation of keyword */
extern const unsigned int KEYWORD_INDENTATION;
/*! Width of parameter value */
extern const unsigned int KEYWORD_WIDTH;

} // namespace MASTER_SIM

#endif // MSIM_CONSTANTS_H
