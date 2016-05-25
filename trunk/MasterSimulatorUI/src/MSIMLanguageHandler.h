#ifndef MSIMLanguageHandlerH
#define MSIMLanguageHandlerH

#include <QString>
#include <QCoreApplication>

class QTranslator;

/*! Central class that handles language switching. */
class MSIMLanguageHandler {
	Q_DISABLE_COPY(MSIMLanguageHandler)
public:
	/*! Returns an instance of the language handler singleton. */
	static MSIMLanguageHandler & instance();

	/*! Destructor, removes language handler objects. */
	~MSIMLanguageHandler();

	/*! Returns current language ID. */
	static QString langId();

	/*! Sets the language Id in the settings object. */
	static void setLangId(QString id);

	/*! Installs the translator identified by langId and stores the
		language ID in the program settings. */
	void installTranslator(QString langId);

private:
	/*! The translater for the strings of the program itself. */
	QTranslator * applicationTranslator;
	/*! The translater for strings of the standard dialogs and other Qt library
		messages. */
	QTranslator * systemTranslator;

	/*! Upon construction the translator objects are created and
		the last used translation setting is installed. */
	MSIMLanguageHandler();

};

#endif // MSIMLanguageHandlerH
