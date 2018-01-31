#include "MSIMLogFileDialog.h"
#include "ui_MSIMLogFileDialog.h"

#include <QFileInfo>
#include <QPushButton>
#include <QDialogButtonBox>

#include "MSIMSettings.h"
#include "MSIMProjectHandler.h"

MSIMLogFileDialog::MSIMLogFileDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::MSIMLogFileDialog),
	m_pushButtonOpenFileInTextEditor(NULL),
	m_pushButtonReloadProject(NULL)
{
	m_ui->setupUi(this);
	resize(1400,600);

	m_pushButtonOpenFileInTextEditor = m_ui->buttonBox->addButton(tr("Open file in text editor..."), QDialogButtonBox::ActionRole);
	m_pushButtonReloadProject = m_ui->buttonBox->addButton(tr("Reload project"), QDialogButtonBox::ActionRole);

	connect(m_pushButtonOpenFileInTextEditor, SIGNAL(clicked(bool)),
			this, SLOT(onOpenFileClicked(bool)));
	connect(m_pushButtonReloadProject, SIGNAL(clicked(bool)),
			this, SLOT(onReloadprojectClicked(bool)));
}


MSIMLogFileDialog::~MSIMLogFileDialog() {
	delete m_ui;
}


void MSIMLogFileDialog::setLogFile(const QString & logfilepath, QString projectfilepath, bool editFileButtonVisible) {
	m_logFilePath = logfilepath;
	m_projectFilePath = projectfilepath;
	m_ui->logWidget->showLogFile(m_logFilePath);
	setWindowTitle(QFileInfo(m_logFilePath).fileName());
	if (editFileButtonVisible) {
		m_pushButtonOpenFileInTextEditor->setVisible(true);
		m_pushButtonReloadProject->setVisible(true);
		m_ui->labelOpenFileError->setVisible(true);
		m_ui->labelOpenFileError->setText(tr("Error opening file '%1'.").arg(projectfilepath));
	}
	else {
		m_pushButtonOpenFileInTextEditor->setVisible(false);
		m_pushButtonReloadProject->setVisible(false);
		m_ui->labelOpenFileError->setVisible(false);
	}
}


void MSIMLogFileDialog::onOpenFileClicked(bool /*checked*/) {
	MSIMSettings::instance().openFileInTextEditor(this, m_projectFilePath);
}


void MSIMLogFileDialog::onReloadprojectClicked(bool /*checked*/) {
	const char * const FUNC_ID = "[MSIMLogFileDialog::onReloadprojectClicked]";

//	MSIMProjectHandler::instance().setReload();
	IBK::IBK_Message("\n------------------------------------------------------\n",IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message(IBK::FormatString("Reloading project '%1'\n").arg(IBK::Path(m_projectFilePath.toStdString())),IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message("------------------------------------------------------\n\n",IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	close();
}
