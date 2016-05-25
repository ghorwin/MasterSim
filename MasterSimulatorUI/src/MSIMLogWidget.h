#ifndef MSIMLogWidgetH
#define MSIMLogWidgetH

#include <QDialog>
class QTextBrowser;

/*! A widget with empedded text browser to show the content of the log file. */
class MSIMLogWidget : public QWidget {
	Q_OBJECT
public:
	explicit MSIMLogWidget(QWidget *parent = 0);

	/*! Shows application log file (for use in dialog). */
	void showLogFile( const QString & path);

public slots:
	/*! Clears the text browser, connected to the project handler's signal when
		a new project has been read.
	*/
	void clear();
	/*! Connected to message handler, appends the new message to the output (for use in dock widget). */
	void onMsgReceived(int type, QString msgText);

private:
	QTextBrowser * m_textBrowser;
};

#endif // MSIMLogWidgetH
