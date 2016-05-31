#include "MSIMViewConnections.h"
#include "ui_MSIMViewConnections.h"

MSIMViewConnections::MSIMViewConnections(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::MSIMViewConnections)
{
	m_ui->setupUi(this);
}


MSIMViewConnections::~MSIMViewConnections() {
	delete m_ui;
}
