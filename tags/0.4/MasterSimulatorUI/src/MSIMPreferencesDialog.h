#ifndef MSIMPreferencesDialogH
#define MSIMPreferencesDialogH

#include <QDialog>

class MSIMPreferencesPageGeneral;

class QListWidgetItem;

namespace Ui {
	class MSIMPreferencesDialog;
}

/*! Implementation of the preferences dialog. */
class MSIMPreferencesDialog : public QDialog {
	Q_OBJECT
public:
	/*! Constructor.*/
	MSIMPreferencesDialog(QWidget * parent);
	/*! Destructor. */
	~MSIMPreferencesDialog();

	/*! Spawns the dialog and returns when user has closed the dialog.
		\return Returns true if user has changed preferences (some of which may require an update of views).
	*/
	bool edit();

private slots:
	/*! Triggered when different page is selected via list widget icon. */
	void on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
	/*! Triggered when different page is selected via row index. */
	void on_listWidget_currentRowChanged(int currentRow);

protected:
	/*! QDialog::accept() re-implemented for input data checking (called indirectly from buttonBox). */
	void accept();

private:
	/*! Transfers values from Settings object to user interface (config pages).*/
	void updateUi();

	/*! Transfers the current settings from all configuration pages into
		the settings object.
		If one of the options was set wrong, the function will pop up a dialog
		asking the user to fix it.
		\return Returns true, if all settings were successfully stored. Otherwise
				 false which signals that the dialog must not be closed, yet.
	*/
	bool storeConfig();

	/*! GUI member. */
	Ui::MSIMPreferencesDialog		*m_ui;

	/*! The general preferences page. */
	MSIMPreferencesPageGeneral		*m_pageGeneral;
};

#endif // MSIMPreferencesDialogH
