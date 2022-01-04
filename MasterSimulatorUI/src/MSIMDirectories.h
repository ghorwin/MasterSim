#ifndef MSIMDirectoriesH
#define MSIMDirectoriesH

#include <QString>

/*! Provides default locations for resource and user data files based on IBK_DEPLOYMENT
	flag setting and OS.
	The IBK_DEPLOYMENT flag is set in IBK_BuildFlags.h, a file modified by the release scripts.
*/
class MSIMDirectories {
public:
	/*! Returns the platform-specific root directory of all read-only resource files. */
	static QString resourcesRootDir();

	/*! Returns the path to the application's translation file path. */
	static QString translationsFilePath(const QString & langID);

	/*! Returns the path to the Qt translation file path. */
	static QString qtTranslationsFilePath(const QString & langID);

	/*! Returns the platform-specific root directory of all user database files. */
	static QString userDataDir();

	/*! Returns path to global log file. */
	static QString globalLogFile();
};

#endif // MSIMDirectoriesH
