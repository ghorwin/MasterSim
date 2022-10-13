#include "MSIMDirectories.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>

#include <IBK_configuration.h>
#include "MSIMUIConstants.h"

QString MSIMDirectories::resourcesRootDir() {
	QString installPath = qApp->applicationDirPath();

	// override install path (exe-file location) if REPOROOT path is given
#ifdef REPOROOT
	installPath = QString(REPOROOT) += "/bin/debug";
#endif

#if defined(IBK_DEPLOYMENT)
	// deployment mode

#if defined(Q_OS_WIN)
	// in Deployment mode, resources are below install directory
	return installPath + "/resources";
#elif defined(Q_OS_MAC)
	// in deployment mode, we have them in MasterSim.app/Contents/Resources
	// where install path is MasterSim.app/MacOS
	return installPath + "/../Resources";
#elif defined(Q_OS_UNIX)

#ifdef IBK_BUILDING_DEBIAN_PACKAGE

	// we install to /usr/bin/MasterSimulatorUI
	// and the package data is in
	//               /usr/share/mastersim
	return installPath + "/../share/mastersim";

#else // IBK_BUILDING_DEBIAN_PACKAGE

	return installPath + "/../resources";

#endif // IBK_BUILDING_DEBIAN_PACKAGE

#endif // defined(Q_OS_UNIX)


#else // IBK_DEPLOYMENT

	// development (IDE) mode

#if defined(Q_OS_WIN)
	// in development mode, we have the resources in the data directory
	// executables are build in bin/debug or bin/release
	return installPath + "/../../MasterSimulatorUI/resources";
#elif defined(Q_OS_MAC)
	// in development mode, we have the resources outside the bundle
	return installPath + "/../../../../MasterSimulatorUI/resources";
#elif defined(Q_OS_UNIX)
	return QFileInfo(installPath + "/../../MasterSimulatorUI/resources").absoluteFilePath();
#endif

#endif // IBK_DEPLOYMENT
}


QString MSIMDirectories::translationsFilePath(const QString & langID) {
#ifdef IBK_BUILDING_DEBIAN_PACKAGE
	QString installPath = qApp->applicationDirPath();
	return installPath + QString("/../share/locale/%1/LC_MESSAGES/MasterSimulatorUI.qm").arg(langID);
#else // IBK_BUILDING_DEBIAN_PACKAGE
	return QFileInfo(resourcesRootDir() + QString("/translations/MasterSimulatorUI_%1.qm").arg(langID)).absoluteFilePath();
#endif // IBK_BUILDING_DEBIAN_PACKAGE
}


QString MSIMDirectories::qtTranslationsFilePath(const QString & langID) {
#if defined(Q_OS_LINUX)
	return QString("/usr/share/qt5/translations/qt_%1.qm").arg(langID);
#else
	// in all other cases the qt_xx.qm files are located in the resources path
	return resourcesRootDir() + QString("/translations/qt_%1.qm").arg(langID);
#endif
}


QString MSIMDirectories::userDataDir() {
	// we have different user data directories, based on OS
#if defined(Q_OS_WIN)
	return QDir::toNativeSeparators(QDir::home().absolutePath() + "/AppData/Roaming/MasterSim");
#else
	// on Unix/Mac OS we store user data under home directory
	return QDir::toNativeSeparators(QDir::home().absolutePath() + "/.local/share/MasterSim");
#endif // Q_OS_WIN
}


QString MSIMDirectories::globalLogFile() {
	return userDataDir() + "/MasterSim.log";
}

