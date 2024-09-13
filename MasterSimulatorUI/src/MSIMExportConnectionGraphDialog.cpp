#include "MSIMExportConnectionGraphDialog.h"
#include "ui_MSIMExportConnectionGraphDialog.h"

#include <QPrintPreviewWidget>
#include <QPrintDialog>
#include <QPageSetupDialog>
#include <BM_ZoomMeshGraphicsView.h>
#include <BM_SceneManager.h>

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

	connect(m_ui->toolButtonAlignTop, &QToolButton::clicked,
			m_printPreviewWidget, &QPrintPreviewWidget::updatePreview);
	connect(m_ui->toolButtonAlignVCenter, &QToolButton::clicked,
			m_printPreviewWidget, &QPrintPreviewWidget::updatePreview);
	connect(m_ui->toolButtonAlignBottom, &QToolButton::clicked,
			m_printPreviewWidget, &QPrintPreviewWidget::updatePreview);
	connect(m_ui->toolButtonAlignLeft, &QToolButton::clicked,
			m_printPreviewWidget, &QPrintPreviewWidget::updatePreview);
	connect(m_ui->toolButtonAlignHCenter, &QToolButton::clicked,
			m_printPreviewWidget, &QPrintPreviewWidget::updatePreview);
	connect(m_ui->toolButtonAlignRight, &QToolButton::clicked,
			m_printPreviewWidget, &QPrintPreviewWidget::updatePreview);
}


MSIMExportConnectionGraphDialog::~MSIMExportConnectionGraphDialog() {
	delete m_ui;
	delete m_printer;
}


void MSIMExportConnectionGraphDialog::print(QPrinter *printer, bool showMarginFrame) {
	QPainter painter(printer);
	// NOTE: painter's coordinate system is located already at (margin-left, margin-top)

	double scale = m_ui->spinBoxScaleFactor->value()/100.;
	BLOCKMOD::SceneManager * scene = qobject_cast<BLOCKMOD::SceneManager *>(m_blockModWidget->scene());

	QRectF sourceRect = scene->itemsBoundingRect();
	// scale scene rect
	QRectF scaledSourceRect = QRectF(sourceRect.left()*scale, sourceRect.top()*scale, sourceRect.width()*scale, sourceRect.height()*scale);

	// we now need to compute source rect in scene
	QSizeF pageSize = printer->pageLayout().paintRectPixels(printer->resolution()).size();

	double xleft = 0;
	double ytop = 0;

	double pxc = pageSize.width()/2;
	double pyc = pageSize.height()/2;

	// some alignment math - compute left and top
	if (m_ui->toolButtonAlignHCenter->isChecked()) {
		xleft = pxc - 0.5*scaledSourceRect.width();
	}
	else if (m_ui->toolButtonAlignRight->isChecked()) {
		xleft = pageSize.width() - scaledSourceRect.width();
	}

	if (m_ui->toolButtonAlignVCenter->isChecked()) {
		ytop = pyc - 0.5*scaledSourceRect.height();
	}
	else if (m_ui->toolButtonAlignBottom->isChecked()) {
		ytop = pageSize.height() - scaledSourceRect.height();
	}
	if (showMarginFrame) {
		painter.save();
		QPen dash(Qt::DashLine);
		dash.setColor(Qt::gray);
		painter.setPen(dash);
		painter.drawRect(0,0,(int)pageSize.width(),(int)pageSize.height());
		painter.restore();
	}

	// now we can compute the target rectangle
	QRectF targetRect(xleft, ytop, scaledSourceRect.width(), scaledSourceRect.height());

	// if the targetRect is outside the pageRect, clip it on all four sides
	if (targetRect.left() < 0) {
		double deltaX = - targetRect.left();
		targetRect.setLeft( targetRect.left() + deltaX);
		scaledSourceRect.setLeft( scaledSourceRect.left() + deltaX);
	}

	if (targetRect.right() > pageSize.width()) {
		double deltaX = targetRect.right() - pageSize.width();
		targetRect.setRight( targetRect.right() - deltaX);
		scaledSourceRect.setRight( scaledSourceRect.right() - deltaX);
	}

	if (targetRect.top() < 0) {
		double deltaY = - targetRect.top();
		targetRect.setTop( targetRect.top() + deltaY);
		scaledSourceRect.setTop( scaledSourceRect.top() + deltaY);
	}

	if (targetRect.bottom() > pageSize.height()) {
		double deltaY = targetRect.bottom() - pageSize.height();
		targetRect.setBottom( targetRect.bottom() - deltaY);
		scaledSourceRect.setBottom( scaledSourceRect.bottom() - deltaY);
	}

	// scale source rect back to scene rect
	sourceRect = QRectF(scaledSourceRect.left()/scale, scaledSourceRect.top()/scale, scaledSourceRect.width()/scale, scaledSourceRect.height()/scale);
	scene->render(&painter, targetRect, sourceRect, Qt::IgnoreAspectRatio);

}


void MSIMExportConnectionGraphDialog::on_spinBoxScaleFactor_valueChanged(int) {
	m_printPreviewWidget->updatePreview();
}


void MSIMExportConnectionGraphDialog::on_pushButtonPageSetup_clicked() {
	QPageSetupDialog dlg(m_printer, this);
	dlg.exec();
	m_printPreviewWidget->updatePreview();
}


void MSIMExportConnectionGraphDialog::on_pushButtonPrint_clicked() {
	QPrintDialog dlg(m_printer, this);
	if (dlg.exec() == QDialog::Accepted) {
		print(m_printer, false);
		close();
	}
}


void MSIMExportConnectionGraphDialog::on_buttonBox_clicked(QAbstractButton *) {
	close(); // only one button
}


void MSIMExportConnectionGraphDialog::showEvent(QShowEvent * event) {
	QWidget::showEvent(event);
	m_printPreviewWidget->updatePreview();
}
