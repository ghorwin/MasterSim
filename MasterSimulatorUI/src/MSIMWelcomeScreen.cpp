#include "MSIMWelcomeScreen.h"
#include "ui_MSIMWelcomeScreen.h"

#include <QDesktopServices>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTextStream>
#include <QNetworkAccessManager>
#include <QPixmap>
#include <QUrl>

#include <MSIM_Project.h>

#include "MSIMSettings.h"
#include "MSIMDirectories.h"
#include "MSIMConversion.h"
#include "MSIMLanguageHandler.h"
#include "MSIMUIConstants.h"

extern const char * const HTML_TEMPLATE;
extern const char * const RECENT_PROJECT_TABLE_TEMPLATE;

MSIMWelcomeScreen::MSIMWelcomeScreen(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::MSIMWelcomeScreen),
	m_networkManager(new QNetworkAccessManager),
	m_showRecentlyOpenedFiles(true)
{
	m_ui->setupUi(this);
	connect(m_networkManager, SIGNAL(finished(QNetworkReply*)),
			this, SLOT(downloadFinished(QNetworkReply*)));
	connect(m_ui->textBrowser, SIGNAL(anchorClicked(QUrl)),
			this, SLOT(onAnchorClicked(QUrl)));
}


MSIMWelcomeScreen::~MSIMWelcomeScreen() {
	delete m_ui;
}


void MSIMWelcomeScreen::updateWelcomePage() {
	// download news content (only once per application start)
	if (m_welcomePageNews.isEmpty()) {
		// download file
		QNetworkRequest request; //(QUrl(NEWS_URL));
		/*QNetworkReply *reply = */m_networkManager->get(request);
	}
	// now create overall page
	QString htmlPage = HTML_TEMPLATE;
	// insert news
	if (m_welcomePageNews.isEmpty()) {
		htmlPage.replace("${NEWS}", QString("<i>%1</i>")
						 .arg(tr("Retrieving news content")));
	}
	else {
		htmlPage.replace("${NEWS}", m_welcomePageNews);
	}
	htmlPage.replace("${RECENT_PROJECTS_LINK}", tr("Recently opened projects"));
	htmlPage.replace("${DEMO_CASES_LINK}", tr("Examples/Validation cases"));

	if (m_showRecentlyOpenedFiles) {
		// compose recent project file list table

		QString recentProjects = "<h1>" + tr("Recently opened projects") + "</h1><p>\n";
		for (int i=0; i<MSIMSettings::instance().m_recentProjects.count(); ++i) {
			QFileInfo finfo(MSIMSettings::instance().m_recentProjects[i]);
			QString recentProjectTable = RECENT_PROJECT_TABLE_TEMPLATE;
			QString description;
			// check if file still exists
			if (finfo.exists()) {
				MASTER_SIM::Project pro;
				try {
					pro.read(IBK::Path(finfo.filePath().toUtf8().data()), true);
				}
				catch (IBK::Exception &) {
					// error reading project file, missing permissions?
					pro = MASTER_SIM::Project(); // reset with empty instance
				}
				QString created;
				if (pro.m_created.empty())
					created = "---";
				else
					created = utf82QString(pro.m_created);
				QString lastModified;
				if (pro.m_lastEdited.empty())
					lastModified = "---";
				else
					lastModified = utf82QString(pro.m_lastEdited);
				description = tr("<i>Created: %1, Last modified: %2</i><br>%3")
									  .arg(created)
									  .arg(lastModified)
									  .arg(utf82QString(pro.m_comment));
			}
			else {
				description = tr("<i><font color=\"#800000\">Project not accessible</font></i> <a href=\"projectRemove:%1\">Remove %2</a>").arg( i ).arg( finfo.fileName() );
			}

			QString thumbPath = MSIMDirectories::userDataDir()  + "/thumbs/" + finfo.fileName() + ".png";

			QFileInfo thumbFileInfo(thumbPath);
			// check if file exists
			if (thumbFileInfo.exists())
				thumbPath = "<a href=\"project:${PROJECT_FULL_PATH}\"><img src=\"" + thumbFileInfo.absoluteFilePath() + "\"></img></a>&nbsp;";
			else
				thumbPath = "&nbsp;";
			thumbPath = thumbPath.replace("${PROJECT_FULL_PATH}", finfo.filePath());

			recentProjectTable = recentProjectTable.replace("${THUMBNAILSIZE}", QString("%1").arg(MSIMSettings::instance().m_thumbNailSize+8));
			recentProjectTable = recentProjectTable.replace("${PROJECT_FILENAME}", finfo.fileName());
			recentProjectTable = recentProjectTable.replace("${PROJECT_FULL_PATH}", finfo.filePath());
			recentProjectTable = recentProjectTable.replace("${PROJECT_DESCRIPTION}", description);
			recentProjectTable = recentProjectTable.replace("${IMG_FILENAME}", thumbPath);
			recentProjects += recentProjectTable;
		}
		htmlPage.replace("${RECENT_PROJECT_FILELIST}", recentProjects);
	}
	else {
		QString recentProjects = "<h1>" + tr("Examples/Validation cases") + "</h1><p>\n";

		// process all files in examples directory
		QDir examplesDir(MSIMDirectories::resourcesRootDir() + "/examples");
		QStringList exampleFiles;
		MSIMSettings::recursiveSearch(examplesDir, exampleFiles, QStringList() << "msim");

		// for each file, parse its header, extract comments, open thumbnail image and compose table with information
		foreach (QString fname, exampleFiles) {
			MASTER_SIM::Project pro;
			try {
				pro.read(IBK::Path(fname.toUtf8().data()), true);
				QString recentProjectTable = RECENT_PROJECT_TABLE_TEMPLATE;
				QFileInfo finfo(fname);
				recentProjectTable = recentProjectTable.replace("${PROJECT_FILENAME}", finfo.fileName());
				recentProjectTable = recentProjectTable.replace("${PROJECT_FULL_PATH}", finfo.filePath());

				QString description = utf82QString(pro.m_comment);
				recentProjectTable = recentProjectTable.replace("${PROJECT_DESCRIPTION}", description);

				// open thumbnail image for this project
				QString thumbPath = fname+".png";

				QFileInfo thumbFileInfo(thumbPath);
				QPixmap p;
				// check if file exists
				if (thumbFileInfo.exists() && p.load(thumbPath)) {
					thumbPath = "<a href=\"project:${PROJECT_FULL_PATH}\"><img src=\"" + thumbFileInfo.absoluteFilePath() + "\"></img></a>&nbsp;";
					thumbPath = thumbPath.replace("${PROJECT_FULL_PATH}", finfo.filePath());
					recentProjectTable = recentProjectTable.replace("${THUMBNAILSIZE}", QString("%1").arg(p.width()));
				}
				else {
					thumbPath = "&nbsp;";
					recentProjectTable = recentProjectTable.replace("${THUMBNAILSIZE}", QString("%1").arg(MSIMSettings::instance().m_thumbNailSize+8));
				}

				recentProjectTable = recentProjectTable.replace("${IMG_FILENAME}", thumbPath);
				recentProjects += recentProjectTable;
			}
			catch (IBK::Exception &) {
				// error reading project file, missing permissions?
				pro = MASTER_SIM::Project(); // reset with empty instance
			}
		}

		htmlPage.replace("${RECENT_PROJECT_FILELIST}", recentProjects);
	}

	// finally set welcome page in textbrowser
	m_ui->textBrowser->setHtml(htmlPage);
}


void MSIMWelcomeScreen::onAnchorClicked( const QUrl & link ) {
	// if anchor starts with "project:" we emit the "open project" signal
	if (link.toString().startsWith("project:")) {
		QString fname = link.toString();
		fname = fname.right(fname.length()-8);
		emit openProject(fname);
		return;
	}
	else if (link.toString().startsWith("projectRemove:")) {

		// extract project index to delete and remove it from list
		QString index = link.toString();
		index = index.right(index.length()-9);
		bool ok;
		int ind = index.toInt(&ok);
		if (ok && ind < MSIMSettings::instance().m_recentProjects.size()){
			MSIMSettings::instance().m_recentProjects.removeAt(ind);
		}

		// refresh view afterwards
		updateWelcomePage();

		// refresh menue structure
		emit updateRecentList();

		return;
	}
	else if (link.toString().startsWith("page:")) {
		QString page = link.toString().right(link.toString().length()-5);
		if (page == "recentFiles") {
			m_showRecentlyOpenedFiles = true;
		}
		else if (page == "demoFiles") {
			m_showRecentlyOpenedFiles = false;
		}
		updateWelcomePage();
		return;
	}
	// must be an external link, call desktop services to open webbrowser
	QDesktopServices::openUrl(link);
}


void MSIMWelcomeScreen::on_toolButtonNewProject_clicked() {
	emit newProjectClicked();
}


void MSIMWelcomeScreen::on_toolButtonOpenProject_clicked() {
	emit openProjectClicked();
}


void MSIMWelcomeScreen::downloadFinished(QNetworkReply *reply) {
	if (reply->error()) {
		m_welcomePageNews = QString("<i>%1</i>")
							.arg(tr("unable to download news content"));
	}
	else {
		QByteArray newsRaw = reply->readAll();
		// extract text for current language
		QString news = QString::fromUtf8(newsRaw);
		if (news.indexOf("[lang:")) {
			QStringList langTexts = news.split("[lang:");
			for (int i=0; i<langTexts.count(); ++i) {
				QString langHeader = langTexts[i].left(8);
				if (langHeader.count() == 8) {
					QString langId = langHeader.right(2);
					if (langId == MSIMLanguageHandler::instance().langId()) {
						news = langTexts[i].right(langTexts[i].count()-8);
					}
				}
			}
		}
		m_welcomePageNews = QString("<p>&nbsp;<p><hr>") + news;
	}

	reply->deleteLater();
	// this will trigger a welcome page update (but no further download attempt)
	updateWelcomePage();
}



const char * const HTML_TEMPLATE =
		"<html>\n"
		"<head>\n"
		"<!-- <link rel=stylesheet type=\"text/css\" href=\"welcome.css\"> -->\n"
		"<style type=\"text/css\">\n"
		"body     { font-size: medium; color: black; background-color: white }\n"
		"a        { color: #0053A6; text-decoration:none }\n"
		"a:hover  { color: #1C7DEF; background-color: white }\n"
		"p        { font-size: medium; text-align: justify; margin-top:0px; margin-bottom:8px;   }\n"
		"h1       { font-size: large; color: #003264; font-weight:bold; \n"
		"           text-decoration: none; margin-top:15px; margin-bottom:15px }\n"
		"h2       { font-size: medium; color: #0069A8; font-weight:bold; margin-top:15px; margin-bottom:6px }\n"
		"h3       { font-size: medium; color: #00660F; font-weight:bold; margin-top:10px; margin-bottom:2px}\n"
		"table    { font-size: medium }\n"
		"b        { color: black }\n"
		"li       { text-align: justify }\n"
		"</style>\n"
		"\n"
		"</head>\n"
		"<body>\n"
		"<table border=\"0\" cellspacing=\"10\" cellpadding=\"0\" width=\"100%\">\n"
		"<tr valign=\"top\">\n"
		"<td>\n"
		"<p>\n"
		"[ <a href=\"page:recentFiles\">${RECENT_PROJECTS_LINK}</a> ] [ <a href=\"page:demoFiles\">${DEMO_CASES_LINK}</a> ]\n"
		"<p>\n"
		"<hr>\n"
		"${RECENT_PROJECT_FILELIST}\n"
		"\n"
		"</td>\n"
		"<td width=30>&nbsp;</td>\n"
		"<td width=300>\n"
		"${NEWS}\n"
		"</td>\n"
		"\n"
		"</tr>\n"
		"</table>\n"
		"\n"
		"</body>\n"
		"</html>\n"
		"\n";


const char * const RECENT_PROJECT_TABLE_TEMPLATE =
		"<table border=\"0\" cellspacing=\"2\" cellpadding=\"0\">\n"
		"<tr valign=center><th width=\"${THUMBNAILSIZE}\" rowspan=\"3\">${IMG_FILENAME}</th><th align=left>${PROJECT_FILENAME}</th></tr>\n"
		"<tr valign=center><td align=left><a href=\"project:${PROJECT_FULL_PATH}\">${PROJECT_FULL_PATH}</a></td></tr>\n"
		"<tr valign=top><td align=justify>${PROJECT_DESCRIPTION}</td></tr>\n"
		"</table>\n"
		"<br>\n"
		"\n";


