#include "MSIMAboutDialog.h"
#include "ui_MSIMAboutDialog.h"

#include <MSIM_Constants.h>

MSIMAboutDialog::MSIMAboutDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::MSIMAboutDialog)
{
	m_ui->setupUi(this);

	setWindowTitle(QString("MasterSimulator %1").arg(MASTER_SIM::LONG_VERSION));
}


MSIMAboutDialog::~MSIMAboutDialog() {
	delete m_ui;
}
