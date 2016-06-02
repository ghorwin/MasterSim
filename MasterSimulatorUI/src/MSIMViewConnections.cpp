#include "MSIMViewConnections.h"
#include "ui_MSIMViewConnections.h"

#include "MSIMUIConstants.h"
#include "MSIMProjectHandler.h"

#include <IBK_Exception.h>

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

	formatTable(m_ui->tableWidgetConnections);
	formatTable(m_ui->tableWidgetSlaves);
}


MSIMViewConnections::~MSIMViewConnections() {
	delete m_ui;
}


void MSIMViewConnections::onModified( int modificationType, void * data ) {
	switch ((MSIMProjectHandler::ModificationTypes)modificationType) {
		case MSIMProjectHandler::AllModified :
			break;
		default:
			return; // nothing to do for us
	}

	// TODO : store current check states before clearing table

	// update tables based on project file content
	m_ui->tableWidgetConnections->clearContents();
	m_ui->tableWidgetSlaves->clearContents();

	m_ui->tableWidgetSlaves->setRowCount(project().m_simulators.size());
	for (unsigned int i=0; i<project().m_simulators.size(); ++i) {
		const MASTER_SIM::Project::SimulatorDef & simDef = project().m_simulators[i];
		QTableWidgetItem * item = new QTableWidgetItem( QString::fromUtf8(simDef.m_name.c_str()) );
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
		item->setData(Qt::UserRole, QColor(simDef.m_color.toQRgb()));
		m_ui->tableWidgetSlaves->setItem(i, 0, item);
	}
	m_ui->tableWidgetConnections->setRowCount(project().m_graph.size() );
	for (unsigned int i=0; i<project().m_graph.size(); ++i) {
		const MASTER_SIM::Project::GraphEdge & edge = project().m_graph[i];
		QTableWidgetItem * item = new QTableWidgetItem( QString::fromStdString(edge.m_outputVariableRef)); // no utf8 here
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		try {
			std::string slaveName = edge.outputSlaveName();
			// find slave in list of slaves
			const MASTER_SIM::Project::SimulatorDef & simDef = project().simulatorDefinition(slaveName);
			item->setData(Qt::UserRole, QColor(simDef.m_color.toQRgb()));
		}
		catch (IBK::Exception & ex) {
			ex.writeMsgStackToError();
			item->setToolTip( QString::fromStdString(ex.what())); /// \todo translation of error message to user?
			item->setBackground( QColor("#B22222") );
			item->setData(Qt::UserRole+1, true);
		}
		m_ui->tableWidgetConnections->setItem(i, 0, item);

		item = new QTableWidgetItem( QString::fromStdString(edge.m_inputVariableRef)); // no utf8 here
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		try {
			std::string slaveName = edge.inputSlaveName();
			// find slave in list of slaves
			const MASTER_SIM::Project::SimulatorDef & simDef = project().simulatorDefinition(slaveName);
			item->setData(Qt::UserRole, QColor(simDef.m_color.toQRgb()));
		}
		catch (IBK::Exception & ex) {
			ex.writeMsgStackToError();
			item->setToolTip( QString::fromStdString(ex.what())); /// \todo translation of error message to user?
			item->setBackground( QColor("#B22222") );
			item->setData(Qt::UserRole+1, true);
		}
		m_ui->tableWidgetConnections->setItem(i, 1, item);
	}
}


void MSIMViewConnections::on_pushButton_clicked() {
	// check that slave names are not the same

	// find all matches
	// create graph edges
	// create undo action for graph editing
}
