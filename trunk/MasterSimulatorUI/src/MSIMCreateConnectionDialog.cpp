#include "MSIMCreateConnectionDialog.h"
#include "ui_MSIMCreateConnectionDialog.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>

#include "MSIMUIConstants.h"
#include "MSIMMainWindow.h"

MSIMCreateConnectionDialog::MSIMCreateConnectionDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::MSIMCreateConnectionDialog)
{
	m_ui->setupUi(this);

	// setup tables
	m_ui->tableWidgetOutputVariable->verticalHeader()->setVisible(false);
	m_ui->tableWidgetOutputVariable->setColumnCount(3);
	QStringList headers;
	headers << tr("Slave") << tr("Variable") << tr("Type");
	m_ui->tableWidgetOutputVariable->setHorizontalHeaderLabels(headers);
	m_ui->tableWidgetOutputVariable->setSortingEnabled(true);
	m_ui->tableWidgetOutputVariable->horizontalHeader()->setSortIndicatorShown(true);
	m_ui->tableWidgetOutputVariable->sortByColumn(0, Qt::AscendingOrder);

	m_ui->tableWidgetInputVariable->verticalHeader()->setVisible(false);
	m_ui->tableWidgetInputVariable->setColumnCount(3);
	headers.clear();
	headers << tr("Slave") << tr("Variable") << tr("Type");
	m_ui->tableWidgetInputVariable->setHorizontalHeaderLabels(headers);
	m_ui->tableWidgetInputVariable->setSortingEnabled(true);
	m_ui->tableWidgetInputVariable->horizontalHeader()->setSortIndicatorShown(true);
	m_ui->tableWidgetInputVariable->sortByColumn(0, Qt::AscendingOrder);

	formatTable(m_ui->tableWidgetOutputVariable);
	formatTable(m_ui->tableWidgetInputVariable);
}


MSIMCreateConnectionDialog::~MSIMCreateConnectionDialog() {
	delete m_ui;
}


void MSIMCreateConnectionDialog::updateTables() {
	// remember select rows
	int outputVarRow = m_ui->tableWidgetOutputVariable->currentRow();
	int inputVarRow = m_ui->tableWidgetInputVariable->currentRow();

	// first clear tables
	m_ui->tableWidgetOutputVariable->clearContents();
	m_ui->tableWidgetInputVariable->clearContents();

	// process all FMUs currently loaded

}
