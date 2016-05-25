#include  "MSIMUIConstants.h"

const char * const ORG_NAME				= "IBK";
const char * const PROGRAM_NAME			= "MasterSim %1";
const char * const DOT_FILE_EXTENSION	= ".msim";

extern const char * const NEWS_URL		= "";

#if defined(Q_OS_MAC) // Q_OS_UNIX

const int TABLE_FONT_SIZE = 10;
const char * const FIXED_FONT_FAMILY = "Monaco";

#elif defined(Q_OS_UNIX)

const int TABLE_FONT_SIZE = 9;
const char * const FIXED_FONT_FAMILY = "Monospace";

#else

const int TABLE_FONT_SIZE = 9;
const char * const FIXED_FONT_FAMILY = "Courier New";

#endif
