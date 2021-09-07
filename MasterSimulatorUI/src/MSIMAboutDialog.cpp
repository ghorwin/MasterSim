#include "MSIMAboutDialog.h"
#include "ui_MSIMAboutDialog.h"

#include <MSIM_Constants.h>

MSIMAboutDialog::MSIMAboutDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::MSIMAboutDialog)
{
	m_ui->setupUi(this);

	setWindowTitle(QString("MasterSimulator %1").arg(MASTER_SIM::LONG_VERSION));

	QString labelStyle(
				"font-size:10pt; color: #2121a6; text-decoration:none"
				);

	QLabel * linkLabel = new QLabel( QString("<a href=\"https://bauklimatik-dresden.de/mastersim\"><span style=\"%1\">https://bauklimatik-dresden.de/mastersim</span></a>").arg(labelStyle));
	linkLabel->setParent(this);
	linkLabel->resize(400,25);
	linkLabel->setAutoFillBackground(false);
	linkLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	linkLabel->setOpenExternalLinks(true);
	linkLabel->move(28,487);
	linkLabel->setAttribute(Qt::WA_TranslucentBackground);

	layout()->setMargin(0);
	layout()->setSizeConstraint( QLayout::SetFixedSize );
}


MSIMAboutDialog::~MSIMAboutDialog() {
	delete m_ui;
}
