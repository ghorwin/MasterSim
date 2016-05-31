#include "MSIMViewSlaves.h"
#include "ui_MSIMViewSlaves.h"

MSIMViewSlaves::MSIMViewSlaves(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::MSIMViewSlaves)
{
	m_ui->setupUi(this);
	m_ui->gridLayout->setMargin(0);
}


MSIMViewSlaves::~MSIMViewSlaves() {
	delete m_ui;
}
