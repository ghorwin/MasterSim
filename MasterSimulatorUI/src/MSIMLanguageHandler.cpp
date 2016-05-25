#include "MSIMLanguageHandler.h"
#include <QTranslator>
#include <QSettings>
#include <QCoreApplication>
#include <QLocale>
#include <QDebug>

#include <IBK_messages.h>
#include <IBK_FormatString.h>

#include "MSIMUIConstants.h"
#include "MSIMDirectories.h"

MSIMLanguageHandler & MSIMLanguageHandler::instance() {
	static MSIMLanguageHandler myHandler;
	return myHandler;
}

MSIMLanguageHandler::MSIMLanguageHandler() :
	applicationTranslator(NULL),
	systemTranslator(NULL)
{
}


MSIMLanguageHandler::~MSIMLanguageHandler() {
	// get rid of old translators
	// at this time, the application object doesn't live anylonger, so we
	// can savely destruct the translator objects
	delete applicationTranslator; applicationTranslator = NULL;
	delete systemTranslator; systemTranslator = NULL;
}


QString MSIMLanguageHandler::langId() {
	const char * const FUNC_ID = "[MSIMLanguageHandler::langId]";

	QSettings config(ORG_NAME, PROGRAM_NAME);
	QString langid = config.value("LangID", QString() ).toString();
	if (langid.isEmpty()) {
		// try to determine language id from OS
		QString localeName = QLocale::system().name();
		int pos = localeName.indexOf('_');
		if (pos != -1)
			localeName = localeName.left(pos);
		IBK::IBK_Message( IBK::FormatString("Translation required for locale: '%1'.\n").arg(localeName.toUtf8().data()),
			IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		langid = localeName;
	}
	return langid;
}


void MSIMLanguageHandler::setLangId(QString id) {
	QSettings config(ORG_NAME, PROGRAM_NAME);
	config.setValue("LangID", id );
}


void MSIMLanguageHandler::installTranslator(QString langId) {
	const char * const FUNC_ID = "[MSIMLanguageHandler::installTranslator]";

	// get rid of old translators
	if (applicationTranslator != NULL) {
		qApp->removeTranslator(applicationTranslator);
		delete applicationTranslator; applicationTranslator = NULL;
	}
	if (systemTranslator != NULL) {
		qApp->removeTranslator(systemTranslator);
		delete systemTranslator; systemTranslator = NULL;
	}


	// create new translators, unless we are using english
	if (langId == "en") {
		QSettings config(ORG_NAME, PROGRAM_NAME);
		config.setValue("LangID", langId);
		QLocale loc(QLocale::English);
		loc.setNumberOptions(QLocale::OmitGroupSeparator | QLocale::RejectGroupSeparator);
		QLocale::setDefault(loc);
		return;
	}


	QString translationPath = MSIMDirectories::translationsDir();
	IBK::IBK_Message( IBK::FormatString("Translation file path = '%1'.\n").arg(translationPath.toUtf8().data()),
		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

	systemTranslator = new QTranslator;

	// system translator first
	if (systemTranslator->load("qt_" + langId, translationPath)) {
		qApp->installTranslator(systemTranslator);
		IBK::IBK_Message( IBK::FormatString("Installing system translator using file: 'qt_%1'.\n").arg(langId.toUtf8().data()),
			IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	}
	else {
		IBK::IBK_Message( IBK::FormatString("Could not load system translator file: 'qt_%1'.\n").arg(langId.toUtf8().data()),
			IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		// no translator found, remove it again
		delete systemTranslator;
		systemTranslator = NULL;
	}


	applicationTranslator = new QTranslator;
	if (applicationTranslator->load("MasterSimulatorUI_" + langId, translationPath)) {
		IBK::IBK_Message( IBK::FormatString("Installing application translator using file: 'MasterSimulatorUI_%1'.\n").arg(langId.toUtf8().data()),
			IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		qApp->installTranslator(applicationTranslator);
		// remember translator in settings
		QSettings config(ORG_NAME, PROGRAM_NAME);
		config.setValue("LangID", langId);
	}
	else {
		IBK::IBK_Message( IBK::FormatString("Could not load application translator file: 'MasterSimulatorUI_%1'.\n").arg(langId.toUtf8().data()),
			IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		delete applicationTranslator;
		applicationTranslator = NULL;
	}

	// now also set the corresponding locale settings (for number display etc.)
	if (langId == "de") {
		QLocale loc(QLocale::German);
		loc.setNumberOptions(QLocale::OmitGroupSeparator | QLocale::RejectGroupSeparator);
		QLocale::setDefault(loc);
	}
}
