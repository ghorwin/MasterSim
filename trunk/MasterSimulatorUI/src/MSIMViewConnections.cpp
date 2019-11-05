#include "MSIMViewConnections.h"
#include "ui_MSIMViewConnections.h"

#include <QSortFilterProxyModel>
#include <QMessageBox>

#include <IBK_Exception.h>

#include <BM_SceneManager.h>

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

	connect(&MSIMProjectHandler::instance(), SIGNAL(modified(unsigned int,void*)),
			this, SLOT(onModified(unsigned int,void*)));

	// setup tables
	m_ui->tableWidgetConnections->verticalHeader()->setVisible(false);
	m_ui->tableWidgetConnections->setColumnCount(2);
	QStringList headers;
	headers << tr("Output variable") << tr("Input variable");
	m_ui->tableWidgetConnections->setHorizontalHeaderLabels(headers);
	m_ui->tableWidgetConnections->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	m_ui->tableWidgetConnections->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

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
	m_ui->tableWidgetOutputVariable->setColumnCount(4);
	headers.clear();
	headers << tr("Slave") << tr("Variable") << tr("Type") << tr("Unit");
	m_ui->tableWidgetOutputVariable->setHorizontalHeaderLabels(headers);
	m_ui->tableWidgetOutputVariable->setSortingEnabled(true);
	m_ui->tableWidgetOutputVariable->horizontalHeader()->setSortIndicatorShown(true);
	m_ui->tableWidgetOutputVariable->sortByColumn(0, Qt::AscendingOrder);

	m_ui->tableWidgetOutputVariable->setItemDelegate(new MSIMConnectionItemDelegate(this));
	m_ui->tableWidgetInputVariable->setItemDelegate(new MSIMConnectionItemDelegate(this));

	m_ui->tableWidgetInputVariable->verticalHeader()->setVisible(false);
	m_ui->tableWidgetInputVariable->setColumnCount(4);
	headers.clear();
	headers << tr("Slave") << tr("Variable") << tr("Type") << tr("Unit");
	m_ui->tableWidgetInputVariable->setHorizontalHeaderLabels(headers);
	m_ui->tableWidgetInputVariable->setSortingEnabled(true);
	m_ui->tableWidgetInputVariable->horizontalHeader()->setSortIndicatorShown(true);
	m_ui->tableWidgetInputVariable->sortByColumn(0, Qt::AscendingOrder);

	formatTable(m_ui->tableWidgetConnections);
	formatTable(m_ui->tableWidgetSlaves);
	formatTable(m_ui->tableWidgetOutputVariable);
	formatTable(m_ui->tableWidgetInputVariable);

	resizeTableColumns();
}


MSIMViewConnections::~MSIMViewConnections() {
	delete m_ui;
}


void MSIMViewConnections::onModified(unsigned int modificationType, void * /* data */) {
	switch ((MSIMProjectHandler::ModificationTypes)modificationType) {
		case MSIMProjectHandler::AllModified :
		case MSIMProjectHandler::SlavesModified : // slaves may have been renamed
		case MSIMProjectHandler::ConnectionsModified :
			break;
		default:
			return; // nothing to do for us
	}

	blockMySignals(this, true);

	// Mind: when loading a new project the "unchecked" states of the old project will be
	//       used. So when a slave "xxx" was unchecked in the current project, it will
	//       also be unchecked in the newly opened project, when there is a slave "xxx" as well.
	std::set<std::string> uncheckedSlaveNames;
	for (int i=0; i<m_ui->tableWidgetSlaves->rowCount(); ++i) {
		if (m_ui->tableWidgetSlaves->item(i,0)->checkState() == Qt::Unchecked)
			uncheckedSlaveNames.insert(m_ui->tableWidgetSlaves->item(i,0)->text().toUtf8().data());
	}

	// update tables based on project file content
	m_ui->tableWidgetSlaves->clearContents();

	m_ui->comboBoxSlave1->clear();
	m_ui->comboBoxSlave2->clear();

	m_ui->tableWidgetSlaves->setRowCount(project().m_simulators.size());
	for (unsigned int i=0; i<project().m_simulators.size(); ++i) {
		const MASTER_SIM::Project::SimulatorDef & simDef = project().m_simulators[i];
		QString slaveName = QString::fromStdString(simDef.m_name);
		QTableWidgetItem * item = new QTableWidgetItem( slaveName );
		item->setFlags(Qt::ItemIsEnabled);
		item->setData(Qt::TextColorRole, QColor(simDef.m_color.toQRgb()));
		m_ui->tableWidgetSlaves->setItem((int)i, 0, item);

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
	resizeTableColumns();
}


void MSIMViewConnections::resizeEvent(QResizeEvent *event) {
	QWidget::resizeEvent(event);
	resizeTableColumns();
}


void MSIMViewConnections::showEvent(QShowEvent *event) {
	QWidget::showEvent(event);
	resizeTableColumns();
}


void MSIMViewConnections::on_tableWidgetSlaves_cellChanged(int /* row */, int /* column */) {
	updateConnectionsTable();
}


void MSIMViewConnections::on_toolButtonAddConnection_clicked() {
	// get selected output variable and input variable
	int outputVarCurrentIdx = m_ui->tableWidgetOutputVariable->currentRow();
	int inputVarCurrentIdx = m_ui->tableWidgetInputVariable->currentRow();

	// if input var index is -1, then the table widget is empty and we simply skip this
	if (outputVarCurrentIdx == -1 || inputVarCurrentIdx == -1)
		return;

	// check for matching variable types
	if (m_ui->tableWidgetOutputVariable->item(outputVarCurrentIdx, 2)->text() !=
		m_ui->tableWidgetInputVariable->item(inputVarCurrentIdx, 2)->text())
	{
		QMessageBox::critical(this, tr("Connection error"), tr("Cannot connect variables of different type."));
		return;
	}

	// create new graph edge and add it to project
	MASTER_SIM::Project::GraphEdge edge;
	edge.m_outputVariableRef = m_ui->tableWidgetOutputVariable->item(outputVarCurrentIdx, 0)->text().toUtf8().data();
	edge.m_outputVariableRef += ".";
	edge.m_outputVariableRef += m_ui->tableWidgetOutputVariable->item(outputVarCurrentIdx, 1)->text().toUtf8().data();

	edge.m_inputVariableRef = m_ui->tableWidgetInputVariable->item(inputVarCurrentIdx, 0)->text().toUtf8().data();
	edge.m_inputVariableRef += ".";
	edge.m_inputVariableRef += m_ui->tableWidgetInputVariable->item(inputVarCurrentIdx, 1)->text().toUtf8().data();

	MASTER_SIM::Project p = project();
	p.m_graph.push_back(edge);

	// create undo action
	BLOCKMOD::Network n = MSIMProjectHandler::instance().sceneManager()->network(); // new network
	MSIMUndoConnections * cmd = new MSIMUndoConnections(tr("Connection added"), p, n);
	cmd->push();
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
	BLOCKMOD::Network n = MSIMProjectHandler::instance().sceneManager()->network(); // net network
	/// \todo remove correspondin socket connections
	MSIMUndoConnections * cmd = new MSIMUndoConnections(tr("Connection removed"), p, n);
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
				outputVars[tokens.back()] = slave2Name + "." + var.m_name;
			}
		}
		for (unsigned int i=0; i<modelDesc1.m_variables.size(); ++i) {
			const MASTER_SIM::FMIVariable & var = modelDesc1.m_variables[i];
			if (var.m_causality == MASTER_SIM::FMIVariable::C_INPUT) {
				// extract last token
				std::vector<std::string> tokens;
				IBK::explode(var.m_name, tokens, ".", IBK::EF_NoFlags);
				inputVars[tokens.back()] = slave1Name + "." + var.m_name;
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
	BLOCKMOD::Network n = MSIMProjectHandler::instance().sceneManager()->network(); // net network
	/// \todo auto-create socket connections (several!)
	MSIMUndoConnections * cmd = new MSIMUndoConnections(tr("Connections added"), p, n);
	cmd->push();

//	QMessageBox::information(this, tr("Connection result"), tr("%1 new connections are made between slaves.").arg(newConnections));
}


void MSIMViewConnections::on_tableWidgetOutputVariable_itemDoubleClicked(QTableWidgetItem *) {
	on_toolButtonAddConnection_clicked();
}


void MSIMViewConnections::on_tableWidgetInputVariable_itemDoubleClicked(QTableWidgetItem *) {
	on_toolButtonAddConnection_clicked();
}


// *** private functions ***

void MSIMViewConnections::updateConnectionsTable() {
	blockMySignals(this, true);

	// store current sort column
	int sortColumn = m_ui->tableWidgetConnections->horizontalHeader()->sortIndicatorSection();

	std::vector<MASTER_SIM::Project::GraphCheckErrorCodes> graphCheckResults;
	project().checkGraphs(MSIMMainWindow::instance().modelDescriptions(), graphCheckResults);

	int currentIdx = m_ui->tableWidgetConnections->currentRow();

	m_ui->tableWidgetConnections->clearContents();
	m_ui->tableWidgetConnections->setRowCount(0);
	m_ui->tableWidgetConnections->setSortingEnabled(false);
	for (unsigned int i=0; i<project().m_graph.size(); ++i) {
		const MASTER_SIM::Project::GraphEdge & edge = project().m_graph[i];
		QString itemName = QString::fromStdString(edge.m_outputVariableRef);
		QTableWidgetItem * outItem = new QTableWidgetItem( itemName );
		outItem->setData(Qt::UserRole, i); // also store index in global connection list
		outItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		std::string inputSlaveName;
		std::string outputSlaveName, varName;
		try {
			edge.splitReference(edge.m_outputVariableRef, outputSlaveName, varName);
			// find slave in list of slaves
			const MASTER_SIM::Project::SimulatorDef & simDef = project().simulatorDefinition(outputSlaveName);
			outItem->setData(Qt::TextColorRole, QColor(simDef.m_color.toQRgb()));
		}
		catch (IBK::Exception & ex) {
			ex.writeMsgStackToError();
			outItem->setToolTip(tr("Invalid variable reference or unknown slave."));
			outItem->setBackground( QColor("#B22222") );
			outItem->setData(Qt::UserRole+1, true);
		}
		// optionally add unit information
		try {
			// lookup variable in simulator
			const MASTER_SIM::ModelDescription & modelDesc = MSIMMainWindow::instance().modelDescription(outputSlaveName);
			const MASTER_SIM::FMIVariable & var = modelDesc.variable(varName);
			if (var.m_type == MASTER_SIM::FMIVariable::VT_DOUBLE) {
				QString unit = QString::fromStdString(var.m_unit);
				if (unit.isEmpty())
					unit = "-";
				outItem->setText( tr("%1 [%2]").arg(itemName, unit));
			}
		} catch (...) {
			// in case of missing model descriptions or invalid variable names ... do nothing here!
		}

		itemName = QString::fromStdString(edge.m_inputVariableRef);
		QTableWidgetItem * inItem = new QTableWidgetItem( itemName);
		inItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		try {
			edge.splitReference(edge.m_inputVariableRef, inputSlaveName, varName);
			// find slave in list of slaves
			const MASTER_SIM::Project::SimulatorDef & simDef = project().simulatorDefinition(inputSlaveName);
			inItem->setData(Qt::TextColorRole, QColor(simDef.m_color.toQRgb()));
		}
		catch (IBK::Exception & ex) {
			ex.writeMsgStackToError();
			inItem->setToolTip(tr("Invalid variable reference or unknown slave."));
			inItem->setBackground( QColor("#B22222") );
			inItem->setData(Qt::UserRole+1, true);
		}
		// optionally add unit information
		try {
			// lookup variable in simulator
			const MASTER_SIM::ModelDescription & modelDesc = MSIMMainWindow::instance().modelDescription(inputSlaveName);
			const MASTER_SIM::FMIVariable & var = modelDesc.variable(varName);
			if (var.m_type == MASTER_SIM::FMIVariable::VT_DOUBLE) {
				QString unit = QString::fromStdString(var.m_unit);
				if (unit.isEmpty())
					unit = "-";
				inItem->setText( tr("%1 [%2]").arg(itemName, unit));
			}
		} catch (...) {
			// in case of missing model descriptions or invalid variable names ... do nothing here!
		}

		// check if both input and output slaves are selected
		int currentRow = m_ui->tableWidgetConnections->rowCount();
		m_ui->tableWidgetConnections->setRowCount(currentRow+1);

		// mark items based on check results
		QFont f;
		f.setItalic(true);
		switch (graphCheckResults[i]) {
			case MASTER_SIM::Project::GEC_NoError : break; // nothing to adjust
			case MASTER_SIM::Project::GEC_Undetermined :
				outItem->setFont(f);
				inItem->setFont(f);
				outItem->setToolTip(tr("Connection cannot be analyzed without fmus of referenced slaves."));
				inItem->setToolTip(tr("Connection cannot be analyzed without fmus of referenced slaves."));
			break;
			case MASTER_SIM::Project::GEC_TargetSocketNotInlet :
				inItem->setFont(f);
				inItem->setData(Qt::TextColorRole, QColor(Qt::gray));
				inItem->setToolTip(tr("Not an inlet socket!"));
				outItem->setToolTip(tr("Not an inlet socket!"));
			break;
			case MASTER_SIM::Project::GEC_SourceSocketNotOutlet :
				outItem->setFont(f);
				outItem->setData(Qt::TextColorRole, QColor(Qt::gray));
				outItem->setToolTip(tr("Not an outlet socket!"));
				inItem->setToolTip(tr("Not an outlet socket!"));
			break;
			case MASTER_SIM::Project::GEC_TargetSocketAlreadyConnected :
				outItem->setFont(f);
				inItem->setFont(f);
				outItem->setData(Qt::TextColorRole, QColor(Qt::gray));
				inItem->setData(Qt::TextColorRole, QColor(Qt::gray));
				inItem->setToolTip(tr("Inlet socket is connected twice."));
				outItem->setToolTip(tr("Inlet socket is connected twice."));
			break;
			default : {
				outItem->setFont(f);
				inItem->setFont(f);
				outItem->setData(Qt::TextColorRole, QColor(Qt::gray));
				inItem->setData(Qt::TextColorRole, QColor(Qt::gray));
				outItem->setToolTip(tr("Invalid connection"));
				inItem->setToolTip(tr("Invalid connection"));
			}
		}

		m_ui->tableWidgetConnections->setItem(currentRow, 0, outItem);
		m_ui->tableWidgetConnections->setItem(currentRow, 1, inItem);
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
	int outputVarCurrentIdx = m_ui->tableWidgetOutputVariable->currentRow();
	int inputVarCurrentIdx = m_ui->tableWidgetInputVariable->currentRow();

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
				QTableWidgetItem * item = new QTableWidgetItem( QString::fromStdString(simDef.m_name));
				item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
				item->setTextColor( QRgb(simDef.m_color.toQRgb()));
				table->setItem(currentRow, 0, item);

				item = new QTableWidgetItem( QString::fromStdString(var.m_name));
				item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
				item->setTextColor( QRgb(simDef.m_color.toQRgb()));
				table->setItem(currentRow, 1, item);

				item = new QTableWidgetItem( QString( MASTER_SIM::FMIVariable::varType2String(var.m_type) ));
				item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
				item->setTextColor( QRgb(simDef.m_color.toQRgb()));
				table->setItem(currentRow, 2, item);

				if (var.m_type == MASTER_SIM::FMIVariable::VT_DOUBLE) {
					QString unit = QString::fromStdString(var.m_unit);
					if (unit.isEmpty())
						unit = "-";
					item = new QTableWidgetItem(unit);
				}
				else
					item = new QTableWidgetItem("");
				item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
				item->setTextColor( QRgb(simDef.m_color.toQRgb()));
				table->setItem(currentRow, 3, item);
			}
		}
		catch (IBK::Exception &) {
			// FMU not yet loaded?
		}
	}

	if (m_ui->tableWidgetInputVariable->rowCount() > 0) {
		if (inputVarCurrentIdx == -1)
			inputVarCurrentIdx = 0;
		inputVarCurrentIdx = qMin<int>(inputVarCurrentIdx, m_ui->tableWidgetInputVariable->rowCount()-1);
		m_ui->tableWidgetInputVariable->selectRow(inputVarCurrentIdx);
	}
	if (m_ui->tableWidgetOutputVariable->rowCount() > 0) {
		if (outputVarCurrentIdx == -1)
			outputVarCurrentIdx = 0;
		outputVarCurrentIdx = qMin<int>(outputVarCurrentIdx, m_ui->tableWidgetOutputVariable->rowCount()-1);
		m_ui->tableWidgetOutputVariable->selectRow(outputVarCurrentIdx);
	}

	m_ui->toolButtonAddConnection->setEnabled( (m_ui->tableWidgetInputVariable->currentRow() != -1) &&
											   (m_ui->tableWidgetOutputVariable->currentRow() != -1));

	m_ui->tableWidgetOutputVariable->setSortingEnabled(true);
	m_ui->tableWidgetOutputVariable->sortByColumn(outputSortColumn);

	m_ui->tableWidgetInputVariable->setSortingEnabled(true);
	m_ui->tableWidgetInputVariable->sortByColumn(inputSortColumn);
}


void MSIMViewConnections::resizeTableColumns() {
	// the tables are sized such that the variable name column is adjusted, all other columns are
	// set to their default sizes
	// in case that the slave name very long and there is less 60 pix space for the variable column,
	// we subtract a little from the first column
	m_ui->tableWidgetOutputVariable->resizeColumnToContents(0);
	m_ui->tableWidgetOutputVariable->resizeColumnToContents(2);
	m_ui->tableWidgetOutputVariable->resizeColumnToContents(3);

	int w = m_ui->tableWidgetOutputVariable->contentsRect().width();
	w -= m_ui->tableWidgetOutputVariable->columnWidth(2) + m_ui->tableWidgetOutputVariable->columnWidth(3);
	int w2 = w - m_ui->tableWidgetOutputVariable->columnWidth(0); // size of second column
	if (w2 < 60) { // too small? space is evenly distributed between first two columns
		w2 = w/2;
		w = w/2;
	}
	else {
		w -= w2; // w now is width of first column
	}
	// remaining space is to be distributed by slave and variable columns
	m_ui->tableWidgetOutputVariable->horizontalHeader()->resizeSection(0, w);
	m_ui->tableWidgetOutputVariable->horizontalHeader()->resizeSection(1, w2);

	m_ui->tableWidgetInputVariable->resizeColumnToContents(0);
	m_ui->tableWidgetInputVariable->resizeColumnToContents(2);
	m_ui->tableWidgetInputVariable->resizeColumnToContents(3);
	w = m_ui->tableWidgetInputVariable->contentsRect().width();
	w -= m_ui->tableWidgetInputVariable->columnWidth(2) + m_ui->tableWidgetInputVariable->columnWidth(3);

	w2 = w - m_ui->tableWidgetInputVariable->columnWidth(0); // size of second column
	if (w2 < 60) { // too small? space is evenly distributed between first two columns
		w2 = w/2;
		w = w/2;
	}
	else {
		w -= w2; // w now is width of first column
	}
	m_ui->tableWidgetInputVariable->horizontalHeader()->resizeSection(0, w);
	m_ui->tableWidgetInputVariable->horizontalHeader()->resizeSection(1, w2);

}
