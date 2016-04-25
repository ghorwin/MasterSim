#include "IBK_Constants.h"

namespace IBK {

const unsigned int INVALID_ELEMENT = 0xFFFFFFFF;

const char * const VERSION = "4.2.1";


const char * const  MONTH_NAMES[12]         = { "Jan", "Feb", "Mar", "Apr",
												"May", "Jun", "Jul", "Aug",
												"Sep", "Oct", "Nov", "Dec"};

unsigned int SECONDS_PER_MONTH[12] 			= { 31*86400, 28*86400, 31*86400, 30*86400,
												31*86400, 30*86400, 31*86400, 31*86400,
												30*86400, 31*86400, 30*86400, 31*86400 };

const unsigned int SECONDS_PER_YEAR = 31536000;

const unsigned int SECONDS_PER_DAY = 86400;

const unsigned int USER_ID_START = 1 << 11;

const unsigned int VOID_ID = (unsigned int)(-1);


const char * const PLACEHOLDER_PROJECT_DIR = "Project Directory";

const char * const PLACEHOLDER_DATABASE_ROOT = "Database Root";

const char * const PLACEHOLDER_MATERIALS_DIR = "Material Database";

const char * const PLACEHOLDER_CLIMATE_DIR = "Climate Database";

} // namespace IBK

