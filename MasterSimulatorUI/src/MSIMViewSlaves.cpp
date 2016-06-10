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

#include <IBK_algorithm.h>

#include "MSIMProjectHandler.h"
#include "MSIMUIConstants.h"
#include "MSIMSlaveItemDelegate.h"
#include "MSIMConversion.h"
#include "MSIMUndoSlaves.h"
#include "MSIMMainWindow.h"

MSIMViewSlaves::MSIMViewSlaves(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::MSIMViewSlaves)
{
	m_ui->setupUi(this);
	m_ui->verticalLayout_2->setContentsMargins(9,0,9,9);

	connect(&MSIMProjectHandler::instance(), SIGNAL(modified(int,void*)), this, SLOT(onModified(int,void*)));

	// setup tables
	m_ui->tableWidgetFMUs->horizontalHeader()->setVisible(false);
	m_ui->tableWidgetFMUs->verticalHeader()->setVisible(false);
	m_ui->tableWidgetFMUs->setColumnCount(1);
	QStringList headers;
	headers << tr("Full path");
	m_ui->tableWidgetFMUs->setHorizontalHeaderLabels(headers);
	m_ui->tableWidgetFMUs->horizontalHeader()->setStretchLastSection(true);

//	m_ui->tableWidgetSlaves->horizontalHeader()->setVisible(false);
	m_ui->tableWidgetSlaves->verticalHeader()->setVisible(false);
	m_ui->tableWidgetSlaves->setColumnCount(4);
	headers.clear();
	headers << "" << tr("Name") << tr("FMU") << tr("Cycle Nr.");
	m_ui->tableWidgetSlaves->setHorizontalHeaderLabels(headers);

	formatTable(m_ui->tableWidgetFMUs);
	formatTable(m_ui->tableWidgetSlaves);

	m_ui->tableWidgetSlaves->horizontalHeader()->resizeSection(0,m_ui->tableWidgetSlaves->verticalHeader()->defaultSectionSize());
	m_ui->tableWidgetSlaves->setItemDelegate(new MSIMSlaveItemDelegate(this));

	m_ui->tableWidgetFMUs->setSortingEnabled(true);

	connect(m_ui->pushButtonParseFMUs, SIGNAL(clicked()), &MSIMMainWindow::instance(), SLOT(on_actionEditParseFMUs_triggered()));
}


MSIMViewSlaves::~MSIMViewSlaves() {
	delete m_ui;
}


void MSIMViewSlaves::onModified( int modificationType, void * /* data */ ) {
	switch ((MSIMProjectHandler::ModificationTypes)modificationType) {
		case MSIMProjectHandler::AllModified :
		case MSIMProjectHandler::SlavesModified :
			break;
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
	QSettings settings(ORG_NAME, PROGRAM_NAME);
	QString fmuSearchPath = settings.value("FMUSearchPath", QString()).toString();
	if (fmuSearchPath.isEmpty()) {
		fmuSearchPath = QFileInfo(MSIMProjectHandler::instance().projectFile()).dir().absolutePath();
	}

	QString fname = QFileDialog::getOpenFileName(this, tr("Select FMU"), fmuSearchPath, tr("FMU files (*.fmu)"));
	if (fname.isEmpty())
		return; // Dialog was cancelled

	QFileInfo finfo(fname);
	fmuSearchPath = finfo.dir().absolutePath();
	settings.setValue("FMUSearchPath", fmuSearchPath);

	MASTER_SIM::Project p = project();

	// create simulator definition
	MASTER_SIM::Project::SimulatorDef simDef;
	simDef.m_name = finfo.baseName().toUtf8().data(); // disambiguity?
	simDef.m_name = IBK::pick_name(simDef.m_name, p.m_simulators.begin(), p.m_simulators.end());
	simDef.m_pathToFMU = fname.toUtf8().data();

	// create undo action
	p.m_simulators.push_back(simDef);

	MSIMUndoSlaves * cmd = new MSIMUndoSlaves(tr("Slave added"), p);
	cmd->push();
}


void MSIMViewSlaves::on_toolButtonRemoveSlave_clicked() {

	MASTER_SIM::Project p = project();

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

	// create undo action
	MSIMUndoSlaves * cmd = new MSIMUndoSlaves(tr("Slave removed"), p);
	cmd->push();
}


void MSIMViewSlaves::on_tableWidgetSlaves_cellChanged(int row, int column) {
	// triggered when editor finishes, update simulator definition in selected row

	MASTER_SIM::Project p = project();

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
	MSIMUndoSlaves * cmd = new MSIMUndoSlaves(tr("Slave modified"), p);
	cmd->push();
}


void MSIMViewSlaves::on_checkBoxRelativeFMUPaths_toggled(bool checked) {
	updateSlaveTable();
}


void MSIMViewSlaves::updateSlaveTable() {
	blockMySignals(this, true);

	int currentSlaveIdx = m_ui->tableWidgetSlaves->currentRow();

	// update tables based on project file content
	m_ui->tableWidgetFMUs->clear();
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
			projectFilePath = projectFilePath.parentPath();
			// now compose relative path
			try {
				fmuFilePath = fmuFilePath.relativePath(projectFilePath);
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
//	qDebug() << m_ui->tableWidgetSlaves->width();
//	qDebug() << contentsRectWith;
//	qDebug() << colSizeSum;
	if (colSizeSum < contentsRectWith-20) {
		unsigned int totalSize = m_ui->tableWidgetSlaves->horizontalHeader()->sectionSize(0) +
								 m_ui->tableWidgetSlaves->horizontalHeader()->sectionSize(1) +
								 m_ui->tableWidgetSlaves->horizontalHeader()->sectionSize(3);
		m_ui->tableWidgetSlaves->horizontalHeader()->resizeSection(2, contentsRectWith-totalSize);
	}
	m_ui->tableWidgetFMUs->setRowCount(fmuPaths.count());
	unsigned int rowIdx = 0;
	for (QSet<QString>::const_iterator it = fmuPaths.constBegin(); it != fmuPaths.constEnd(); ++it, ++rowIdx) {
		QTableWidgetItem * item = new QTableWidgetItem( *it );
		item->setFlags(Qt::ItemIsEnabled);
		m_ui->tableWidgetFMUs->setItem(rowIdx, 0, item);
	}
	m_ui->tableWidgetFMUs->resizeColumnsToContents();

	m_ui->toolButtonRemoveSlave->setEnabled(!project().m_simulators.empty());
	if (!project().m_simulators.empty()) {
		if (currentSlaveIdx == -1)
			currentSlaveIdx = 0;
		currentSlaveIdx = qMin<int>(currentSlaveIdx, project().m_simulators.size()-1);
		m_ui->tableWidgetSlaves->selectRow(currentSlaveIdx);
	}
	m_ui->tableWidgetFMUs->sortByColumn(0, Qt::AscendingOrder);

	blockMySignals(this, false);}
