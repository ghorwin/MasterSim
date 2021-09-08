#include "MSIMSettings.h"

#include <IBK_SolverArgsParser.h>
#include <IBK_MessageHandler.h>
#include <IBK_messages.h>

#include <QLocale>
#include <QDesktopServices>
#include <QSettings>
#include <QProcess>
#include <QMessageBox>
#include <QTextStream>

#include "MSIMConversion.h"
#include <MSIM_Constants.h>

MSIMSettings * MSIMSettings::m_self = nullptr;

const char * const					MSIMSettings::PROPERTY_KEYWORDS[MSIMSettings::NUM_PT] = {
	"LastImportDirectory",
	"LastFileOpenDirectory"
};


MSIMSettings & MSIMSettings::instance() {
	Q_ASSERT_X(m_self != nullptr, "[MSIMSettings::instance]", "You must create an instance of "
		"MSIMSettings before accessing MSIMSettings::instance()!");
	return *m_self;
}


MSIMSettings::MSIMSettings(const QString & organization, const QString & appName) :
	m_organization(organization),
	m_appName(appName)
{
	// singleton check
	Q_ASSERT_X(m_self == nullptr, "[MSIMSettings::MSIMSettings]", "You must not create multiple instances of "
		"classes that derive from MSIMSettings!");
	m_self = this;
}


MSIMSettings::~MSIMSettings() {
	m_self = nullptr;
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

	m_thumbNailSize = 300; // for now we do not have thumbnails, yet

	// determine text executable
	m_textEditorExecutable.clear();

	// default to XTerm
	m_terminalEmulator = TE_XTerm;

	/// \todo Improve default text editor detection
#ifdef Q_OS_UNIX
	m_textEditorExecutable = "gedit";
#elif defined(Q_OS_WIN)
	m_textEditorExecutable = "C:\\Program Files\\Notepad++\\notepad++.exe";
	if (!QFileInfo(m_textEditorExecutable).exists())
		m_textEditorExecutable = "C:\\Program Files (x86)\\Notepad++\\notepad++.exe";
#else
	// OS x editor?
#endif

	m_flags[NoSplashScreen] = false;
	m_flags[FullScreen] = false;

#if defined(Q_OS_WIN)
	// auto-detect postproc 2 in install directory
	QString postProc2FilePath;
	QString postProc2FilePathMask = "Program Files\\IBK\\PostProc 2.%1\\PostProcApp.exe";
	// search postproc (first 20 versions)
	for (int i=0; i<20; ++i) {
		QString postProc2FilePath2 = postProc2FilePathMask.arg(i);
		if (QFile(postProc2FilePath2).exists()) {
			postProc2FilePath = postProc2FilePath2;
			break;
		}
	}
#elif defined (Q_OS_MAC)
	QString postProc2FilePath = "/Applications/PostProcApp.app/Contents/MacOS/PostProcApp";
#else
	QString postProc2FilePath = "PostProcApp";
#endif
	if (QFile(postProc2FilePath).exists())
		m_postProcExecutable = postProc2FilePath;

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
		std::string str = argParser.args()[1];
#ifdef Q_OS_WIN
		// On Windows, use codepage encoding instead of UTF8
		m_initialProjectFile = QString::fromLatin1(str.c_str() );
#else
		m_initialProjectFile = QString::fromUtf8( str.c_str() );
		// remove "file://" prefix
		if (m_initialProjectFile.indexOf("file://") == 0)
			m_initialProjectFile = m_initialProjectFile.mid(7);
#endif
	}
}


void MSIMSettings::read(QString regName) {
//	const char * const FUNC_ID = "[MSIMSettings::read]";

	if (regName.isEmpty())
		regName = m_appName;

	// if no settings exist for current version, we attempt to read from a previous version,
	// where we construct the name from format "name <major>.<minor>"

	QSettings settings( m_organization, regName );
	if (!settings.contains("LangID")) {
		// attempt reading with reduced version number
		QStringList tokens = regName.split(".");
		if (tokens.size() == 2) {
			bool success;
			int minor = tokens[1].toInt(&success);
			if (success && minor > 2) {
				--minor;
				regName = tokens[0] + "." + QString::number(minor);
				read(regName);
				return;
			}
			// fall through and attempt reset defaults if keys cannot be read
		}
	}
	m_lastProjectFile = settings.value("LastProjectFile", QString()).toString();
	m_recentProjects = settings.value("RecentProjects", QStringList()).toStringList();

	m_maxRecentProjects = settings.value("MaxRecentProjects", m_maxRecentProjects).toUInt();
	m_maxNumUNDOSteps = settings.value("MaxNumUndoSteps", m_maxNumUNDOSteps).toUInt();

	m_versionIdentifier = settings.value("VersionIdentifier", QString()).toString(); // on first call, this version number is empty

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

	m_postProcExecutable = settings.value("PostProcExecutable", QString()).toString();
	m_terminalEmulator = (TerminalEmulators)settings.value("TerminalEmulator", TE_XTerm).toInt();

}


void MSIMSettings::readMainWindowSettings(QByteArray &geometry, QByteArray &state) {
	QSettings settings( m_organization, m_appName );
	geometry = settings.value("MainWindowGeometry", QByteArray()).toByteArray();
	state = settings.value("MainWindowState", QByteArray()).toByteArray();
}


void MSIMSettings::write(QByteArray geometry, QByteArray state) {

	QSettings settings( m_organization, m_appName );
	settings.setValue("VersionIdentifier", m_versionIdentifier);
	settings.setValue("LastProjectFile", m_lastProjectFile);
	settings.setValue("RecentProjects", m_recentProjects);

	settings.setValue("MaxRecentProjects", m_maxRecentProjects );
	settings.setValue("UndoSize",m_maxNumUNDOSteps);

	settings.setValue("TextEditorExecutable", m_textEditorExecutable );
	settings.setValue("LangID", m_langId );

	settings.setValue("LastVersionNumber", MASTER_SIM::LONG_VERSION);

	settings.setValue("UserLogLevelConsole", m_userLogLevelConsole);
	settings.setValue("UserLogLevelLogfile", m_userLogLevelLogfile);

	settings.setValue("MainWindowGeometry", geometry);
	settings.setValue("MainWindowState", state);

	for (QMap<PropertyType, QVariant>::const_iterator it = m_propertyMap.constBegin();
		 it != m_propertyMap.constEnd(); ++it)
	{
		settings.setValue(PROPERTY_KEYWORDS[it.key()], it.value());
	}

	settings.setValue("PostProcExecutable", m_postProcExecutable );
	settings.setValue("TerminalEmulator", m_terminalEmulator);
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


bool MSIMSettings::startProcess(const QString & executable,
									QStringList commandLineArgs,
									const QString & projectFile,
									TerminalEmulators terminalEmulator)
{
	bool success;
	// spawn process
#ifdef Q_OS_WIN

	/// \todo use wide-string version of API and/or encapsulate spawn process into a function

	// Use WinAPI to create a solver process
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	std::string utf8String = projectFile.toStdString().data();
	si.lpTitle = (LPSTR)utf8String.c_str();
//	si.dwFlags = STARTF_USESHOWWINDOW;
//	si.wShowWindow = SW_SHOW;
	ZeroMemory( &pi, sizeof(pi) );
	const unsigned int lower_priority = 0x00004000;
	QString cmdLine = QString("\"%1\" %2 \"%3\"")
		.arg(executable)
		.arg(commandLineArgs.join(" "))
		.arg(projectFile);

	std::string cmd = cmdLine.toLatin1().data();
	// Start the child process.
	if( !CreateProcess( nullptr,	// No module name (use command line).
		&cmd[0],					// Command line.
		nullptr,					// Process handle not inheritable.
		nullptr,					// Thread handle not inheritable.
		FALSE,						// Set handle inheritance to FALSE.
		lower_priority,				// Create with priority lower then normal.
		nullptr,					// Use parent's environment block.
		nullptr,					// Use parent's starting directory.
		&si,						// Pointer to STARTUPINFO structure.
		&pi )						// Pointer to PROCESS_INFORMATION structure.
	)
	{
		return false;
	}
	return true;

#elif defined(Q_OS_MAC)

	QString bashCmdLine = (executable + " " + commandLineArgs.join(" "));

	// on Mac, create a bash script with the command line as content
	QString projectPath = QFileInfo(projectFile).dir().absolutePath();
	QString tmpPath = projectPath + "/" + QFileInfo(projectFile).baseName() + ".sh";
	QFile bashFile(tmpPath);
	bashFile.open(QFile::WriteOnly);
	QTextStream strm(&bashFile);
	strm << "#!/bin/bash\n";
	// only for debugging we need to add the library fall back path
#ifdef IBK_DEBUG
	strm << QString("export DYLD_FALLBACK_LIBRARY_PATH=%1:%2\n")
			.arg(MSIMSettings::instance().m_installDir + "/../../../../../externals/lib_x64")
			.arg(MSIMSettings::instance().m_installDir + "/../../../../../lib_x64");
#endif
	strm << bashCmdLine + '\n';

	// finally set executable permissions
	bashFile.setPermissions(QFile::ReadUser | QFile::WriteUser | QFile::ExeUser | QFile::ExeGroup | QFile::ExeOther );
	bashFile.close();

	QStringList allCmdLine{ "-a" , "Terminal.app" , tmpPath };
	QProcess::execute("open", allCmdLine);

#else // for all other platforms LINUX is expected

	qint64 pid;
	switch (terminalEmulator) {
		case TE_XTerm : {
			commandLineArgs = QStringList() << "-hold"
											<< "-fa" << "'Monospace'"
											<< "-fs" << "9"
											<< "-geometry" << "120x40" << "-e" << executable << commandLineArgs
											<< projectFile; // append project file to arguments, no quotes needed, since Qt takes care of that
			QString terminalProgram = "xterm";
			success = QProcess::startDetached(terminalProgram, commandLineArgs, QString(), &pid);
		} break;

		case TE_GnomeTerminal : {
			// create a bash script in a temporary location with the command line as content
			QString projectPath = QFileInfo(projectFile).dir().absolutePath();
			QString tmpPath = projectPath + "/" + QFileInfo(projectFile).baseName() + ".sh";
			QFile bashFile(tmpPath);
			bashFile.open(QFile::WriteOnly);
			QTextStream strm(&bashFile);
			strm << "#!/bin/bash\n\n";
			strm << executable << " " << commandLineArgs.join(" ") << " \"" << projectFile << "\"" << "\n"; // mind the quotes around the project file here!
			// add a line to halt script execution once done
			strm << "exec bash\n";
			// finally set executable permissions
			bashFile.setPermissions(QFile::ReadUser | QFile::WriteUser | QFile::ExeUser | QFile::ExeGroup | QFile::ExeOther );
			bashFile.close();

			// Command line: "gnome-terminal -- /path/to/project/run/script.sh"
			commandLineArgs = QStringList() << "--tab"  << "--" << tmpPath;
			QString terminalProgram = "gnome-terminal";
			success = QProcess::startDetached(terminalProgram, commandLineArgs, QString(), &pid);
		} break;

		default:
			commandLineArgs << projectFile; // append project file to arguments, no quotes needed, since Qt takes care of that
			success = QProcess::startDetached(executable, commandLineArgs, QString(), &pid);
	}


	return success;

#endif // Q_OS_WIN
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
