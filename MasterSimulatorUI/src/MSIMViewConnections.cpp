#include "MSIMViewConnections.h"
#include "ui_MSIMViewConnections.h"

#include <QSortFilterProxyModel>

#include <IBK_Exception.h>

#include "MSIMUIConstants.h"
#include "MSIMProjectHandler.h"

#include "MSIMSlaveItemDelegate.h"
#include "MSIMConnectionItemDelegate.h"
#include "MSIMConversion.h"


MSIMViewConnections::MSIMViewConnections(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::MSIMViewConnections)
{
	m_ui->setupUi(this);
	m_ui->verticalLayout->setContentsMargins(9,0,9,9);

	connect(&MSIMProjectHandler::instance(), SIGNAL(modified(int,void*)), this, SLOT(onModified(int,void*)));

	// setup tables
	m_ui->tableWidgetConnections->verticalHeader()->setVisible(false);
	m_ui->tableWidgetConnections->setColumnCount(2);
	QStringList headers;
	headers << tr("Output variable") << tr("Input variable");
	m_ui->tableWidgetConnections->setHorizontalHeaderLabels(headers);
	m_ui->tableWidgetConnections->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
	m_ui->tableWidgetConnections->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);

	m_ui->tableWidgetSlaves->verticalHeader()->setVisible(false);
	m_ui->tableWidgetSlaves->setColumnCount(1);
	headers.clear();
	headers << tr("Simulator Name");
	m_ui->tableWidgetSlaves->setHorizontalHeaderLabels(headers);
	m_ui->tableWidgetSlaves->horizontalHeader()->setStretchLastSection(true);
	m_ui->tableWidgetConnections->horizontalHeader()->setSortIndicatorShown(true);
	m_ui->tableWidgetConnections->sortByColumn(0, Qt::AscendingOrder);

	formatTable(m_ui->tableWidgetConnections);
	formatTable(m_ui->tableWidgetSlaves);
}


MSIMViewConnections::~MSIMViewConnections() {
	delete m_ui;
}


void MSIMViewConnections::onModified( int modificationType, void * /* data */) {
	switch ((MSIMProjectHandler::ModificationTypes)modificationType) {
		case MSIMProjectHandler::AllModified :
		case MSIMProjectHandler::SlavesModified : // slaves may have been renamed
			break;
		default:
			return; // nothing to do for us
	}

	blockMySignals(this, true);

	/// \todo store current check states before clearing table
	/// \todo store current sort column

	// update tables based on project file content
	m_ui->tableWidgetSlaves->clearContents();

	m_ui->comboBoxSlave1->clear();
	m_ui->comboBoxSlave2->clear();

	m_ui->tableWidgetSlaves->setRowCount(project().m_simulators.size());
	for (unsigned int i=0; i<project().m_simulators.size(); ++i) {
		const MASTER_SIM::Project::SimulatorDef & simDef = project().m_simulators[i];
		QString slaveName = QString::fromUtf8(simDef.m_name.c_str());
		QTableWidgetItem * item = new QTableWidgetItem( slaveName );
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
		item->setCheckState(Qt::Checked);
		item->setData(Qt::TextColorRole, QColor(simDef.m_color.toQRgb()));
		m_ui->tableWidgetSlaves->setItem(i, 0, item);

		m_ui->comboBoxSlave1->addItem(slaveName);
		m_ui->comboBoxSlave2->addItem(slaveName);
	}

	if (m_ui->comboBoxSlave1->count() > 0) {
		m_ui->comboBoxSlave1->setCurrentIndex(0);
		m_ui->comboBoxSlave2->setCurrentIndex(0);
	}

	blockMySignals(this, false);

	updateConnectionsTable();
}


void MSIMViewConnections::on_pushButton_clicked() {
	// check that slave names are not the same

	// find all matches
	// create graph edges
	// create undo action for graph editing
}


void MSIMViewConnections::on_tableWidgetSlaves_cellChanged(int /* row */, int /* column */) {
	updateConnectionsTable();
}


void MSIMViewConnections::updateConnectionsTable() {
	blockMySignals(this, true);

	std::set<std::string> checkedSlaveNames;
	for (int i=0; i<m_ui->tableWidgetSlaves->rowCount(); ++i) {
		if (m_ui->tableWidgetSlaves->item(i,0)->checkState() == Qt::Checked)
			checkedSlaveNames.insert(m_ui->tableWidgetSlaves->item(i,0)->text().toUtf8().data());
	}

	m_ui->tableWidgetConnections->clearContents();
	m_ui->tableWidgetConnections->setRowCount(0);
	m_ui->tableWidgetConnections->setSortingEnabled(false);
	for (unsigned int i=0; i<project().m_graph.size(); ++i) {
		const MASTER_SIM::Project::GraphEdge & edge = project().m_graph[i];
		QTableWidgetItem * outItem = new QTableWidgetItem( QString::fromStdString(edge.m_outputVariableRef)); // no utf8 here
		outItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		std::string inputSlaveName;
		std::string outputSlaveName;
		try {
			outputSlaveName = edge.outputSlaveName();
			// find slave in list of slaves
			const MASTER_SIM::Project::SimulatorDef & simDef = project().simulatorDefinition(outputSlaveName);
			outItem->setData(Qt::TextColorRole, QColor(simDef.m_color.toQRgb()));
		}
		catch (IBK::Exception & ex) {
			ex.writeMsgStackToError();
			outItem->setToolTip( QString::fromStdString(ex.what())); /// \todo translation of error message to user?
			outItem->setBackground( QColor("#B22222") );
			outItem->setData(Qt::UserRole+1, true);
		}

		QTableWidgetItem * inItem = new QTableWidgetItem( QString::fromStdString(edge.m_inputVariableRef)); // no utf8 here
		inItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		try {
			inputSlaveName = edge.inputSlaveName();
			// find slave in list of slaves
			const MASTER_SIM::Project::SimulatorDef & simDef = project().simulatorDefinition(inputSlaveName);
			inItem->setData(Qt::TextColorRole, QColor(simDef.m_color.toQRgb()));
		}
		catch (IBK::Exception & ex) {
			ex.writeMsgStackToError();
			inItem->setToolTip( QString::fromStdString(ex.what())); /// \todo translation of error message to user?
			inItem->setBackground( QColor("#B22222") );
			inItem->setData(Qt::UserRole+1, true);
		}

		// check if both input and output slaves are selected
		if (checkedSlaveNames.find(inputSlaveName) != checkedSlaveNames.end() &&
			checkedSlaveNames.find(outputSlaveName) != checkedSlaveNames.end())
		{
			int currentRow = m_ui->tableWidgetConnections->rowCount();
			m_ui->tableWidgetConnections->setRowCount(currentRow+1);

			m_ui->tableWidgetConnections->setItem(currentRow, 0, outItem);
			m_ui->tableWidgetConnections->setItem(currentRow, 1, inItem);
		}
		else {
			delete outItem;
			delete inItem;
		}
	}

	m_ui->tableWidgetConnections->setSortingEnabled(true);

	blockMySignals(this, false);
}

