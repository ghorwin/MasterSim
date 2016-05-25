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

	m_ui->lineEditTerminal->setVisible(false);
	m_ui->checkBoxUseTerminal->setVisible(false);
	m_ui->labelUseTerminal->setVisible(false);

	/// \todo Qt Support: Tab order in custom widgets...
}


MSIMPreferencesPageGeneral::~MSIMPreferencesPageGeneral() {
	delete m_ui;
}


void MSIMPreferencesPageGeneral::updateUi() {
	MSIMSettings & s = MSIMSettings::instance();
	// transfer data to Ui
	m_ui->filepathTextEditor->setFilename(s.m_textEditorExecutable);
}


bool MSIMPreferencesPageGeneral::storeConfig() {
	// no checks necessary
	MSIMSettings & s = MSIMSettings::instance();
	s.m_textEditorExecutable = m_ui->filepathTextEditor->filename();

	return true;
}


