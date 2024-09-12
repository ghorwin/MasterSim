#include "MSIMExportConnectionGraphDialog.h"
#include "ui_MSIMExportConnectionGraphDialog.h"

MSIMExportConnectionGraphDialog::MSIMExportConnectionGraphDialog(QWidget *parent, BLOCKMOD::ZoomMeshGraphicsView * blockModWidget) :
	QDialog(parent),
	m_ui(new Ui::MSIMExportConnectionGraphDialog),
	m_blockModWidget(blockModWidget)
{
	m_ui->setupUi(this);
}


MSIMExportConnectionGraphDialog::~MSIMExportConnectionGraphDialog() {
	delete m_ui;
}


