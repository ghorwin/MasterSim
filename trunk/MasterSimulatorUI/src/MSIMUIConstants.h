#ifndef MSIMUIConstantsH
#define MSIMUIConstantsH

#include <QString>
class QTableWidget;

#include <MSIM_Constants.h>

/*! Organization name, also used as registry/settings key for saving the user. */
extern const char * const ORG_NAME;
/*! Program name, also used as registry/settings key for saving the user
	defined settings and the MRU list. */
extern const char * const PROGRAM_NAME;

/*! URL for news content to show on welcome page. */
extern const char * const NEWS_URL;

/*! Composes program name and version string. */
inline QString PROGRAM_VERSION_NAME() { return QString(PROGRAM_NAME).arg(MASTER_SIM::VERSION); }

/*! File extension including the dot (".msim"). */
extern const char * const DOT_FILE_EXTENSION;

/*! Font size to be used in tables. */
extern const int TABLE_FONT_SIZE;
/*! Fixed-size font family to be used in tables. */
extern const char * const FIXED_FONT_FAMILY;

/*! Utility function to ensure consistant table formatting. */
void formatTable(QTableWidget * tab);


#endif  // MSIMUIConstantsH
