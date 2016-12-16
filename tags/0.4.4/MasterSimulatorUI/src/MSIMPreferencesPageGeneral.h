#ifndef MSIMPreferencesPageGeneralH
#define MSIMPreferencesPageGeneralH

#include <QWidget>

namespace Ui {
	class MSIMPreferencesPageGeneral;
}

/*! The configuration page with general user preferences. */
class MSIMPreferencesPageGeneral : public QWidget {
	Q_OBJECT
	Q_DISABLE_COPY(MSIMPreferencesPageGeneral)
public:
	/*! Default constructor. */
	explicit MSIMPreferencesPageGeneral(QWidget *parent = 0);
	/*! Destructor. */
	~MSIMPreferencesPageGeneral();

	/*! Updates the user interface with values in Settings object.*/
	void updateUi();

	/*! Transfers the current settings from the configuration page into
		the settings object.
		If one of the options was set wrong, the function will pop up a dialog
		asking the user to fix it.
		\return Returns true, if all settings were successfully stored. Otherwise
				 false which signals that the dialog must not be closed, yet.
	*/
	bool storeConfig();

private:
	Ui::MSIMPreferencesPageGeneral *m_ui;
};


#endif // MSIMPreferencesPageGeneralH
