#ifndef MSIMUIConstantsH
#define MSIMUIConstantsH

#include <QString>

#include <MSIM_Constants.h>

/*! Organization name, also used as registry/settings key for saving the user. */
extern const char * const ORG_NAME;
/*! Program name, also used as registry/settings key for saving the user
	defined settings and the MRU list. */
extern const char * const PROGRAM_NAME;

/*! Composes program name and version string. */
inline QString PROGRAM_VERSION_NAME() { return QString(PROGRAM_NAME).arg(MASTER_SIM::VERSION); }

/*! File extension including the dot (".msim"). */
extern const char * const DOT_FILE_EXTENSION;

#endif  // MSIMUIConstantsH
