#include "MSIMSettings.h"

#include <IBK_SolverArgsParser.h>
#include <IBK_MessageHandler.h>
#include <IBK_messages.h>

#include <QLocale>
#include <QDesktopServices>
#include <QSettings>
#include <QProcess>
#include <QMessageBox>

#include "MSIMConversion.h"

MSIMSettings * MSIMSettings::m_self = NULL;

const char * const					MSIMSettings::PROPERTY_KEYWORDS[MSIMSettings::NUM_PT] = {
	"LastImportDirectory",
	"LastFileOpenDirectory"
};


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

	// determine text executable
	m_textEditorExecutable.clear();
#ifdef Q_OS_UNIX
	m_textEditorExecutable = "gedit";
#elif defined(Q_OS_WIN)
	m_textEditorExecutable = "C:\\Program Files (x86)\\Notepad++\\notepad++.exe";
#else
	// OS x editor?
#endif

	/// \todo Implement default text editor detection

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
	if (argParser.args().size() > 1){
		std::string dummy = argParser.args()[1];
		m_initialProjectFile = utf82QString(dummy);
	}
}


void MSIMSettings::read() {
//	const char * const FUNC_ID = "[MSIMSettings::read]";

	QSettings settings( m_organization, m_appName );
	m_lastProjectFile = settings.value("LastProjectFile", QString()).toString();
	m_recentProjects = settings.value("RecentProjects", QStringList()).toStringList();

	m_maxRecentProjects = settings.value("MaxRecentProjects", m_maxRecentProjects).toUInt();
	m_maxNumUNDOSteps = settings.value("MaxNumUndoSteps", m_maxNumUNDOSteps).toUInt();

	QString tmpTextEditorExecutable = settings.value("TextEditorExecutable", m_textEditorExecutable ).toString();
	if (!tmpTextEditorExecutable.isEmpty())
		m_textEditorExecutable = tmpTextEditorExecutable;
	m_langId = settings.value("LangID", QString() ).toString();

	m_userLogLevelConsole = (IBK::verbosity_levels_t)settings.value("UserLogLevelConsole", m_userLogLevelConsole ).toInt();
	m_userLogLevelLogfile = (IBK::verbosity_levels_t)settings.value("UserLogLevelLogfile", m_userLogLevelLogfile ).toInt();

	for (unsigned int i=0; i<NUM_PT; ++i) {
		QVariant var = settings.value(PROPERTY_KEYWORDS[i], QVariant());
		if (var.isValid())
			m_propertyMap.insert((PropertyType)i, var);
	}
}


void MSIMSettings::readMainWindowSettings(QByteArray &geometry, QByteArray &state) {
	QSettings settings( m_organization, m_appName );
	geometry = settings.value("MainWindowGeometry", QByteArray()).toByteArray();
	state = settings.value("MainWindowState", QByteArray()).toByteArray();
}


void MSIMSettings::write(QByteArray geometry, QByteArray state) {

	QSettings settings( m_organization, m_appName );
	settings.setValue("LastProjectFile", m_lastProjectFile);
	settings.setValue("RecentProjects", m_recentProjects);

	settings.setValue("MaxRecentProjects", m_maxRecentProjects );
	settings.setValue("UndoSize",m_maxNumUNDOSteps);

	settings.setValue("TextEditorExecutable", m_textEditorExecutable );
	settings.setValue("LangID", m_langId );

	settings.setValue("UserLogLevelConsole", m_userLogLevelConsole);
	settings.setValue("UserLogLevelLogfile", m_userLogLevelLogfile);

	settings.setValue("MainWindowGeometry", geometry);
	settings.setValue("MainWindowState", state);

	for (QMap<PropertyType, QVariant>::const_iterator it = m_propertyMap.constBegin();
		 it != m_propertyMap.constEnd(); ++it)
	{
		settings.setValue(PROPERTY_KEYWORDS[it.key()], it.value());
	}
}


bool MSIMSettings::openFileInTextEditor(QWidget * parent, const QString & filepath) const {
	// check if editor has been set in preferences
	if (m_textEditorExecutable.isEmpty()) {
		QMessageBox::critical(parent, tr("Missing user preferences"), tr("Please open the preferences dialog and specify "
																	   "a text editor first!"));
		return false;
	}

	bool res = QProcess::startDetached( m_textEditorExecutable, QStringList() << filepath );
	if (!res) {
		QMessageBox::critical(parent, tr("Error starting external application"), tr("Text editor '%1' could not be started.")
							  .arg(m_textEditorExecutable));
	}
	return res;
}


void MSIMSettings::recursiveSearch(QDir baseDir, QStringList & files, const QStringList & extensions) {
	QStringList	fileList = baseDir.entryList(QStringList(), QDir::AllEntries | QDir::NoDotAndDotDot, QDir::Name);

	foreach (QString f, fileList) {
		QString fullPath = baseDir.absoluteFilePath(f);
		QFileInfo finfo(fullPath);
		if (finfo.isDir()) {
			recursiveSearch(QDir(fullPath), files, extensions);
		}
		else {
			bool found = false;
			foreach (QString ext, extensions) {
				if (finfo.suffix() == ext) {
					found = true;
					break;
				}
			}
			if (found)
				files.append(fullPath);
		}
	}
}
