#include "MSIMMainWindow.h"
#include <QApplication>
#include <QFont>
#include <QSplashScreen>
#include <QTimer>

#include <IBK_MessageHandlerRegistry.h>

#include <memory>
#include <iostream>

#include "MSIMSettings.h"
#include "MSIMUIConstants.h"
#include "MSIMDirectories.h"

int main(int argc, char *argv[]) {
	QApplication a(argc, argv);

	// *** Locale setup for Unix/Linux ***
#if defined(Q_OS_LINUX)
	setlocale(LC_NUMERIC,"C");
#endif


	// *** Font size adjustment ***
#if defined(Q_OS_MAC)
	QFont f = qApp->font();
	f.setPointSize(10);
	qApp->setFont(f);
	qApp->setDesktopSettingsAware(false);
#elif defined(Q_OS_UNIX)
/*
	QFont f = qApp->font();
	f.setPointSize(9);
	qApp->setFont(f);
	qApp->setDesktopSettingsAware(false);
*/
#elif defined(Q_OS_WIN)
	QFont f = qApp->font();
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

	IBK::MessageHandler * messageHandler = IBK::MessageHandlerRegistry::instance().messageHandler();
	std::string errmsg;
	messageHandler->openLogFile(MSIMDirectories::globalLogFile().toUtf8().data(), false, errmsg);
	messageHandler->setConsoleVerbosityLevel( settings.m_userLogLevelConsole );
	messageHandler->setLogfileVerbosityLevel( settings.m_userLogLevelLogfile );


	// *** Install translator ***

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
	{ // open scope to control lifetime of main window, ensure that main window instance dies before settings or project handler

		MSIMMainWindow w;

#if defined(Q_OS_WIN)
		// add user settings related window resize at program start
		w.showMaximized();
		//w.resize( settings.m_xSizeAtProgrammClose, settings.m_ySizeAtProgrammClose);
		//w.show();
#elif defined(Q_OS_LINUX)
		w.show();
#else
		w.show();
#endif

		// start event loop
		res = a.exec();
	} // here our mainwindow dies, main window goes out of scope and UI goes down -> destructor does ui and thread cleanup

	// return exit code to environment
	return res;
}
