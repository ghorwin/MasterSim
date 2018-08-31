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

	MSIMDebugApplication a(argc, argv);

//	QPalette p;
//	p.setBrush(QPalette::Window, QColor(44,44,44));
//	a.setPalette(p);

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
	f.setPointSize(9);
	qApp->setFont(f);
	qApp->setWindowIcon(QIcon(":/gfx/MasterSimulator_48x48.png"));
#elif defined(Q_OS_WIN)
	QFont f = qApp->font();
	f.setPointSize(8);
	qApp->setFont(f);
#endif


	// *** Create and initialize setting object ***
	MSIMSettings settings(ORG_NAME, MASTER_SIM::PROGRAM_NAME);
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

#if defined(Q_OS_UNIX)

	// copy icon files, unless existing already
//	QString iconFile = QDir::home().absoluteFilePath(".local/share/icons/hicolor/32x32/apps/mastersim.png");
#ifdef IBK_DEPLOYMENT
	QString iconLocation = MSIMDirectories::resourcesRootDir();
#else
	QString iconLocation = MSIMDirectories::resourcesRootDir() + "/gfx/logo";
#endif
	QStringList iconSizes;
	iconSizes << "16" << "32" << "48" << "64" << "128" << "256" << "512";
	QString targetPath = QDir::home().absoluteFilePath(".icons/hicolor/%1x%1/apps/mastersim.png");
//	QString targetPath = QDir::home().absoluteFilePath(".local/share/icons/hicolor/%1x%1/apps/mastersim.png");
	foreach (QString s, iconSizes) {
		QString iconFile = iconLocation + "/icon_" + s + ".png";
		QDir::home().mkpath( QString(".icons/hicolor/%1x%1/apps").arg(s));
		QString targetFile = targetPath.arg(s);
		if (!QFile(targetFile).exists())
			QFile::copy(iconFile, targetFile);
	}

	// generate .desktop file, if it does not exist yet
	QString desktopFileContents =
			"[Desktop Entry]\n"
			"Name=MasterSim %1\n"
			"Comment=FMI Co-Simulations Master\n"
			"Exec=%2/MasterSimulatorUI\n"
			"Icon=mastersim\n"
			"Terminal=false\n"
			"Type=Application\n"
			"Categories=Science;\n"
			"StartupNotify=true\n";
	desktopFileContents = desktopFileContents.arg(MASTER_SIM::LONG_VERSION).arg(settings.m_installDir);
	QStringList dirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
	if (!dirs.empty()) {
		QString desktopFile = dirs[0] + "/mastersimulatorui.desktop";
		if (!QFile(desktopFile).exists()) {
			QFile deskFile(desktopFile);
			deskFile.open(QFile::WriteOnly);
			QTextStream strm(&deskFile);
			strm << desktopFileContents;
			deskFile.setPermissions((QFile::Permission)0x755);
			deskFile.close();
		}
	}
#endif

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
