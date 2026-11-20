#include "MSIMPreferencesPageGeneral.h"
#include "ui_MSIMPreferencesPageGeneral.h"

#include "MSIMSettings.h"

MSIMPreferencesPageGeneral::MSIMPreferencesPageGeneral(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::MSIMPreferencesPageGeneral)
{
	m_ui->setupUi(this);

#ifdef Q_OS_WIN
	m_ui->filepathTextEditor->setup("", true, true, tr("Executables (*.exe);;All files (*.*)"));
#else
	m_ui->filepathTextEditor->setup("", true, true, tr("All files (*.*)"));
#endif

	// order must match MSIMSettings::ColorScheme
	m_ui->comboBoxColorScheme->addItem(tr("Light"));
	m_ui->comboBoxColorScheme->addItem(tr("Dark"));
}


MSIMPreferencesPageGeneral::~MSIMPreferencesPageGeneral() {
	delete m_ui;
}


void MSIMPreferencesPageGeneral::updateUi() {
	MSIMSettings & s = MSIMSettings::instance();
	// transfer data to Ui
	m_ui->filepathTextEditor->setFilename(s.m_textEditorExecutable);
	m_ui->filepathPostProcExecutable->setFilename(s.m_postProcExecutable);
	m_ui->comboBoxColorScheme->setCurrentIndex(s.m_colorScheme);
}


bool MSIMPreferencesPageGeneral::storeConfig() {
	// no checks necessary
	MSIMSettings & s = MSIMSettings::instance();
	s.m_textEditorExecutable = m_ui->filepathTextEditor->filename();
	s.m_postProcExecutable = m_ui->filepathPostProcExecutable->filename();
#ifdef Q_OS_MAC
	int pos = s.m_postProcExecutable.indexOf("PostProcApp.app");
	if (pos != -1)
		s.m_postProcExecutable = s.m_postProcExecutable.left(pos) + "PostProcApp.app/Contents/MacOS/PostProcApp";
#endif

	MSIMSettings::ColorScheme colorScheme = (MSIMSettings::ColorScheme)m_ui->comboBoxColorScheme->currentIndex();
	if (colorScheme != s.m_colorScheme) {
		s.m_colorScheme = colorScheme;
		s.applyWidgetStyle();
	}

	return true;
}


