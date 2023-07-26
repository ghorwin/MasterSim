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

#ifdef Q_OS_WIN
#undef UNICODE
#include <Windows.h>
#endif

MSIMSettings * MSIMSettings::m_self = nullptr;

const char * const					MSIMSettings::PROPERTY_KEYWORDS[MSIMSettings::NUM_PT] = {
	"LastImportDirectory",
	"LastFileOpenDirectory",
	"LastFMUImportDirectory",
	"LastExampleSaveDirectory"
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
	argParser.addOption(0, "lang", "Specify the program language using a 2 character language ID.", "en;de;it;...", "en");
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
	(void)terminalEmulator; // to get rid of compiler warning - only used for Linux
	// spawn process
#ifdef Q_OS_WIN

	// Use WinAPI to create a solver process
	// our executable path and the project file may be utf8 encoded, which Windows cannot handle natively.
	// As we want to support both VC and MinGW, we rely on Ansi-conversion and convert all strings to Ansi OEM page encoding first.

	std::string projectFileUtf8 = projectFile.toStdString().data();
	std::string projectFileLatin1 = IBK::UTF8ToANSIString(projectFileUtf8); // Mind: buffer must exist until CreateProcess call is through
	QString cmdLine = QString("\"%1\" %2 \"%3\"")
		.arg(executable,
			 commandLineArgs.join(" "),
			 projectFile);

	std::string cmdLatin1 = IBK::UTF8ToANSIString(cmdLine.toStdString());

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	si.lpTitle = (LPSTR)projectFileLatin1.data();
//	si.dwFlags = STARTF_USESHOWWINDOW;
//	si.wShowWindow = SW_SHOW;
	ZeroMemory( &pi, sizeof(pi) );
	const unsigned int lower_priority = 0x00004000;
	// Start the child process.
	if( !CreateProcess( nullptr,	// No module name (use command line).
		&cmdLatin1[0],				// Command line.
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

	QString bashCmdLine = (executable + " " + commandLineArgs.join(" ") + " \"" + projectFile + '"');

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
	return QProcess::execute("open", allCmdLine) == 0;

#else // for all other platforms LINUX is expected

	bool success;
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


#include <QFileInfo>
#include <QMessageBox>
#include <QDir>
#include <QDebug>
#include <QProcess>

void MSIMSettings::linuxDesktopIntegration(QWidget * parent,
							 const QString & iconLocation,
							 const QString & appname,               // MasterSim
							 const QString & appIDname,             // mastersim
							 const QString & desktopAppComment,     // FMI Co-Sim Master
							 const QString & desktopAppExec,        // /path/to/bin/MasterSimulatorUI
							 const QString & fileExtension          // msim   (for *.msim)
							 )
{
	// compose path to desktop-file, if existing, prompt user to "update" system integration, otherwise prompt to
	// "setup" integration

	// mimetype:     application/mastersim
	// icon-name:    mastersim         (prefix must match mimetype)
	// desktop-file: mastersim.desktop

	QString desktopFile = QDir::home().absoluteFilePath(QString(".local/share/applications/%1.desktop").arg(appIDname));
	if (QFileInfo::exists(desktopFile)) {
		int res = QMessageBox::question(parent, tr("Update Desktop Integration"), tr("Should the existing desktop integration and %2-file type association be updated?").arg(fileExtension),
										QMessageBox::Yes | QMessageBox::No);
		if (res == QMessageBox::No)
			return;
	}
	else {
		int res = QMessageBox::question(parent, tr("Update Desktop Integration"), tr("Should %1 set up the desktop integration and associate %2-file types with %1?").arg(appname).arg(fileExtension),
										QMessageBox::Yes | QMessageBox::No);
		if (res == QMessageBox::No)
			return;
	}


	// copy icon files, unless existing already
	QStringList iconSizes;
	iconSizes << "16" << "32" << "48" << "64" << "128" << "256" << "512";
	QString iconRootDir = QDir::home().absoluteFilePath(".local/share/icons/hicolor");
	foreach (QString s, iconSizes) {
		// path to source icon file
		QString iconFile = iconLocation + "/Icon_" + s + ".png";
		if (!QFile::exists(iconFile)) {
			qDebug() << QString("Missing icon file '%1'").arg(iconFile);
			continue;
		}

		// compose resolution-specific target path
		QString targetPath = QDir::home().absoluteFilePath(iconRootDir + "/%1x%1").arg(s);

		// copy desktop icon file, for example, 48x48 icon goes to 'hicolor/48x48/apps'
		QDir::home().mkpath( targetPath + "/apps"); // create path if not yet existing
		QString desktopIconTargetFile = targetPath + QString("/apps/%1.png").arg(appIDname);
		QFile::copy(iconFile, desktopIconTargetFile);

		// copy mimetype icon file, for example, 48x48 icon goes to 'hicolor/48x48/mimetypes' and is named 'application-<appIDname>'
		QDir::home().mkpath( targetPath + "/mimetypes"); // create path if not yet existing
		QString mimetypeIconTargetPath = targetPath + QString("/mimetypes/application-%1.png").arg(appIDname);
		QFile::copy(iconFile, mimetypeIconTargetPath);
	}

	// generate .desktop file, if it does not exist yet
	QString desktopFileContents =
			"[Desktop Entry]\n"
			"Name=%1\n"
			"Comment=%2\n"
			"Exec=%3\n"
			"Icon=%4\n"
			"Terminal=false\n"
			"Type=Application\n"
			"Categories=Science\n"
			"StartupNotify=true\n"
			"MimeType=application/%4\n";
	desktopFileContents = desktopFileContents.arg(appname, desktopAppComment, desktopAppExec, appIDname);
	QFile deskFile(desktopFile);
	deskFile.open(QFile::WriteOnly);
	QTextStream strm(&deskFile);
	strm << desktopFileContents;
	deskFile.setPermissions((QFile::Permission)0x7755);
	deskFile.close();

	// also create Mime file for file type associations for 'application/appIDname'
	QString mimeFileContents =
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
			"<mime-info xmlns='http://www.freedesktop.org/standards/shared-mime-info'>\n"
			"	<mime-type type=\"application/%1\">\n"
			"		<comment>%2 project file</comment>\n"
			"		<glob pattern=\"*.%3\"/>\n"
			"	</mime-type>\n"
			"</mime-info>\n";
	mimeFileContents = mimeFileContents.arg(appIDname, appname, fileExtension);
	QString mimeDir = QDir::home().absoluteFilePath(".local/share/mime");
	QString mimeFile = mimeDir + QString("/packages/%1.xml").arg(appIDname);
	QFile mimeF(mimeFile);
	mimeF.open(QFile::WriteOnly);
	QTextStream strm2(&mimeF);
	strm2 << mimeFileContents;
	mimeF.close();

	// mime-type database update is still needed; if that doesn't work, we can't help it
	QProcess::execute("update-mime-database", QStringList() << mimeDir);

	QProcess::execute("update-icon-caches", QStringList() << iconRootDir);

	// Note: one still needs to logout/logon to make the icon association effective

	QMessageBox::information(parent, tr("Update Desktop Integration"), tr("Created application shortcut and registered file association. Changes will take effect after next login."));
}
