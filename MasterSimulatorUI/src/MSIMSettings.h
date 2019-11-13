#ifndef MSIMSettingsH
#define MSIMSettingsH

#include <QCoreApplication>
#include <QDir>
#include <QObject>
#include <QVariant>
#include <QMap>

#include <IBK_ArgParser.h>
#include <IBK_messages.h>
#include <IBK_SolverArgsParser.h>
#include <IBK_BuildFlags.h>

/*! This class provides settings functionality, including:
	* read and write method of settings
	* generic parse functions for cmdline
	Essentially, this is a wrapper around QSettings that defines
	often used properties and handles saving/restoring of these
	properties.

	First thing in your application should be the instantiation
	of the Settings object.
	\code
	// initialize settings object
	MSIMSettings mySettings(organization, applicationName);
	// settings can be accessed application wide via the
	// singleton pattern
	MSIMSettings::instance().read();
	\endcode
*/
class MSIMSettings {
	Q_DECLARE_TR_FUNCTIONS(MSIMSettings)
	Q_DISABLE_COPY(MSIMSettings)
public:
	/*! Possible command line flags recognized by the program. */
	enum CmdLineFlags {
		/*! If set, the splash screen is not shown. */
		NoSplashScreen,
		/*! If set, the application is shown maximized. */
		FullScreen,
		/*! Number of flags. */
		NumCmdLineFlags
	};


	/*! Returns the instance of the singleton. */
	static MSIMSettings & instance();

	/*! Standard constructor.
		\param organization Some string defining the group/organization/company (major registry root name).
		\param appName Some string defining the application name (second part of registry root name).

		You may only instantiate one instance of the settings object in your application. An attempt to
		create a second instance will raise an exception in the constructor.
	*/
	MSIMSettings(const QString & organization, const QString & appName);

	/*! Destructor. */
	~MSIMSettings();

	/*! Sets default options (after first program start). */
	void setDefaults();

	/*! Adds MSIM-specific command line arguments to the arg parser. */
	void updateArgParser(IBK::ArgParser & argParser);

	/*! Overrides settings data with command line arguments.
		This function is called after setDefaults() and read(). It can be used
		to override settings read from configuration or default settings.
	*/
	void applyCommandLineArgs(const IBK::ArgParser & argParser);

	/*! Reads the user specific config data.
		The data is read in the usual method supported by the various platforms.
		The default implementation reads and populates all member variables.
	*/
	void read(QString regName = QString());

	/*! Reads the main window configuration properties.
		\param geometry A bytearray with the main window geometry (see QMainWindow::saveGeometry())
		\param state A bytearray with the main window state (toolbars, dockwidgets etc.)
					(see QMainWindow::saveState())
	*/
	void readMainWindowSettings(QByteArray &geometry, QByteArray &state);

	/*! Writes the user specific config data.
		The data is writted in the usual method supported by the various platforms.
		The default implementation writes all member variables.
		\param geometry A bytearray with the main window geometry (see QMainWindow::saveGeometry())
		\param state A bytearray with the main window state (toolbars, dockwidgets etc.)
					(see QMainWindow::saveState())
	*/
	void write(QByteArray geometry, QByteArray state);
	/*! Convenience function, checks for defined text editor executable, than spawns an external
		process and shows the file in the text editor.
	*/
	bool openFileInTextEditor(QWidget * parent, const QString & filepath) const;


	// ****** member variables ************

	/*! Organization ID name (used for QSettings).
		\note This value is NOT stored in the configuration and must not be altered by the user.
	*/
	QString						m_organization;

	/*! Application name (used for QSettings).
		\note This value is NOT stored in the configuration and must not be altered by the user.
	*/
	QString						m_appName;

	/*! Holds the file path to the project that
		should be opened during startup.

		By default, this value is set from m_lastProjectFile which is
		retrieved from the settings (i.e. registry) in read().
		However, if a project file was specified
		on the command line, the parseCmdLine() function will set
		this file instead.
		The separate property is needed in case that the command-line specified
		filename is invalid. In this case the m_lastProjectFile property is
		preserved and a start of the application without command line will
		correctly load the last successfully loaded project.

		\note This value is NOT stored in the configuration and must not be altered by the user.
	*/
	QString						m_initialProjectFile;

	/*! Installation path (without trailing backslash).
		This value is composed from the application file name in the setDefaults() function.
		\warning The install dir can be a relative path name.

		\note This value is NOT stored in the configuration and must not be altered by the user.
	*/
	QString						m_installDir;

	/*! Maximum number of files in the MRU (most-recently-used) files list. */
	unsigned int				m_maxRecentProjects;

	/*! Entries of the MRU list. */
	QStringList					m_recentProjects;

	/*! Maximum number of undo steps. */
	unsigned int				m_maxNumUNDOSteps;

	/*! Size of the thumb nail images shown on the welcome page. */
	unsigned int				m_thumbNailSize;

	/*! The file last opened in the UI.
		This value should be set by the application whenever a project
		file was successfully loaded or saved.
	*/
	QString						m_lastProjectFile;


	/*! Executable file name of external text editor.
		The text editor executable is determined automatically in setDefaults(),
		but can be customized by the user.
	*/
	QString						m_textEditorExecutable;

	/*! Current language ID (en, de, etc.)
		Can be interpreted by the application to load the respective translation files.
		This should be done before any UI component is instantiated.
		\note Language-ID might be empty, in which case the application should determine
			  the language ID automatically based on the current System locale.
	*/
	QString						m_langId;

	/*! Flags for command line settings. */
	bool						m_flags[NumCmdLineFlags];

	/*! Stores the logging threshold for console. */
	IBK::verbosity_levels_t		m_userLogLevelConsole;

	/*! Stores the logging threshold for the log file. */
	IBK::verbosity_levels_t		m_userLogLevelLogfile;

	/*! The version number stored during last run (used to detect version changes). */
	QString						m_lastVersionNumber;

	/*! Path to Post-Proc executable. */
	QString						m_postProcExecutable;

	/*! Enumeration values for different properties to be managed in settings.
	*/
	enum PropertyType {
		PT_LastImportDirectory,
		PT_LastFileOpenDirectory,
		NUM_PT
	};

	/*! Generic property map to store and retrieve configuration settings. */
	QMap<PropertyType, QVariant>		m_propertyMap;

	/*! Recursive directory search function.
		Collects list of all files in directory 'baseDir' and all its child directories that
		match any of the extensions in stringlist 'extensions'.
	*/
	static void recursiveSearch(QDir baseDir, QStringList & files, const QStringList & extensions);

	/*! Keywords used for serialization of the properties. */
	static const char * const			PROPERTY_KEYWORDS[NUM_PT];

private:

	/*! The global pointer to the MSIMSettings object.
		This pointer is set in the constructor, and cleared in the destructor.
	*/
	static MSIMSettings				*m_self;
};


#endif // MSIMSettingsH
