#ifndef MSIMWelcomeScreenH
#define MSIMWelcomeScreenH

#include <QWidget>
#include <QUrl>

namespace Ui {
	class MSIMWelcomeScreen;
}

class QNetworkAccessManager;
class QNetworkReply;

/*! The welcome screen, showing news and recently projects. */
class MSIMWelcomeScreen : public QWidget {
	Q_OBJECT

public:
	explicit MSIMWelcomeScreen(QWidget *parent = 0);
	~MSIMWelcomeScreen();

	/*! Updates the welcome page content. */
	void updateWelcomePage();

public slots:
	/*! Triggered when user clicks on a project file or external link. */
	void onAnchorClicked( const QUrl & link );

signals:
	/*! Emitted when user clicked on the file name of an example/recently used file. */
	void openProject(const QString & projectFile);

	void newProjectClicked();
	void openProjectClicked();
	void updateRecentList();

private slots:
	void on_toolButtonNewProject_clicked();
	void on_toolButtonOpenProject_clicked();
	void downloadFinished(QNetworkReply *reply);

private:
	Ui::MSIMWelcomeScreen	*m_ui;

	QNetworkAccessManager	*m_networkManager;

	/*! Holds the news content for the welcome page. */
	QString					m_welcomePageNews;

	/*! If true, the start page shows recently opened files instead of demos and examples. */
	bool					m_showRecentlyOpenedFiles;
};

#endif // MSIMWelcomeScreenH
