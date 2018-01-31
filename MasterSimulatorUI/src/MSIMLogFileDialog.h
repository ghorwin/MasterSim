#ifndef MSIMLogFileDialogH
#define MSIMLogFileDialogH

#include <QDialog>

namespace Ui {
	class MSIMLogFileDialog;
}

/*! A dialog that shows the a text/log file.
	Usage:
	\code
	MSIMLogFileDialog dlg;
	dlg.setLogFile("path/to/logfile.txt");
	dlg.exec();
	\endcode
*/
class MSIMLogFileDialog : public QDialog {
	Q_OBJECT

public:
	explicit MSIMLogFileDialog(QWidget *parent = 0);
	~MSIMLogFileDialog();

	/*! Call this function to tell the dialog to load the contents of the
		log file into it.
	*/
	void setLogFile(const QString & logfilepath, QString projectfilepath, bool editFileButtonVisible);

private slots:
	/*! Connected to custom button in button box. */
	void onOpenFileClicked(bool checked);

	/*! Connected to custom button in button box. */
	void onReloadprojectClicked(bool checked);

private:
	Ui::MSIMLogFileDialog	*m_ui;
	QPushButton				*m_pushButtonOpenFileInTextEditor;
	QPushButton				*m_pushButtonReloadProject;
	QString					m_projectFilePath;
	QString					m_logFilePath;
};

#endif // MSIMLogFileDialogH
