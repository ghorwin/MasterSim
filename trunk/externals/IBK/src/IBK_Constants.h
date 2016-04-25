#ifndef IBK_constantsH
#define IBK_constantsH

namespace IBK {

/*! used to specify a unused or invalid item index. */
extern const unsigned int INVALID_ELEMENT;

/*! The library version (full version number). */
extern const char * const VERSION;

/*! The seconds in each of the twelve months. */
extern unsigned int SECONDS_PER_MONTH[12];

/*! The abbreviated names of the months for date In/Output. */
extern const char * const MONTH_NAMES[12];

/*! Seconds in a regular year. */
extern const unsigned int SECONDS_PER_YEAR;

/*! Seconds per day. */
extern const unsigned int SECONDS_PER_DAY;

/*! Current start id for user space in the various databases. */
extern const unsigned int USER_ID_START;

/*! ID used to identify VOID types, for example VOID materials.
	\warning Changing this constant may cause problems with older project/data files that store
	the old IDs and new software versions would not recognize these materials as VOIDs.
*/
extern const unsigned int VOID_ID;


/*! \todo rename to something related to materials. */
enum materialDBTypes {
	MATDB_TYPE_DEFAULT,
	MATDB_TYPE_ALL,
	MATDB_TYPE_COND,
	MATDB_TYPE_DELPHIN5,
	MATDB_TYPE_DELPHIN6
};

/*! Default placeholder name to be used to reference the project directory. */
extern const char * const PLACEHOLDER_PROJECT_DIR;
/*! Default placeholder name to be used to reference the root directory of all databases. */
extern const char * const PLACEHOLDER_DATABASE_ROOT;
/*! Default placeholder name to be used to reference the material database directory. */
extern const char * const PLACEHOLDER_MATERIALS_DIR;
/*! Default placeholder name to be used to reference the climate database directory. */
extern const char * const PLACEHOLDER_CLIMATE_DIR;

} // namespace IBK

/*! \file IBK_Constants.h
	\brief Program and library constants.
*/

#endif // IBK_constantsH
