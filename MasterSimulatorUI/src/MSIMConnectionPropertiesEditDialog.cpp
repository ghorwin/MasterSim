#include "MSIMConnectionPropertiesEditDialog.h"
#include "ui_MSIMConnectionPropertiesEditDialog.h"

#include <QMessageBox>

MSIMConnectionPropertiesEditDialog::MSIMConnectionPropertiesEditDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::MSIMConnectionPropertiesEditDialog)
{
	m_ui->setupUi(this);
}


MSIMConnectionPropertiesEditDialog::~MSIMConnectionPropertiesEditDialog() {
	delete m_ui;
}


int MSIMConnectionPropertiesEditDialog::edit(double & offset, double & scaleFactor) {
	m_ui->lineEditOffset->setText( tr("%L1").arg(offset) );
	m_ui->lineEditScaleFactor->setText( tr("%L1").arg(scaleFactor) );
	int res = exec();
	if (res == QDialog::Accepted) {
		bool ok;
		offset = QLocale().toDouble(m_ui->lineEditOffset->text(), &ok);
		if (!ok)
			offset = m_ui->lineEditOffset->text().toDouble(&ok);
		Q_ASSERT(ok);
		scaleFactor = QLocale().toDouble(m_ui->lineEditScaleFactor->text(), &ok);
		if (!ok)
			scaleFactor = m_ui->lineEditScaleFactor->text().toDouble(&ok);
		Q_ASSERT(ok);
	}
	return res;
}


void MSIMConnectionPropertiesEditDialog::accept() {
	bool ok;
	double value = QLocale().toDouble(m_ui->lineEditOffset->text(), &ok);
	if (!ok)
		value = m_ui->lineEditOffset->text().toDouble(&ok);
	if (!ok) {
		QMessageBox::critical(this, tr("Connection parameter error"), tr("Invalid value for offset."));
		m_ui->lineEditOffset->selectAll();
		m_ui->lineEditOffset->setFocus();
		return;
	}
	(void)value; // silence compiler wanin
	value = QLocale().toDouble(m_ui->lineEditScaleFactor->text(), &ok);
	if (!ok)
		value = m_ui->lineEditScaleFactor->text().toDouble(&ok);
	if (!ok) {
		QMessageBox::critical(this, tr("Connection parameter error"), tr("Invalid value for scale factor."));
		m_ui->lineEditScaleFactor->selectAll();
		m_ui->lineEditScaleFactor->setFocus();
		return;
	}
	(void)value; // silence compiler wanin
	QDialog::accept();
}
