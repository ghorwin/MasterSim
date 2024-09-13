#include "MSIMExportConnectionGraphDialog.h"
#include "ui_MSIMExportConnectionGraphDialog.h"

#include <QPrintPreviewWidget>
#include <QPageSetupDialog>
#include <BM_ZoomMeshGraphicsView.h>

MSIMExportConnectionGraphDialog::MSIMExportConnectionGraphDialog(QWidget *parent, BLOCKMOD::ZoomMeshGraphicsView * blockModWidget) :
	QDialog(parent),
	m_ui(new Ui::MSIMExportConnectionGraphDialog),
	m_printer(new QPrinter),
	m_blockModWidget(blockModWidget)
{
	m_ui->setupUi(this);
	m_ui->verticalLayoutPrintOptions->setMargin(0);

	// construct print preview widget
	m_printPreviewWidget = new QPrintPreviewWidget(m_printer, this);
	m_ui->horizontalLayoutPrint->addWidget(m_printPreviewWidget);
	m_ui->horizontalLayoutPrint->setStretch(1,1);

	connect(m_printPreviewWidget, &QPrintPreviewWidget::paintRequested,
			this, &MSIMExportConnectionGraphDialog::renderPrintPreview);
}


MSIMExportConnectionGraphDialog::~MSIMExportConnectionGraphDialog() {
	delete m_ui;
	delete m_printer;
}


void MSIMExportConnectionGraphDialog::renderPrintPreview(QPrinter *printer) {
	QPainter painter(printer);

	QTransform trans;
	int scale = m_ui->spinBoxScaleFactor->value();
	trans.scale(scale/100., scale/100.);
	painter.setTransform(trans);
	m_blockModWidget->render(&painter);
}



void MSIMExportConnectionGraphDialog::on_spinBoxScaleFactor_valueChanged(int) {
	m_printPreviewWidget->updatePreview();
}


void MSIMExportConnectionGraphDialog::on_pushButtonPrintSetup_clicked() {
	QPageSetupDialog dlg(this);
	dlg.exec();
}
