#include "MSIMSlaveTableWidget.h"

#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>

#include "MSIMUIConstants.h"
#include "MSIMSlaveItemDelegate.h"

MSIMSlaveTableWidget::MSIMSlaveTableWidget(QWidget * parent) :
	QTableWidget(parent)
{
	// setup tables
//	m_ui->tableWidgetFMUs->horizontalHeader()->setVisible(false);
//	m_ui->tableWidgetFMUs->verticalHeader()->setVisible(false);
//	m_ui->tableWidgetFMUs->setColumnCount(1);
	QStringList headers;
//	headers << tr("Full path");
//	m_ui->tableWidgetFMUs->setHorizontalHeaderLabels(headers);
//	m_ui->tableWidgetFMUs->horizontalHeader()->setStretchLastSection(true);

//	m_ui->tableWidgetSlaves->horizontalHeader()->setVisible(false);
	verticalHeader()->setVisible(false);
	setColumnCount(4);
	headers.clear();
	headers << "" << tr("Name") << tr("FMU") << tr("Cycle Nr.");
	setHorizontalHeaderLabels(headers);

	formatTable(this);

	horizontalHeader()->resizeSection(0, verticalHeader()->defaultSectionSize());
	setItemDelegate(new MSIMSlaveItemDelegate(this));

}


void MSIMSlaveTableWidget::resizeEvent(QResizeEvent *event) {
	// get space of first and last column
	int column1 = horizontalHeader()->sectionSize(0);
	int column2 = horizontalHeader()->sectionSize(1);
	int column4 = horizontalHeader()->sectionSize(3);
	int column3 = width()-2-column1-column2-column4;
	horizontalHeader()->resizeSection(2, column3);
	QTableWidget::resizeEvent(event);
}
