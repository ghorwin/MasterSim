#include "MSIMLogWidget.h"

#include <QTextBrowser>
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>

#include <IBK_messages.h>

#include "MSIMUIConstants.h"
#include "MSIMDirectories.h"

MSIMLogWidget::MSIMLogWidget(QWidget *parent) :
	QWidget(parent)
{
	QVBoxLayout * lay = new QVBoxLayout;

	// *** setup textbrowser ***
	m_textBrowser = new QTextBrowser(this);
	m_textBrowser->setFontFamily( FIXED_FONT_FAMILY );
	m_textBrowser->setWordWrapMode(QTextOption::NoWrap);

	lay->addWidget(m_textBrowser);
	lay->setMargin(0);
	setLayout(lay);

	resize(800,250);
}


void MSIMLogWidget::showLogFile( const QString & logFilePath ) {
	m_textBrowser->clear();
	QFile f(logFilePath);
	if (!f.open(QFile::ReadOnly)) {
		m_textBrowser->append(tr("Cannot open logfile '%1'.").arg(logFilePath));
		return;
	}
	QTextStream strm(&f);
	QString line = strm.readLine();
	while (!line.isNull()) {
#define USE_PLAIN_TEXT_LOGWINDOW
#ifdef USE_PLAIN_TEXT_LOGWINDOW
		m_textBrowser->append(line);
#else
		// check for warning/error and color lines accordingly
		QString html = QString("<span style=\"font-family:'%1'; font-size:%2pt; color:%4\">%3</span><br>")
				.arg(FIXED_FONT_FAMILY).arg(TABLE_FONT_SIZE).arg(line);
		if (line.indexOf("[Warning") != -1)
			html = html.arg("#ffdd00");
		else if (line.indexOf("[Error") != -1)
			html = html.arg("#dd0000");
		else
			html = html.arg("#000000");
		m_textBrowser->insertHtml(html);
#endif // USE_PLAIN_TEXT_LOGWINDOW
		line = strm.readLine();
	}
}


void MSIMLogWidget::onMsgReceived(int type, QString msgText) {
#ifdef USE_PLAIN_TEXT_LOGWINDOW
	(void)type;
	m_textBrowser->append(msgText);
#else
	QString html = QString("<span style=\"font-family:'%1'; font-size:%2pt; color:%4\">%3</span><br>")
			.arg(FIXED_FONT_FAMILY).arg(TABLE_FONT_SIZE).arg(msgText);
	switch (type) {
		case IBK::MSG_WARNING :
			html = html.arg("#ffdd00");
			break;
		case IBK::MSG_ERROR :
			html = html.arg("#dd0000");
			break;
		default:
			html = html.arg("#000000");
	}
	m_textBrowser->insertHtml(html);
#endif // USE_PLAIN_TEXT_LOGWINDOW
}


void MSIMLogWidget::clear() {
	m_textBrowser->clear();
}
