#include "MSIMSettings.h"

#include <IBK_SolverArgsParser.h>
#include <IBK_MessageHandler.h>
#include <IBK_messages.h>

#include <QLocale>
#include <QDesktopServices>
#include <QSettings>


MSIMSettings * MSIMSettings::m_self = NULL;

MSIMSettings & MSIMSettings::instance() {
	Q_ASSERT_X(m_self != NULL, "[MSIMSettings::instance]", "You must create an instance of "
		"MSIMSettings before accessing MSIMSettings::instance()!");
	return *m_self;
}


MSIMSettings::MSIMSettings(const QString & organization, const QString & appName) :
	m_organization(organization),
	m_appName(appName)
{
	// singleton check
	Q_ASSERT_X(m_self == NULL, "[MSIMSettings::MSIMSettings]", "You must not create multiple instances of "
		"classes that derive from MSIMSettings!");
	m_self = this;
}


MSIMSettings::~MSIMSettings() {
	m_self = NULL;
}


void MSIMSettings::setDefaults() {
	// compose default values

	// installation directory
	m_installDir = qApp->applicationDirPath();

	// clear last project file (if any, it is read in the read() function)
	m_lastProjectFile.clear();

	// set default log level
	m_userLogLevelConsole = IBK::VL_STANDARD;
	m_userLogLevelLogfile = IBK::VL_INFO;

	m_maxRecentProjects = 10;

	m_maxNumUNDOSteps = 10000;

	m_xSizeAtProgrammClose = 1024;
	m_ySizeAtProgrammClose = 768;

	m_flags[NoSplashScreen] = false;
	m_flags[FullScreen] = false;
}


void MSIMSettings::updateArgParser(IBK::ArgParser & argParser) {
	argParser.addFlag(0, "no-splash", "Disables splash screen.");
	argParser.addOption(0, "lang", "Specify the program language using a 2 character language ID.", "<en;de;it;...>", "en");
}


void MSIMSettings::applyCommandLineArgs(const IBK::ArgParser & argParser) {
	if (argParser.hasOption("no-splash"))
		m_flags[NoSplashScreen] = argParser.flagEnabled("no-splash");
	if (argParser.hasOption("lang"))
		m_langId = QString::fromStdString(argParser.option("lang"));
	// first positional argument is project file
	if (argParser.args().size() > 1)
		m_initialProjectFile = QString::fromStdString(argParser.args()[1]);
}


void MSIMSettings::read() {
//	const char * const FUNC_ID = "[MSIMSettings::read]";

	QSettings settings( m_organization, m_appName );
	m_xSizeAtProgrammClose = settings.value("LastXSize", m_xSizeAtProgrammClose ).toUInt();
	m_ySizeAtProgrammClose = settings.value("LastYSize", m_ySizeAtProgrammClose ).toUInt();

	m_lastProjectFile = settings.value("LastProjectFile", QString()).toString();
	m_recentProjects = settings.value("RecentProjects", QStringList()).toStringList();

	m_maxRecentProjects = settings.value("MaxRecentProjects", m_maxRecentProjects).toUInt();
	m_maxNumUNDOSteps = settings.value("MaxNumUndoSteps", m_maxNumUNDOSteps).toUInt();

	m_langId = settings.value("LangID", QString() ).toString();

	m_userLogLevelConsole = (IBK::verbosity_levels_t)settings.value("UserLogLevelConsole", m_userLogLevelConsole ).toInt();
	m_userLogLevelLogfile = (IBK::verbosity_levels_t)settings.value("UserLogLevelLogfile", m_userLogLevelLogfile ).toInt();
}


void MSIMSettings::write() {

	QSettings settings( m_organization, m_appName );
	settings.setValue("LastXSize", m_xSizeAtProgrammClose);
	settings.setValue("LastYSize", m_ySizeAtProgrammClose);
	settings.setValue("LastProjectFile", m_lastProjectFile);
	settings.setValue("RecentProjects", m_recentProjects);

	settings.setValue("MaxRecentProjects", m_maxRecentProjects );
	settings.setValue("UndoSize",m_maxNumUNDOSteps);

	settings.setValue("LangID", m_langId );

	settings.setValue("UserLogLevelConsole", m_userLogLevelConsole);
	settings.setValue("UserLogLevelLogfile", m_userLogLevelLogfile);
}

