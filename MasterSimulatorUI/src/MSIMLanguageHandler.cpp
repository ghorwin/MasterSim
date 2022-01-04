#include "MSIMLanguageHandler.h"
#include <QTranslator>
#include <QSettings>
#include <QCoreApplication>
#include <QLocale>
#include <QDebug>

#include <IBK_messages.h>
#include <IBK_FormatString.h>
#include <IBK_configuration.h>

#include <MSIM_Constants.h>

#include "MSIMUIConstants.h"
#include "MSIMDirectories.h"
#include "MSIMSettings.h"

MSIMLanguageHandler & MSIMLanguageHandler::instance() {
	static MSIMLanguageHandler myHandler;
	return myHandler;
}

MSIMLanguageHandler::MSIMLanguageHandler() :
	applicationTranslator(nullptr),
	systemTranslator(nullptr)
{
}


MSIMLanguageHandler::~MSIMLanguageHandler() {
	// get rid of old translators
	// at this time, the application object doesn't live anylonger, so we
	// can savely destruct the translator objects
	delete applicationTranslator; applicationTranslator = nullptr;
	delete systemTranslator; systemTranslator = nullptr;
}


QString MSIMLanguageHandler::langId() {
	const char * const FUNC_ID = "[MSIMLanguageHandler::langId]";

	QSettings config(MSIMSettings::instance().m_organization, MSIMSettings::instance().m_appName);
	QString langid = config.value("LangID", QString() ).toString();
	if (langid.isEmpty()) {
		// try to determine language id from OS
		QString localeName = QLocale::system().name();
		int pos = localeName.indexOf('_');
		if (pos != -1)
			localeName = localeName.left(pos);
		IBK::IBK_Message( IBK::FormatString("Translation required for locale: '%1'.\n").arg(localeName.toStdString()),
			IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		langid = localeName;
	}
	return langid;
}


void MSIMLanguageHandler::setLangId(QString id) {
	QSettings config(MSIMSettings::instance().m_organization, MSIMSettings::instance().m_appName);
	config.setValue("LangID", id );
}


void MSIMLanguageHandler::installTranslator(QString langId) {
	const char * const FUNC_ID = "[MSIMLanguageHandler::installTranslator]";

	// get rid of old translators
	if (applicationTranslator != nullptr) {
		qApp->removeTranslator(applicationTranslator);
		delete applicationTranslator; applicationTranslator = nullptr;
	}
	if (systemTranslator != nullptr) {
		qApp->removeTranslator(systemTranslator);
		delete systemTranslator; systemTranslator = nullptr;
	}


	// create new translators, unless we are using english
	if (langId == "en") {
		QSettings config(MSIMSettings::instance().m_organization, MSIMSettings::instance().m_appName);
		config.setValue("LangID", langId);
		QLocale loc(QLocale::English);
		loc.setNumberOptions(QLocale::OmitGroupSeparator | QLocale::RejectGroupSeparator);
		QLocale::setDefault(loc);
		return;
	}


	// We distinguish between regular install, i.e. with deployed Qt libs on Windows and Darwin
	// and with pre-installed qt on Linux/Unix plattforms
	//
	// In case of Windows/MacOS the qt-translation files are shipped together with the application,
	// yet in case of Linux the qt translation files are always in /usr/share/qt5/translations
	//
	// The application translation files are stored in the MSIMDirectories::translationsDir(), which returns
	// the appropriate path based on whether the define IBK_BUILDING_DEBIAN_PACKAGE is set or not.
	// If this define is set, we expect a directory structure like
	//   <prefix>/bin/MasterSimulatorUI
	//   <prefix>/share/mastersim/translations

	QString translationFilePath = MSIMDirectories::translationsFilePath(langId);
	QString qtTranslationFilePath = MSIMDirectories::qtTranslationsFilePath(langId);

	IBK::IBK_Message( IBK::FormatString("App translation file path = '%1'.\n").arg(translationFilePath.toStdString()),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message( IBK::FormatString("Qt translation file path  = '%1'.\n").arg(qtTranslationFilePath.toStdString()),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

	// system translator first, filename is for example "qt_de"
	systemTranslator = new QTranslator;
	QFileInfo finfoQt(qtTranslationFilePath);
	if (finfoQt.exists() && systemTranslator->load(finfoQt.fileName(), finfoQt.dir().absolutePath())) {
		qApp->installTranslator(systemTranslator);
		IBK::IBK_Message( IBK::FormatString("Qt translation file loaded successfully\n"),
			IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	}
	else {
		IBK::IBK_Message( IBK::FormatString("Could not load system translator file: 'qt_%1'.\n").arg(langId.toStdString()),
			IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		// no translator found, remove it again
		delete systemTranslator;
		systemTranslator = nullptr;
	}

	applicationTranslator = new QTranslator;
	QFileInfo finfo(translationFilePath);
	if (finfo.exists() && applicationTranslator->load(finfo.fileName(), finfo.dir().absolutePath())) {
		IBK::IBK_Message( IBK::FormatString("Application translator loaded successfully\n"),
			IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		qApp->installTranslator(applicationTranslator);
		// remember translator in settings
		QSettings config(MSIMSettings::instance().m_organization, MSIMSettings::instance().m_appName);
		config.setValue("LangID", langId);
	}
	else {
		IBK::IBK_Message( IBK::FormatString("Could not load application translator file: 'MasterSimulatorUI_%1'.\n").arg(langId.toStdString()),
			IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		delete applicationTranslator;
		applicationTranslator = nullptr;
	}

	// now also set the corresponding locale settings (for number display etc.)
	if (langId == "de") {
		QLocale loc(QLocale::German);
		loc.setNumberOptions(QLocale::OmitGroupSeparator | QLocale::RejectGroupSeparator);
		QLocale::setDefault(loc);
	}
}
