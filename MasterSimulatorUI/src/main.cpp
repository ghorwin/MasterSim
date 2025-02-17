#include <QApplication>
#include <QFont>
#include <QSplashScreen>
#include <QTimer>
#include <QLocale>
#include <QPalette>
#include <QStandardPaths>
#include <QTextStream>

#include <IBK_Exception.h>
#include <IBK_messages.h>
#include <IBK_configuration.h>

#include <memory>
#include <iostream>

#include "MSIMMainWindow.h"
#include "MSIMSettings.h"
#include "MSIMUIConstants.h"
#include "MSIMDirectories.h"
#include "MSIMDirectories.h"
#include "MSIMLanguageHandler.h"
#include "MSIMMessageHandler.h"
#include "MSIMDebugApplication.h"
#include "MSIMConversion.h"

#include "MSIM_Constants.h"

#if QT_VERSION >= 0x050000
void qDebugMsgHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
	(void) type;
	(void) context;
	std::cout << msg.toStdString() << std::endl;
}
#else
void qDebugMsgHandler(QtMsgType type, const char *msg) {
	(void) type;
	std::cout << msg << std::endl;
}
#endif


int main(int argc, char *argv[]) {
	const char * const FUNC_ID = "[main]";

#if QT_VERSION >= 0x050E00
	QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

	MSIMDebugApplication a(argc, argv);

#if QT_VERSION >= 0x050000
	qInstallMessageHandler(qDebugMsgHandler);
#else
	qInstallMsgHandler(qDebugMsgHandler);
#endif


	// *** Locale setup for Unix/Linux ***
#if defined(Q_OS_UNIX)
	setlocale(LC_NUMERIC,"C");
#endif

	const QString ProgramVersionName = QString("MasterSim %1").arg(MASTER_SIM::VERSION);

	// *** Font size adjustment ***
#if defined(Q_OS_MAC)
//	QFont f = qApp->font();
//	f.setPointSize(10);
//	qApp->setFont(f);
//	qApp->setDesktopSettingsAware(false);
#elif defined(Q_OS_UNIX)
	QFont f = qApp->font();
	f.setPointSize(9);
	qApp->setFont(f);
#elif defined(Q_OS_WIN)
	QFont f = qApp->font();
	f.setPointSize(8);
	qApp->setFont(f);
#endif

	qApp->setApplicationName(ProgramVersionName);
	qApp->setWindowIcon(QIcon(":/gfx/logo/Icon_64.png"));


	// *** Create and initialize setting object ***
	MSIMSettings settings(ORG_NAME, ProgramVersionName);
	settings.setDefaults();
	settings.read();
	// if we have just upgraded to a new version, try to import settings from the last minor version
	if (settings.m_versionIdentifier.isEmpty() && settings.m_lastProjectFile.isEmpty()) {
		unsigned int major, minor, patch;
		IBK::decode_version_number(MASTER_SIM::VERSION, major, minor, patch);
		for (int i=(int)minor-1; i>0; --i) {
			QString VersionName = QString("MasterSim %1.%2").arg(major).arg(i);
			settings.m_appName = VersionName;
			settings.read();
			if (!settings.m_versionIdentifier.isEmpty() || !settings.m_lastProjectFile.isEmpty() ||
					settings.m_propertyMap[MSIMSettings::PT_LastFileOpenDirectory].isValid())
				break;
		}
		settings.m_appName = ProgramVersionName;
	}
	settings.m_versionIdentifier = MASTER_SIM::VERSION;


	// *** Initialize Command Line Argument Parser ***
	IBK::ArgParser parser;
	settings.updateArgParser(parser); // add MasterSim-specific UI argument options


	// *** Apply command line arguments ***
	parser.parse(argc, argv);

	// configure man page output
	parser.m_appname = "mastersim-gui";
	parser.m_syntaxArguments = "[flags] [options] [project file]";
	parser.m_manManualName = "MasterSim Manual";
	parser.m_manReleaseDate = MASTER_SIM::RELEASE_DATE;
	parser.m_manVersionString = MASTER_SIM::LONG_VERSION;
	parser.m_manShortDescription = "FMI Co-Simulation Graphical Configuration Tool";

	// Note: mind the line breaks that end format commands!
	parser.m_manLongDescription = ".B mastersim-gui\n"
			"is a graphical user interface to configure/define FMI co-simulation "
			"scenarios and generate MasterSim project files. These project files "
			"are used by the commandline co-simulator \n.BR mastersim\\fR.";

	// handle default arguments (--help and --man-page)
	if (parser.handleDefaultFlags(std::cout))
		return EXIT_SUCCESS;
	settings.applyCommandLineArgs(parser);


	// *** Create log file directory and setup message handler ***
	QDir baseDir;
	baseDir.mkpath(MSIMDirectories::userDataDir());

	MSIMMessageHandler messageHandler;
	IBK::MessageHandlerRegistry::instance().setMessageHandler( &messageHandler );
	std::string errmsg;
	messageHandler.openLogFile(MSIMDirectories::globalLogFile().toStdString(), false, errmsg);
	messageHandler.setConsoleVerbosityLevel( settings.m_userLogLevelConsole );
	messageHandler.setLogfileVerbosityLevel( settings.m_userLogLevelLogfile );

	// *** Install translator ***
	if (parser.hasOption("lang")) {
		std::string dummy = parser.option("lang");
		QString langid = utf82QString(dummy);
		if (langid != MSIMLanguageHandler::instance().langId()) {
			IBK::IBK_Message( IBK::FormatString("Installing translator for language: '%1'.\n")
								.arg(langid.toStdString()),
								IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			MSIMLanguageHandler::instance().installTranslator(langid);
		}
	}
	else {
		MSIMLanguageHandler::instance().installTranslator(MSIMLanguageHandler::langId());
	}


	// *** Create and show splash-screen ***
#if QT_VERSION >= 0x050000
	std::unique_ptr<QSplashScreen> splash;
#else
	std::auto_ptr<QSplashScreen> splash;
#endif
	if (!settings.m_flags[MSIMSettings::NoSplashScreen]) {
		QPixmap pixmap;
		pixmap.load(":/gfx/SplashScreen_MasterSim.png","PNG");
		splash.reset(new QSplashScreen(pixmap, Qt::WindowStaysOnTopHint | Qt::SplashScreen));
		splash->show();
		qApp->processEvents();
		QTimer::singleShot(2000, splash.get(), SLOT(close()));
	}


	// *** Setup and show MainWindow and start event loop ***
	int res;
	try { // open scope to control lifetime of main window, ensure that main window instance dies before settings or project handler

		MSIMMainWindow w;
		qApp->processEvents();

		// add user settings related window resize at program start
#if defined(Q_OS_WIN)
		w.showMaximized();
#elif defined(Q_OS_LINUX)
		w.show();
#else
		w.show();
#endif

		// start event loop
		res = a.exec();
	} // here our mainwindow dies, main window goes out of scope and UI goes down -> destructor does ui and thread cleanup
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		return EXIT_FAILURE;
	}

	// return exit code to environment
	return res;
}
