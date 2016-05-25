#ifndef MSIMDirectoriesH
#define MSIMDirectoriesH

#include <QString>

/*! Provides default locations for resource and user data files based on IBK_DEPLOYMENT
	flag setting and OS.
*/
class MSIMDirectories {
public:
	/*! Returns the platform-specific root directory of all read-only resource files. */
	static QString resourcesRootDir();

	/*! Returns the platform-specific directory of all read-only translation files. */
	static QString translationsDir();

	/*! Returns the platform-specific root directory of all user database files. */
	static QString userDataDir();

	/*! Returns path to global log file. */
	static QString globalLogFile();
};

#endif // MSIMDirectoriesH
