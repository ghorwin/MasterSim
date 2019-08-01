#ifndef MSIMPostProcBindingsH
#define MSIMPostProcBindingsH


#ifdef _WIN32
	#undef UNICODE
	#include <windows.h>
#endif

#include <string>

#include <QProcess>
#include <QString>

#include <IBK_Path.h>

/*! This class encapsulates everything related to PostProc interaction functionality. */
class MSIMPostProcBindings {
public:

	/*! This function generates and returns the path to the default session file. */
	static IBK::Path defaultSessionFilePath(const QString & projectFile);

	/*! This function generates the default post-proc session file using information from
		the project.
	*/
	static void generateDefaultSessionFile(const QString & projectFile);
};


/*! Wrapper around window-specific process handling and Linux/Mac functionality.
*/
class MSIMPostProcHandler {
public:
	MSIMPostProcHandler();

	/*! Tests, if a post proc process is already active.
		If a process is active, it will bring the postproc main window to top.
		If the process is not active, it will return 1;
		In case of any WINAPI error (or *nix problem), the function returns 2.
		The function returns 0 if the postproc window could be shown correctly.
	*/
	int reopenIfActive();

	/*! Spawns a new instance of the postprocessing and passes
		the session file if not empty (only for PostProc2).
		\return Returns true, if the process could be created successfully.
	*/
	bool spawnPostProc(const std::string & sessionFile);

#ifdef _WIN32
	HANDLE m_postProcHandle;
#else
	qint64 m_postProcHandle; // process ID on linux/mac
#endif
};


#endif // MSIMPostProcBindingsH
