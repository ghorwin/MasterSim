#include "MSIMMainWindow.h"
#include <QApplication>
#include <QFont>
#include <QSplashScreen>
#include <QTimer>
#include <QLocale>

#include <IBK_Exception.h>
#include <IBK_messages.h>

#include <memory>
#include <iostream>

#include "MSIMSettings.h"
#include "MSIMUIConstants.h"
#include "MSIMDirectories.h"
#include "MSIMDirectories.h"
#include "MSIMLanguageHandler.h"
#include "MSIMMessageHandler.h"
#include "MSIMDebugApplication.h"
#include "MSIMConversion.h"

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

	// *** Font size adjustment ***
#if defined(Q_OS_MAC)
//	QFont f = qApp->font();
//	f.setPointSize(10);
//	qApp->setFont(f);
//	qApp->setDesktopSettingsAware(false);
#elif defined(Q_OS_UNIX)
	QFont f = qApp->font();
//	qDebug() << f;
	f.setPointSize(9);
	qApp->setFont(f);
	qApp->setDesktopSettingsAware(false);
	qApp->setWindowIcon(QIcon(":/gfx/MasterSimulator_48x48.png"));

#elif defined(Q_OS_WIN)
	QFont f = qApp->font();
//	qDebug() << f;
	f.setPointSize(8);
	qApp->setFont(f);
//	qApp->setDesktopSettingsAware(false);
#endif


	// *** Create and initialize setting object ***
	MSIMSettings settings(ORG_NAME, PROGRAM_NAME);
	settings.setDefaults();
	settings.read();


	// *** Initialize Command Line Argument Parser ***
	IBK::ArgParser argParser;
	settings.updateArgParser(argParser); // add MasterSim-specific UI argument options


	// *** Apply command line arguments ***
	argParser.parse(argc, argv);
	// handle default arguments (--help)
	if (argParser.flagEnabled("help")) {
		argParser.printHelp(std::cout);
		return EXIT_SUCCESS;
	}
	settings.applyCommandLineArgs(argParser);


	// *** Create log file directory and setup message handler ***
	QDir baseDir;
	baseDir.mkpath(MSIMDirectories::userDataDir());

	MSIMMessageHandler messageHandler;
	IBK::MessageHandlerRegistry::instance().setMessageHandler( &messageHandler );
	std::string errmsg;
	messageHandler.openLogFile(MSIMDirectories::globalLogFile().toUtf8().data(), false, errmsg);
	messageHandler.setConsoleVerbosityLevel( settings.m_userLogLevelConsole );
	messageHandler.setLogfileVerbosityLevel( settings.m_userLogLevelLogfile );


	// *** Install translator ***
	if (argParser.hasOption("lang")) {
		std::string dummy = argParser.option("lang");
		QString langid = utf82QString(dummy);
		if (langid != MSIMLanguageHandler::instance().langId()) {
			IBK::IBK_Message( IBK::FormatString("Installing translator for language: '%1'.\n")
								.arg(langid.toUtf8().data()),
								IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			MSIMLanguageHandler::instance().installTranslator(langid);
		}
	}
	else {
		MSIMLanguageHandler::instance().installTranslator(MSIMLanguageHandler::langId());
	}


	// *** Create and show splash-screen ***
	std::auto_ptr<QSplashScreen> splash;
	if (!settings.m_flags[MSIMSettings::NoSplashScreen]) {
		QPixmap pixmap;
		pixmap.load(":/images/splash_screen_de.png","PNG");
		splash.reset(new QSplashScreen(pixmap, Qt::WindowStaysOnTopHint | Qt::SplashScreen));
		splash->show();
		QTimer::singleShot(2000, splash.get(), SLOT(close()));
	}


	// *** Setup and show MainWindow and start event loop ***
	int res;
	try { // open scope to control lifetime of main window, ensure that main window instance dies before settings or project handler

		MSIMMainWindow w;

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
