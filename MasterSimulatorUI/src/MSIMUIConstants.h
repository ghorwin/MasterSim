#ifndef MSIMUIConstantsH
#define MSIMUIConstantsH

#include <QString>

class QTableWidget;
class QTableView;

#include <MSIM_Constants.h>

/*! Organization name, also used as registry/settings key for saving the user. */
extern const char * const ORG_NAME;

/*! File extension including the dot (".msim"). */
extern const char * const DOT_FILE_EXTENSION;

/*! Font size to be used in tables. */
extern const int TABLE_FONT_SIZE;
/*! Fixed-size font family to be used in tables. */
extern const char * const FIXED_FONT_FAMILY;

extern const char * const MASTERSIM_URL;
extern const char * const MASTERSIM_BUG_REPORT;

/*! URL to online help page, only need to add "_de.html" or "en_html" */
extern const char * const MASTERSIM_ONLINE_MANUAL_URL;

/*! Utility function to ensure consistant table formatting. */
void formatTable(QTableView * tab, bool caption = true);

#endif  // MSIMUIConstantsH
