#include "MSIMViewSlaves.h"
#include "ui_MSIMViewSlaves.h"

#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QFileDialog>
#include <QSettings>
#include <QFileInfo>
#include <QScrollBar>
#include <QDebug>
#include <QMessageBox>

#include <qttreepropertybrowser.h>
#include <qtvariantproperty.h>
#include <QPW_VariantPropertyManager.h>

#include <IBK_algorithm.h>

#include <BM_Network.h>
#include <BM_SceneManager.h>
#include <BM_Globals.h>

#include "MSIMProjectHandler.h"
#include "MSIMUIConstants.h"
#include "MSIMSlaveItemDelegate.h"
#include "MSIMConversion.h"
#include "MSIMUndoSlaves.h"
#include "MSIMMainWindow.h"

#include <MSIM_Project.h>

MSIMViewSlaves::MSIMViewSlaves(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::MSIMViewSlaves)
{
	m_ui->setupUi(this);
	m_ui->verticalLayout_4->setContentsMargins(9,0,9,9);

	connect(&MSIMProjectHandler::instance(), SIGNAL(modified(int,void*)), this, SLOT(onModified(int,void*)));

	// setup tables
//	m_ui->tableWidgetFMUs->horizontalHeader()->setVisible(false);
//	m_ui->tableWidgetFMUs->verticalHeader()->setVisible(false);
//	m_ui->tableWidgetFMUs->setColumnCount(1);
	QStringList headers;
//	headers << tr("Full path");
//	m_ui->tableWidgetFMUs->setHorizontalHeaderLabels(headers);
//	m_ui->tableWidgetFMUs->horizontalHeader()->setStretchLastSection(true);

//	m_ui->tableWidgetSlaves->horizontalHeader()->setVisible(false);
	m_ui->tableWidgetSlaves->verticalHeader()->setVisible(false);
	m_ui->tableWidgetSlaves->setColumnCount(4);
	headers.clear();
	headers << "" << tr("Name") << tr("FMU") << tr("Cycle Nr.");
	m_ui->tableWidgetSlaves->setHorizontalHeaderLabels(headers);

//	formatTable(m_ui->tableWidgetFMUs);
	formatTable(m_ui->tableWidgetSlaves);

	m_ui->tableWidgetSlaves->horizontalHeader()->resizeSection(0,m_ui->tableWidgetSlaves->verticalHeader()->defaultSectionSize());
	m_ui->tableWidgetSlaves->setItemDelegate(new MSIMSlaveItemDelegate(this));

	m_ui->groupBox->layout()->setContentsMargins(0,0,0,0);
	m_ui->scrollAreaWidgetContents->layout()->setContentsMargins(0,0,0,0);
	m_ui->widgetProperties->updateProperties(-1);

	// set the scene showing the network
	m_ui->blockModWidget->setResolution(1);
	m_ui->blockModWidget->setGridStep(BLOCKMOD::Globals::GridSpacing*10);
}


MSIMViewSlaves::~MSIMViewSlaves() {
	delete m_ui;
}


void MSIMViewSlaves::onModified( int modificationType, void * /* data */ ) {
	switch ((MSIMProjectHandler::ModificationTypes)modificationType) {
		case MSIMProjectHandler::AllModified :
		case MSIMProjectHandler::SlavesModified :
			// sync network with graphical display
			m_ui->blockModWidget->setScene(MSIMProjectHandler::instance().sceneManager());
			break;

		case MSIMProjectHandler::SlaveParameterModified :
			m_ui->widgetProperties->updateProperties( m_ui->tableWidgetSlaves->currentRow() );
			return;

		case MSIMProjectHandler::ProjectPathModified :
			// only need to update if we show relative FMU paths
			if (m_ui->checkBoxRelativeFMUPaths->isChecked())
				break;
		default:
			return; // nothing to do for us
	}

	updateSlaveTable();
}


void MSIMViewSlaves::on_toolButtonAddSlave_clicked() {
	// open file dialog and let user select FMU file
	QSettings settings(ORG_NAME, MASTER_SIM::PROGRAM_NAME);
	QString fmuSearchPath = settings.value("FMUSearchPath", QString()).toString();
	if (fmuSearchPath.isEmpty()) {
		fmuSearchPath = QFileInfo(MSIMProjectHandler::instance().projectFile()).dir().absolutePath();
	}

	QString fname = QFileDialog::getOpenFileName(this, tr("Select FMU"), fmuSearchPath,
												 tr("Slave files (*.fmu *.tsv *.csv);;FMUs (*.fmu)"));
	if (fname.isEmpty())
		return; // Dialog was cancelled

	QFileInfo finfo(fname);
	fmuSearchPath = finfo.dir().absolutePath();
	settings.setValue("FMUSearchPath", fmuSearchPath);

	MASTER_SIM::Project p = project();
	BLOCKMOD::Network n = MSIMProjectHandler::instance().sceneManager()->network();

	// create simulator definition
	MASTER_SIM::Project::SimulatorDef simDef;
	simDef.m_name = finfo.baseName().toUtf8().data(); // disambiguity?
	simDef.m_name = IBK::pick_name(simDef.m_name, p.m_simulators.begin(), p.m_simulators.end());
	simDef.m_pathToFMU = fname.toUtf8().data();
	p.m_simulators.push_back(simDef);

	// create a block for the graphical representation
	unsigned int gx = BLOCKMOD::Globals::GridSpacing*qrand()*int(40.0/RAND_MAX);
	unsigned int gy = BLOCKMOD::Globals::GridSpacing*qrand()*int(40.0/RAND_MAX);
	BLOCKMOD::Block b(QString::fromStdString(simDef.m_name), gx, gy);
	b.m_size = QSize(BLOCKMOD::Globals::GridSpacing*8, BLOCKMOD::Globals::GridSpacing*12);
	n.m_blocks.append(b);

	// create undo action
	MSIMUndoSlaves * cmd = new MSIMUndoSlaves(tr("Slave added"), p, n);
	cmd->push();
}


void MSIMViewSlaves::on_toolButtonRemoveSlave_clicked() {

	MASTER_SIM::Project p = project();
	BLOCKMOD::Network n = MSIMProjectHandler::instance().sceneManager()->network();

	// find currently selected slave definition
	int currentIdx = m_ui->tableWidgetSlaves->currentRow();
	Q_ASSERT(currentIdx != -1);

	// create copy of all connections not made to this slave
	std::string slaveName = p.m_simulators[currentIdx].m_name;

	std::vector<MASTER_SIM::Project::GraphEdge>	graph;
	for (unsigned int i=0; i<p.m_graph.size(); ++i) {
		MASTER_SIM::Project::GraphEdge & edge = p.m_graph[i];
		if (edge.inputSlaveName() != slaveName && edge.outputSlaveName() != slaveName)
			graph.push_back(edge);
	}
	// modify connections
	p.m_graph = graph;

	// remove slave instance
	p.m_simulators.erase( p.m_simulators.begin() + currentIdx);
	n.removeBlock(currentIdx);

	// create undo action
	MSIMUndoSlaves * cmd = new MSIMUndoSlaves(tr("Slave removed"), p, n);
	cmd->push();
}


void MSIMViewSlaves::on_tableWidgetSlaves_cellChanged(int row, int column) {
	// triggered when editor finishes, update simulator definition in selected row

	MASTER_SIM::Project p = project();
	BLOCKMOD::Network n = MSIMProjectHandler::instance().sceneManager()->network();

	QTableWidgetItem * item = m_ui->tableWidgetSlaves->item(row, column);
	switch (column) {
		case 0 : p.m_simulators[row].m_color = IBK::Color::fromQRgb( item->data(Qt::UserRole).value<QColor>().rgba()); break;
		case 1 : {
			std::string newSlaveName = m_ui->tableWidgetSlaves->item(row, column)->text().trimmed().toUtf8().data();
			if (newSlaveName != p.m_simulators[row].m_name) {
				if (newSlaveName.find_first_of(" \t") != std::string::npos) {
					QMessageBox::critical(this, tr("Invalid input"), tr("Slave names may not contain spaces or tabulator characters."));
					item->setText( QString::fromUtf8(p.m_simulators[row].m_name.c_str()) );
					return;
				}
				// check for ambiguous name
				bool found = false;
				for (unsigned int i=0; i<p.m_simulators.size(); ++i) {
					if (i == (unsigned int)row) continue;
					if (newSlaveName == p.m_simulators[row].m_name)
						found = true;
				}
				if (found) {
					QMessageBox::critical(this, tr("Invalid input"), tr("Slave names must be unique identifiers."));
					item->setText( QString::fromUtf8(p.m_simulators[row].m_name.c_str()) );
					return;
				}
			}
			std::string oldName = p.m_simulators[row].m_name;
			p.m_simulators[row].m_name = newSlaveName;

			// now rename all connections with this slave name
			for (unsigned int i=0; i<p.m_graph.size(); ++i) {
				MASTER_SIM::Project::GraphEdge & edge = p.m_graph[i];
				if (edge.inputSlaveName() == oldName) {
					edge.m_inputVariableRef = MASTER_SIM::Project::GraphEdge::replaceSlaveName(edge.m_inputVariableRef, newSlaveName);
				}
				if (edge.outputSlaveName() == oldName) {
					edge.m_outputVariableRef = MASTER_SIM::Project::GraphEdge::replaceSlaveName(edge.m_outputVariableRef, newSlaveName);
				}
			}

			// and also update the connections in the network
			n.renameBlock(row, QString::fromStdString(newSlaveName));

		} break;
		case 2 : {
			IBK::Path path( item->text().toUtf8().data() );
			if (!path.isAbsolute()) {
				IBK::Path projectFilePath = IBK::Path(MSIMProjectHandler::instance().projectFile().toUtf8().data());
				path = projectFilePath.parentPath() / path;
				path.removeRelativeParts();
			}
			p.m_simulators[row].m_pathToFMU = path;
		} break;
		case 3 : p.m_simulators[row].m_cycle = item->text().toUInt(); break;
	}

	// create undo action
	MSIMUndoSlaves * cmd = new MSIMUndoSlaves(tr("Slave modified"), p, n);
	cmd->push();
}


void MSIMViewSlaves::on_checkBoxRelativeFMUPaths_toggled(bool /*checked*/) {
	updateSlaveTable();
}


void MSIMViewSlaves::updateSlaveTable() {
	blockMySignals(this, true);

	int currentSlaveIdx = m_ui->tableWidgetSlaves->currentRow();

	// update tables based on project file content
//	m_ui->tableWidgetFMUs->clear();
	m_ui->tableWidgetSlaves->clearContents();
	QSet<QString> fmuPaths;

	m_ui->tableWidgetSlaves->setRowCount(project().m_simulators.size());
	for (unsigned int i=0; i<project().m_simulators.size(); ++i) {
		const MASTER_SIM::Project::SimulatorDef & simDef = project().m_simulators[i];
		QTableWidgetItem * item = new QTableWidgetItem();
		item->setData(Qt::UserRole, QColor(simDef.m_color.toQRgb()));
		m_ui->tableWidgetSlaves->setItem(i, 0, item);

		item = new QTableWidgetItem( QString::fromUtf8(simDef.m_name.c_str()) );
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
		m_ui->tableWidgetSlaves->setItem(i, 1, item);

		IBK::Path fmuFilePath = simDef.m_pathToFMU;
		if (m_ui->checkBoxRelativeFMUPaths->isChecked()) {
			IBK::Path projectFilePath = IBK::Path(MSIMProjectHandler::instance().projectFile().toUtf8().data());
			try {
				// now compose relative path
				projectFilePath = projectFilePath.parentPath(); // may throw an exception if file path is invalid
				fmuFilePath = fmuFilePath.relativePath(projectFilePath); // may throw an exception if relative path cannot be computed
			}
			catch (IBK::Exception &) {
				fmuFilePath = simDef.m_pathToFMU;
			}
		}
		QString fmuPath = QString::fromUtf8(fmuFilePath.c_str());
		fmuPaths.insert(QString::fromUtf8(simDef.m_pathToFMU.c_str()));
		item = new QTableWidgetItem( fmuPath );
		m_ui->tableWidgetSlaves->setItem(i, 2, item);

		item = new QTableWidgetItem( QString("%1").arg(simDef.m_cycle) );
		m_ui->tableWidgetSlaves->setItem(i, 3, item);
	}
	m_ui->tableWidgetSlaves->resizeColumnsToContents();
	m_ui->tableWidgetSlaves->horizontalHeader()->resizeSection(0,m_ui->tableWidgetSlaves->verticalHeader()->defaultSectionSize());
	// if there is space, stretch 2nd column
	unsigned int colSizeSum = 0;
	for (unsigned int i=0; i<4; ++i)
		colSizeSum += m_ui->tableWidgetSlaves->horizontalHeader()->sectionSize(i);
	unsigned int contentsRectWith = m_ui->tableWidgetSlaves->contentsRect().width();

	if (colSizeSum < contentsRectWith-20) {
		unsigned int totalSize = m_ui->tableWidgetSlaves->horizontalHeader()->sectionSize(0) +
								 m_ui->tableWidgetSlaves->horizontalHeader()->sectionSize(1) +
								 m_ui->tableWidgetSlaves->horizontalHeader()->sectionSize(3);
		m_ui->tableWidgetSlaves->horizontalHeader()->resizeSection(2, contentsRectWith-totalSize);
	}

	m_ui->toolButtonRemoveSlave->setEnabled(!project().m_simulators.empty());
	if (!project().m_simulators.empty()) {
		if (currentSlaveIdx == -1)
			currentSlaveIdx = 0;
		currentSlaveIdx = qMin<int>(currentSlaveIdx, project().m_simulators.size()-1);
		m_ui->tableWidgetSlaves->selectRow(currentSlaveIdx);
	}

	blockMySignals(this, false);
	m_ui->widgetProperties->updateProperties(currentSlaveIdx);
}


void MSIMViewSlaves::on_tableWidgetSlaves_currentCellChanged(int currentRow, int /*currentColumn*/, int /*previousRow*/, int /*previousColumn*/) {
	// update property browser for currently selected slave
	m_ui->widgetProperties->updateProperties(currentRow);
}


void MSIMViewSlaves::on_toolButtonCreateConnection_clicked() {
	// set scene into connection mode
	BLOCKMOD::SceneManager * sceneManager = MSIMProjectHandler::instance().sceneManager();
	sceneManager->enableConnectionMode();
}
