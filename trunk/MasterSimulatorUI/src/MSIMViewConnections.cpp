#include "MSIMViewConnections.h"
#include "ui_MSIMViewConnections.h"

#include <QSortFilterProxyModel>
#include <QMessageBox>

#include <IBK_Exception.h>

#include "MSIMUIConstants.h"
#include "MSIMProjectHandler.h"
#include "MSIMMainWindow.h"

#include "MSIMSlaveItemDelegate.h"
#include "MSIMConnectionItemDelegate.h"
#include "MSIMConversion.h"
#include "MSIMUndoConnections.h"

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

	// setup tables
	m_ui->tableWidgetOutputVariable->verticalHeader()->setVisible(false);
	m_ui->tableWidgetOutputVariable->setColumnCount(3);
	headers.clear();
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

	formatTable(m_ui->tableWidgetConnections);
	formatTable(m_ui->tableWidgetSlaves);
	formatTable(m_ui->tableWidgetOutputVariable);
	formatTable(m_ui->tableWidgetInputVariable);
}


MSIMViewConnections::~MSIMViewConnections() {
	delete m_ui;
}


void MSIMViewConnections::onModified( int modificationType, void * /* data */) {
	switch ((MSIMProjectHandler::ModificationTypes)modificationType) {
		case MSIMProjectHandler::AllModified :
		case MSIMProjectHandler::SlavesModified : // slaves may have been renamed
		case MSIMProjectHandler::ConnectionsModified :
			break;
		default:
			return; // nothing to do for us
	}

	blockMySignals(this, true);


	std::set<std::string> checkedSlaveNames;
	if (m_ui->tableWidgetSlaves->rowCount() == 0) {
		// table is empty - on first fill, use all names
		for (unsigned int i=0; i<project().m_simulators.size(); ++i) {
			const MASTER_SIM::Project::SimulatorDef & simDef = project().m_simulators[i];
			checkedSlaveNames.insert(simDef.m_name);
		}
	}
	else {
		for (int i=0; i<m_ui->tableWidgetSlaves->rowCount(); ++i) {
			if (m_ui->tableWidgetSlaves->item(i,0)->checkState() == Qt::Checked)
				checkedSlaveNames.insert(m_ui->tableWidgetSlaves->item(i,0)->text().toUtf8().data());
		}
	}

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
		if (checkedSlaveNames.find(slaveName.toUtf8().data()) != checkedSlaveNames.end())
			item->setCheckState(Qt::Checked);
		else
			item->setCheckState(Qt::Unchecked);
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
	updateInputOutputVariablesTables();
}


void MSIMViewConnections::on_tableWidgetSlaves_cellChanged(int /* row */, int /* column */) {
	updateConnectionsTable();
}


void MSIMViewConnections::updateConnectionsTable() {
	blockMySignals(this, true);

	// store current sort column
	int sortColumn = m_ui->tableWidgetConnections->horizontalHeader()->sortIndicatorSection();

	std::set<std::string> checkedSlaveNames;
	for (int i=0; i<m_ui->tableWidgetSlaves->rowCount(); ++i) {
		if (m_ui->tableWidgetSlaves->item(i,0)->checkState() == Qt::Checked)
			checkedSlaveNames.insert(m_ui->tableWidgetSlaves->item(i,0)->text().toUtf8().data());
	}

	int currentIdx = m_ui->tableWidgetConnections->currentRow();

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
			outItem->setData(Qt::UserRole, i); // also store index in global connection list
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
	m_ui->tableWidgetConnections->sortByColumn(sortColumn);

	if (m_ui->tableWidgetConnections->rowCount() > 0) {
		if (currentIdx == -1)
			currentIdx = 0;
		currentIdx = qMin<int>(currentIdx, m_ui->tableWidgetConnections->rowCount()-1);
		m_ui->tableWidgetConnections->selectRow(currentIdx);
	}
	m_ui->toolButtonRemoveConnection->setEnabled(m_ui->tableWidgetConnections->currentRow() != -1);

	blockMySignals(this, false);
}


void MSIMViewConnections::updateInputOutputVariablesTables() {
	// remember select rows
	int outputVarRow = m_ui->tableWidgetOutputVariable->currentRow();
	int inputVarRow = m_ui->tableWidgetInputVariable->currentRow();

	int outputSortColumn = m_ui->tableWidgetOutputVariable->horizontalHeader()->sortIndicatorSection();
	int inputSortColumn = m_ui->tableWidgetInputVariable->horizontalHeader()->sortIndicatorSection();

	m_ui->tableWidgetOutputVariable->setSortingEnabled(false);
	m_ui->tableWidgetInputVariable->setSortingEnabled(false);

	// first clear tables
	m_ui->tableWidgetOutputVariable->clearContents();
	m_ui->tableWidgetInputVariable->clearContents();
	m_ui->tableWidgetOutputVariable->setRowCount(0);
	m_ui->tableWidgetInputVariable->setRowCount(0);

	// process all simulators in project
	for (unsigned int i=0; i<project().m_simulators.size(); ++i) {
		const MASTER_SIM::Project::SimulatorDef & simDef = project().m_simulators[i];

		// find associated ModelDescription
		try {
			const MASTER_SIM::ModelDescription & modelDesc = MSIMMainWindow::instance().modelDescription(simDef.m_name);

			// loop over all variables
			for (unsigned int v=0; v<modelDesc.m_variables.size(); ++v) {
				const MASTER_SIM::FMIVariable & var = modelDesc.m_variables[v];
				QTableWidget * table;
				if (var.m_causality == MASTER_SIM::FMIVariable::C_OUTPUT) {
					table = m_ui->tableWidgetOutputVariable;
				}
				else if (var.m_causality == MASTER_SIM::FMIVariable::C_INPUT) {
					table = m_ui->tableWidgetInputVariable;

					// check if this input variable is already connected
					std::string variableRef = simDef.m_name + "." + var.m_name;
					unsigned int e=0;
					for (; e<project().m_graph.size(); ++e) {
						const MASTER_SIM::Project::GraphEdge & edge = project().m_graph[e];
						if (edge.m_inputVariableRef == variableRef)
							break;
					}
					if (e != project().m_graph.size())
						continue; // skip this input variable
				}
				else continue;


				int currentRow = table->rowCount();
				table->setRowCount(currentRow+1);
				QTableWidgetItem * item = new QTableWidgetItem( QString::fromUtf8(simDef.m_name.c_str()));
				item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
				item->setTextColor( QRgb(simDef.m_color.toQRgb()));
				table->setItem(currentRow, 0, item);
				item = new QTableWidgetItem( QString::fromUtf8(var.m_name.c_str()));
				item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
				item->setTextColor( QRgb(simDef.m_color.toQRgb()));
				table->setItem(currentRow, 1, item);
				item = new QTableWidgetItem( QString::fromLatin1( MASTER_SIM::FMIVariable::varType2String(var.m_type) ));
				item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
				item->setTextColor( QRgb(simDef.m_color.toQRgb()));
				table->setItem(currentRow, 2, item);
			}
		}
		catch (IBK::Exception &) {
			// FMU not yet loaded?
		}
	}

	m_ui->tableWidgetOutputVariable->setSortingEnabled(true);
	m_ui->tableWidgetOutputVariable->sortByColumn(outputSortColumn);
	m_ui->tableWidgetInputVariable->setSortingEnabled(true);
	m_ui->tableWidgetInputVariable->sortByColumn(inputSortColumn);

}


void MSIMViewConnections::on_toolButtonAddConnection_clicked() {
	// get selected output variable and input variable
	// create new graph edge and add it to project
}


void MSIMViewConnections::on_toolButtonRemoveConnection_clicked() {

	MASTER_SIM::Project p = project();

	// find currently selected slave definition
	int currentIdx = m_ui->tableWidgetConnections->currentRow();
	Q_ASSERT(currentIdx != -1);

	QTableWidgetItem * outItem = m_ui->tableWidgetConnections->item(currentIdx, 0);
	unsigned int connIdx = outItem->data(Qt::UserRole).toUInt();

	// remove slave instance
	p.m_graph.erase( p.m_graph.begin() + connIdx);

	// create undo action
	MSIMUndoConnections * cmd = new MSIMUndoConnections(tr("Connection removed"), p);
	cmd->push();
}


void MSIMViewConnections::on_pushButtonConnectByVariableName_clicked() {

	std::vector<MASTER_SIM::Project::GraphEdge> newEdges;

	try {
		// get slave names and simulator definitions
		std::string slave1Name = m_ui->comboBoxSlave1->currentText().toUtf8().data();
		std::string slave2Name = m_ui->comboBoxSlave2->currentText().toUtf8().data();
//		const MASTER_SIM::Project::SimulatorDef & slave1Def = project().simulatorDefinition( slave1Name );
//		const MASTER_SIM::Project::SimulatorDef & slave2Def = project().simulatorDefinition( slave2Name );

		// get model descriptions for selected slaves
		const MASTER_SIM::ModelDescription & modelDesc1 = MSIMMainWindow::instance().modelDescription(slave1Name);
		const MASTER_SIM::ModelDescription & modelDesc2 = MSIMMainWindow::instance().modelDescription(slave2Name);

		// slave 1 -> slave 2
		std::map<std::string, std::string> outputVars; // map with key = last part of variable name, value = full variable name
		std::map<std::string, std::string> inputVars; // map with key = last part of variable name, value = full variable name

		for (unsigned int i=0; i<modelDesc1.m_variables.size(); ++i) {
			const MASTER_SIM::FMIVariable & var = modelDesc1.m_variables[i];
			if (var.m_causality == MASTER_SIM::FMIVariable::C_OUTPUT) {
				// extract last token
				std::vector<std::string> tokens;
				IBK::explode(var.m_name, tokens, ".", IBK::EF_NoFlags);
				outputVars[tokens.back()] = slave1Name + "." + var.m_name;
			}
		}
		for (unsigned int i=0; i<modelDesc2.m_variables.size(); ++i) {
			const MASTER_SIM::FMIVariable & var = modelDesc2.m_variables[i];
			if (var.m_causality == MASTER_SIM::FMIVariable::C_INPUT) {
				// extract last token
				std::vector<std::string> tokens;
				IBK::explode(var.m_name, tokens, ".", IBK::EF_NoFlags);
				inputVars[tokens.back()] = slave2Name + "." + var.m_name;
			}
		}

		// now process all input variables and check if there are matching outputs
		for (std::map<std::string, std::string>::const_iterator it = inputVars.begin(); it != inputVars.end(); ++it) {
			std::map<std::string, std::string>::const_iterator out_it = outputVars.find(it->first);
			if (out_it != outputVars.end()) {
				// create graph edges
				MASTER_SIM::Project::GraphEdge edge;
				edge.m_inputVariableRef = it->second;
				edge.m_outputVariableRef = out_it->second;
				newEdges.push_back(edge);
			}
		}

		outputVars.clear();
		inputVars.clear();
		for (unsigned int i=0; i<modelDesc2.m_variables.size(); ++i) {
			const MASTER_SIM::FMIVariable & var = modelDesc2.m_variables[i];
			if (var.m_causality == MASTER_SIM::FMIVariable::C_OUTPUT) {
				// extract last token
				std::vector<std::string> tokens;
				IBK::explode(var.m_name, tokens, ".", IBK::EF_NoFlags);
				outputVars[tokens.back()] = slave1Name + "." + var.m_name;
			}
		}
		for (unsigned int i=0; i<modelDesc1.m_variables.size(); ++i) {
			const MASTER_SIM::FMIVariable & var = modelDesc1.m_variables[i];
			if (var.m_causality == MASTER_SIM::FMIVariable::C_INPUT) {
				// extract last token
				std::vector<std::string> tokens;
				IBK::explode(var.m_name, tokens, ".", IBK::EF_NoFlags);
				inputVars[tokens.back()] = slave2Name + "." + var.m_name;
			}
		}

		// now process all input variables and check if there are matching outputs
		for (std::map<std::string, std::string>::const_iterator it = inputVars.begin(); it != inputVars.end(); ++it) {
			std::map<std::string, std::string>::const_iterator out_it = outputVars.find(it->first);
			if (out_it != outputVars.end()) {
				// create graph edges
				MASTER_SIM::Project::GraphEdge edge;
				edge.m_inputVariableRef = it->second;
				edge.m_outputVariableRef = out_it->second;
				newEdges.push_back(edge);
			}
		}

	}
	catch (IBK::Exception & /* ex */) {
		QMessageBox::critical(this, tr("Error connecting slaves"), tr("Slaves could not be connected. Try analyzing FMUs first!"));
		return;
	}

	if (newEdges.empty()) {
		QMessageBox::information(this, tr("Connection result"), tr("No connections possible between slaves."));
		return;
	}


	MASTER_SIM::Project p = project();

	unsigned int newConnections = 0;
	// add connections to graph that are not yet existing
	for (unsigned int i=0; i<newEdges.size(); ++i) {
		if (std::find(p.m_graph.begin(), p.m_graph.end(), newEdges[i])==p.m_graph.end()) {
			++newConnections;
			p.m_graph.push_back(newEdges[i]);
		}
	}

	if (newConnections == 0) {
		QMessageBox::information(this, tr("Connection result"), tr("No new connections could be made between slaves."));
		return;
	}

	// create undo action
	MSIMUndoConnections * cmd = new MSIMUndoConnections(tr("Connections added"), p);
	cmd->push();

//	QMessageBox::information(this, tr("Connection result"), tr("%1 new connections are made between slaves.").arg(newConnections));
}




void MSIMViewConnections::on_tableWidgetOutputVariable_itemDoubleClicked(QTableWidgetItem *) {
	on_toolButtonAddConnection_clicked();
}


void MSIMViewConnections::on_tableWidgetInputVariable_itemDoubleClicked(QTableWidgetItem *) {
	on_toolButtonAddConnection_clicked();
}
