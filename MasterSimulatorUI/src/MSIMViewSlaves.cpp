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
#include <QTimer>
#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrinter>
#include <QColorDialog>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
#include <QRandomGenerator>
#endif

#include <unzip.h>
#include <tinyxml.h>

#include <IBK_algorithm.h>
#include <IBK_Path.h>
#include <IBK_CSVReader.h>

#include <BM_Network.h>
#include <BM_Globals.h>

#include "MSIMProjectHandler.h"
#include "MSIMUIConstants.h"
#include "MSIMConversion.h"
#include "MSIMMainWindow.h"
#include "MSIMSceneManager.h"
#include "MSIMSlaveBlock.h"
#include "MSIMBlockEditorDialog.h"
#include "MSIMUndoProject.h"

#include "MSIMUndoSlaves.h"
#include "MSIMUndoConnections.h"
#include "MSIMUndoNetworkGeometry.h"
#include "MSIMUndoSlaveParameters.h"
#include "MSIMUndoConnectionModified.h"
#include "MSIMSettings.h"


MSIMViewSlaves::MSIMViewSlaves(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::MSIMViewSlaves),
	m_blockEditorDialog(nullptr)
{
	m_ui->setupUi(this);

	connect(&MSIMProjectHandler::instance(), SIGNAL(modified(unsigned int,void*)),
			this, SLOT(onModified(unsigned int,void*)));

	m_ui->verticalLayoutConnectors->setContentsMargins(0,0,0,0);
	m_ui->verticalLayoutSlaveProperties->setContentsMargins(0,0,0,0);
	m_ui->verticalLayoutProjectProps->setContentsMargins(0,0,0,0);

	m_ui->widgetProperties->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	m_ui->widgetProperties->horizontalHeader()->setStretchLastSection(true);
	m_ui->widgetProperties->verticalHeader()->setVisible(false);

	// upon start there is nothing selected
	updatePropertyStackedWidget(SS_NothingSelected);

	formatTable(m_ui->widgetProperties);

	m_ui->widgetConnectors->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	m_ui->widgetConnectors->horizontalHeader()->setStretchLastSection(true);
	m_ui->widgetConnectors->verticalHeader()->setVisible(false);

	formatTable(m_ui->widgetConnectors);

	// set the scene showing the network
	m_ui->blockModWidget->setResolution(1);
	m_ui->blockModWidget->setGridStep(BLOCKMOD::Globals::GridSpacing*10);
	m_ui->splitter_2->setStretchFactor(1,1);

	m_ui->doubleSpinBoxLinewidth->setSingleStep(0.1);

	m_ui->tableWidgetSlaves->setStyleSheet("selection-background-color: #dfe7fd;");

	m_ui->checkBoxShowEquations->setChecked(MSIMSettings::instance().m_drawConnectorEquations);
}


MSIMViewSlaves::~MSIMViewSlaves() {
	delete m_ui;
}


void MSIMViewSlaves::editBlockItem(const QString & slaveName) {
	BLOCKMOD::SceneManager * sceneManager = qobject_cast<BLOCKMOD::SceneManager *>(m_ui->blockModWidget->scene());
	Q_ASSERT(sceneManager != nullptr);
	const BLOCKMOD::BlockItem * blockItem = sceneManager->blockItemByName(slaveName);
	Q_ASSERT(blockItem != nullptr);
	onBlockActionTriggered(blockItem);
}


bool MSIMViewSlaves::extractFMUAndParseModelDesc(const IBK::Path & fmuFilePath,
												QString & msgLog, MASTER_SIM::ModelDescription & modelDesc, QPixmap & modelPixmap)
{

	// check if the simulation slave is actually a csv/tsv file and use the file reader instead
	if (!IBK::string_nocase_compare(fmuFilePath.extension(), "fmu")) {
		// attempt to read the file as csv/tsv file
		msgLog.append( tr("Reading tabulated data from '%1'\n").arg(QString::fromStdString(fmuFilePath.str())));
		try {
			bool tabFormat = IBK::CSVReader::haveTabSeparationChar(fmuFilePath);
			IBK::CSVReader reader;
			if (tabFormat)
				reader.m_separationCharacter = '\t';
			else
				reader.m_separationCharacter = ',';
			reader.read(fmuFilePath, true, true); // only read header

			// special convention: no time unit, assume "s" seconds
			if (reader.m_units.size() > 0 && reader.m_units[0].empty())
				reader.m_units[0] = "s";

			// sanity checks
			if (reader.m_nColumns < 1 || IBK::Unit(reader.m_units[0]).base_id() != IBK_UNIT_ID_SECONDS)
				throw IBK::Exception("Invalid number of columns or invalid/undefined/missing time unit in first column.",
									 "[MSIMMainWindow::extractFMUsAndParseModelDesc]");

			// now create a model description for this file
			modelDesc.m_modelName = fmuFilePath.filename().withoutExtension().str();
			modelDesc.m_fmuType = MASTER_SIM::ModelDescription::CS_v1;
			// for now we only support double parameters
			for (unsigned int i=1; i<reader.m_nColumns; ++i) {
				// get quantity
				MASTER_SIM::FMIVariable v;
				v.m_name = reader.m_captions[i];
				IBK::trim(v.m_name, " \t\r\"");
				v.m_unit = reader.m_units[i];
				v.m_type = MASTER_SIM::FMIVariable::NUM_VT; // unassigned variable type
				v.m_varIdx = i;
				v.m_causality = MASTER_SIM::FMIVariable::C_OUTPUT;
				v.m_variability = "continuous";
				modelDesc.m_variables.push_back(v);
			}

			msgLog.append( tr("  Variables: %1\n").arg(modelDesc.m_variables.size()));
			for (size_t i=0; i<modelDesc.m_variables.size(); ++i) {
				msgLog.append("    " + QString::fromStdString(modelDesc.m_variables[i].toString()) + "\n");
			}
		}
		catch (IBK::Exception & ex) {
			ex.writeMsgStackToError();
			msgLog.append( tr("Error reading header from csv/tsv file (invalid format?)."));
			return false;
		}
		return true;
	}


	// attempt to unzip FMU
	msgLog.append( tr("Extracting '%1'\n").arg(QString::fromStdString(fmuFilePath.str())));

	std::string fileContent;

	// on windows, unzOpen() needs the filename in local encoding
#ifdef _WIN32
	std::string filenameAnsi = IBK::WstringToANSI(fmuFilePath.wstr(), false);
	unzFile zip = unzOpen(filenameAnsi.c_str());
#else
	unzFile zip = unzOpen(fmuFilePath.c_str());
#endif
	if (zip) {
		if (unzLocateFile(zip, "modelDescription.xml", 1) != UNZ_OK) {
			msgLog.append( tr("ERROR: FMU does not contain the file modelDescription.xml.\n"));
			unzClose( zip );
			return false;
		}

		if (unzOpenCurrentFile( zip ) != UNZ_OK) {
			msgLog.append( tr("ERROR: Could not open modelDescription.xml.\n"));
			unzClose( zip );
			return false;
		}

		unsigned char buffer[4096];
		int readBytes;
		do {
			readBytes = unzReadCurrentFile(zip, buffer, 4096);
			if (readBytes < 0) {
				msgLog.append( tr("ERROR: Error while extracting modelDescription.xml.\n"));
				unzCloseCurrentFile(zip);
				unzClose( zip );
				return false;
			}
			if (readBytes > 0) {
				fileContent += std::string(buffer, buffer+readBytes);
			}
		}
		while (readBytes > 0);

		unzCloseCurrentFile(zip);

		// also try to read the "model.png" file, if it exists
		if (unzLocateFile(zip, "model.png", 1) == UNZ_OK) {
			if (unzOpenCurrentFile( zip ) == UNZ_OK) {
				char buffer[4096];
				int readBytes;
				QByteArray byteArray;
				do {
					readBytes = unzReadCurrentFile(zip, buffer, 4096);
					if (readBytes < 0) {
						msgLog.append( tr("ERROR: Error while extracting model.png.\n"));
						unzCloseCurrentFile(zip);
						unzClose( zip );
						return false;
					}
					if (readBytes > 0) {
						byteArray.append(buffer, readBytes);
					}
				}
				while (readBytes > 0);
				QPixmap p;
				if (p.loadFromData(byteArray, "png")) {
					modelPixmap = p;
				}
			}
			unzCloseCurrentFile(zip);
		}
		unzClose(zip);
	}
	else {
		msgLog.append( tr("ERROR: Could not open FMU file (not a zip archive?).\n"));
		return false;
	}
	try {
		TiXmlDocument doc;
		doc.Parse(fileContent.c_str(), nullptr, TIXML_ENCODING_UTF8);
		if (doc.Error()) {
			msgLog.append( tr("ERROR: Error parsing modelDescription.xml file. Error messages:\n%1\n")
										   .arg(QString::fromUtf8(doc.ErrorDesc())));
			return false;
		}
		modelDesc.readXMLDoc(doc);
		msgLog.append( tr("  Model identifiers:\n"));
		if (!modelDesc.m_modelIdentifier.empty())
			msgLog.append( tr("    FMI v1    : %1\n").arg(QString::fromUtf8(modelDesc.m_modelIdentifier.c_str())));
		if (!modelDesc.m_meV2ModelIdentifier.empty())
			msgLog.append( tr("    FMI v2 ME : %1\n").arg(QString::fromUtf8(modelDesc.m_meV2ModelIdentifier.c_str())));
		if (!modelDesc.m_csV2ModelIdentifier.empty())
			msgLog.append( tr("    FMI v2 CS : %1\n").arg(QString::fromUtf8(modelDesc.m_csV2ModelIdentifier.c_str())));
		msgLog.append( tr("  Variables: %1\n").arg(modelDesc.m_variables.size()));
		// print properties of variables
		for (size_t i=0; i<modelDesc.m_variables.size(); ++i) {
			msgLog.append("    " + QString::fromStdString(modelDesc.m_variables[i].toString()) + "\n");
		}

	}
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		msgLog.append(tr("ERROR: Error parsing modelDescription.xml file. Error messages:\n%1\n")
			.arg(QString::fromStdString(ex.msgStack()) ) );
		return false;
	}
	return true;
}


void MSIMViewSlaves::onModified(unsigned int modificationType, void * /* data */ ) {
	switch (static_cast<MSIMProjectHandler::ModificationTypes>(modificationType)) {
		case MSIMProjectHandler::AllModified :
		case MSIMProjectHandler::SlavesModified : {
			// sync check of all blocks and FMU slaves

			MSIMProjectHandler::instance().syncCoSimNetworkToBlocks();
			updatePropertyStackedWidget(SS_NothingSelected);

			// sync network with graphical display
			if (m_ui->blockModWidget->scene() != nullptr) {
				BLOCKMOD::SceneManager * sceneManager = qobject_cast<BLOCKMOD::SceneManager *>(m_ui->blockModWidget->scene());
				Q_ASSERT(sceneManager != nullptr);
				disconnect(sceneManager, &BLOCKMOD::SceneManager::blockActionTriggered,
						   this, &MSIMViewSlaves::onBlockActionTriggered);
				disconnect(sceneManager, &BLOCKMOD::SceneManager::newConnectionAdded,
						   this, &MSIMViewSlaves::onNewConnectionCreated);
				disconnect(sceneManager, &BLOCKMOD::SceneManager::networkGeometryChanged,
						   this, &MSIMViewSlaves::onNetworkGeometryChanged);
				disconnect(sceneManager, &BLOCKMOD::SceneManager::newBlockSelected,
						   this, &MSIMViewSlaves::onBlockSelected);
				disconnect(sceneManager, &BLOCKMOD::SceneManager::newConnectorSelected,
						this, &MSIMViewSlaves::onConnectorSelected);
				disconnect(sceneManager, &BLOCKMOD::SceneManager::selectionCleared,
						this, &MSIMViewSlaves::onSelectionCleared);
			}
			BLOCKMOD::SceneManager * newSceneManager = const_cast<BLOCKMOD::SceneManager *>(MSIMProjectHandler::instance().sceneManager());
			m_ui->blockModWidget->setScene(newSceneManager);
			connect(newSceneManager, &BLOCKMOD::SceneManager::blockActionTriggered,
					this, &MSIMViewSlaves::onBlockActionTriggered);
			connect(newSceneManager, &BLOCKMOD::SceneManager::newConnectionAdded,
					this, &MSIMViewSlaves::onNewConnectionCreated);
			connect(newSceneManager, &BLOCKMOD::SceneManager::networkGeometryChanged,
					this, &MSIMViewSlaves::onNetworkGeometryChanged);
			connect(newSceneManager, &BLOCKMOD::SceneManager::newBlockSelected,
					this, &MSIMViewSlaves::onBlockSelected);
			connect(newSceneManager, &BLOCKMOD::SceneManager::newConnectorSelected,
					this, &MSIMViewSlaves::onConnectorSelected);
			connect(newSceneManager, &BLOCKMOD::SceneManager::selectionCleared,
					this, &MSIMViewSlaves::onSelectionCleared);
			newSceneManager->installEventFilter ( m_ui->blockModWidget );

			m_ui->textEditDescription->blockSignals(true);
			m_ui->textEditDescription->setPlainText(QString::fromStdString(project().m_comment));
			m_ui->textEditDescription->blockSignals(false);
		} break;

		case MSIMProjectHandler::SlaveParameterModified :
			updatePropertyStackedWidget(SS_SlaveSelected);
			return;

		case MSIMProjectHandler::ProjectPathModified :
			// only need to update if we show relative FMU paths
			if (m_ui->checkBoxRelativeFMUPaths->isChecked())
				break;
			return;

		case MSIMProjectHandler::ConnectionsModified:
			// connector (line) properties and/or connections may have been modified - so update the network
			MSIMProjectHandler::instance().syncCoSimNetworkToBlocks();
			updatePropertyStackedWidget(SS_NothingSelected);
			return;

		case MSIMProjectHandler::SingleConnectionModified:
			// redraw equation text in connections
			MSIMProjectHandler::instance().syncCoSimNetworkToBlocks();
			return;

		// the remaining undo actions do not affect this view
		case MSIMProjectHandler::SimulationSettingsModified :
		case MSIMProjectHandler::NetworkGeometryModified:
			return; // ignored
	}

	updateSlaveTable();
}


template <typename iterator, typename const_iterator>
std::string pickSlaveName(const std::string& basename, iterator first, const_iterator last) {
	if (first==last) return basename;
	std::string basename_mod = basename;
	std::string name;
	int i=0;
	do {
		std::stringstream namestrm;
		if (++i==1)  namestrm << basename_mod;
		else         namestrm << basename_mod << "_" << i;
		name = namestrm.str();
	} while (std::find(first, last, name)!=last);
	return name;
}


void MSIMViewSlaves::on_toolButtonAddSlave_clicked() {
	// use directory where last time an FMU was imported from (in this session)
	// if slave's are imported the first time this session, use the project directory
	QString fmuSearchPath = MSIMSettings::instance().m_propertyMap[MSIMSettings::PT_LastFMUImportDirectory].toString();
	if (fmuSearchPath.isEmpty()) {
		fmuSearchPath = MSIMSettings::instance().m_propertyMap[MSIMSettings::PT_LastFileOpenDirectory].toString();
	}

	// open file dialog and let user select FMU file
	QString fname = QFileDialog::getOpenFileName(this, tr("Select FMU"), fmuSearchPath,
												 tr("Slave files (*.fmu *.tsv *.csv);;FMUs (*.fmu)"),
												 nullptr, QFileDialog::DontUseNativeDialog
												 );
	if (fname.isEmpty())
		return; // Dialog was cancelled

	QFileInfo finfo(fname);
	fmuSearchPath = finfo.dir().absolutePath();
	MSIMSettings::instance().m_propertyMap[MSIMSettings::PT_LastFMUImportDirectory] = fmuSearchPath;

	MASTER_SIM::Project p = project();
	BLOCKMOD::Network n = MSIMProjectHandler::instance().network();

	// create simulator definition
	MASTER_SIM::Project::SimulatorDef simDef;
	simDef.m_name = finfo.baseName().toStdString(); // disambiguity?
	simDef.m_name = pickSlaveName(simDef.m_name, p.m_simulators.begin(), p.m_simulators.end());
	simDef.m_pathToFMU = fname.toStdString();
	p.m_simulators.push_back(simDef);

	// create a block for the graphical representation

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
	unsigned int gx = static_cast<unsigned int>(BLOCKMOD::Globals::GridSpacing*QRandomGenerator::global()->generateDouble()*40.0);
	unsigned int gy = static_cast<unsigned int>(BLOCKMOD::Globals::GridSpacing*QRandomGenerator::global()->generateDouble()*40.0);
#else
	unsigned int gx = static_cast<unsigned int>(BLOCKMOD::Globals::GridSpacing*qrand()*40.0/RAND_MAX);
	unsigned int gy = static_cast<unsigned int>(BLOCKMOD::Globals::GridSpacing*qrand()*40.0/RAND_MAX);
#endif
	BLOCKMOD::Block b(QString::fromStdString(simDef.m_name), gx, gy);
	b.m_size = QSize(static_cast<int>(BLOCKMOD::Globals::GridSpacing*8), static_cast<int>(BLOCKMOD::Globals::GridSpacing*12));
	n.m_blocks.push_back(b);

	// create undo action - this will update the network and also
	MSIMUndoSlaves * cmd = new MSIMUndoSlaves(tr("Slave added"), p, n);
	cmd->push();

	// new block still has NoFmuState

	// now trigger the "analyze FMU" action, but silently, without showing results
	QString msgLog;
	MASTER_SIM::ModelDescription modelDesc;
	QPixmap modelPixmap;
	bool res = extractFMUAndParseModelDesc(simDef.m_pathToFMU, msgLog, modelDesc, modelPixmap);
	if (!res) {
		QMessageBox box(QMessageBox::Critical, tr("Error analyzing FMU/Slave"), tr("There were errors while analizing the slave FMU or data table."), QMessageBox::Ok);
		box.setDetailedText(msgLog);
		box.exec();
	}
	else {
		// insert model description to global model description list
		IBK::Path fmuFullPath = simDef.m_pathToFMU.absolutePath();
		MSIMMainWindow::addModelDescription(fmuFullPath, modelDesc, modelPixmap);

		// new block has now an FMU to look after, so call the sync function to update its appearance
		MSIMProjectHandler::instance().syncCoSimNetworkToBlocks();

		// and finally signal main window to open editor
		// to handle this new slave (i.e. show FMU info dialog)
		emit newSlaveAdded(QString::fromStdString(simDef.m_name), QString::fromStdString(fmuFullPath.str()));
	}
}


void MSIMViewSlaves::on_toolButtonRemoveSlave_clicked() {
	MASTER_SIM::Project p = project();
	BLOCKMOD::Network n = MSIMProjectHandler::instance().network();

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
	BLOCKMOD::Network n = MSIMProjectHandler::instance().network();

	QTableWidgetItem * item = m_ui->tableWidgetSlaves->item(row, column);
	switch (column) {
		case 0 : p.m_simulators[row].m_color = IBK::Color::fromQRgb( item->data(Qt::UserRole).value<QColor>().rgba()); break;
		case 1 : {
			std::string newSlaveName = m_ui->tableWidgetSlaves->item(row, column)->text().trimmed().toStdString();
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
			IBK::Path path( item->text().toStdString() );
			if (!path.isAbsolute()) {
				IBK::Path projectFilePath = IBK::Path(MSIMProjectHandler::instance().projectFile().toStdString());
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


void MSIMViewSlaves::on_tableWidgetSlaves_currentCellChanged(int /*currentRow*/, int /*currentColumn*/, int /*previousRow*/, int /*previousColumn*/) {
	// update property browser for currently selected slave
	updatePropertyStackedWidget(SS_SlaveSelected);
}


void MSIMViewSlaves::onBlockActionTriggered(const BLOCKMOD::BlockItem * blockItem) {
	// first get the block in question
	const BLOCKMOD::Block * b = blockItem->block();
	// check if block has an FMU
	MSIMSlaveBlock::BlockState fmuState = MSIMSlaveBlock::StateNoFMU;
	if (b->m_properties.contains("state"))
		fmuState = (MSIMSlaveBlock::BlockState)b->m_properties["state"].toInt();
	if (fmuState == MSIMSlaveBlock::StateNoFMU) {
		QMessageBox::critical(this, tr("Error in Block Editor"),
							  tr("Cannot edit block without knownledge of inlet/outlet sockets. Must have a valid FMU with a modelDescription.xml file for this slave."));
		return;
	}
	// now startup the block editor

	// editor needs to know number and names of inlet/outlet sockets

	// first lookup associated model description
	// search for simulator definition with this name
	for (const MASTER_SIM::Project::SimulatorDef & simDef : project().m_simulators) {
		if (QString::fromStdString(simDef.m_name) != b->m_name)
			continue;
		// got a simulator with matching name
		const std::map<IBK::Path, MASTER_SIM::ModelDescription> & modelDescriptions = MSIMMainWindow::instance().modelDescriptions();
		// must have a FMU
		std::map<IBK::Path, MASTER_SIM::ModelDescription>::const_iterator it = modelDescriptions.find(simDef.m_pathToFMU);
		Q_ASSERT(it != modelDescriptions.end());
		const MASTER_SIM::ModelDescription & modDesc = it->second;
		// collect list of inlet/outlet variables
		QStringList inletSocketNames;
		QStringList outletSocketNames;
		for (const MASTER_SIM::FMIVariable & var : modDesc.m_variables) {
			if (var.m_causality == MASTER_SIM::FMIVariable::C_INPUT) {
				inletSocketNames.append( QString::fromStdString(var.m_name));
			}
			else if (var.m_causality == MASTER_SIM::FMIVariable::C_OUTPUT) {
				outletSocketNames.append( QString::fromStdString(var.m_name));
			}
		}

		// now launch the editor with a copy of the block
		if (m_blockEditorDialog == nullptr)
			m_blockEditorDialog = new MSIMBlockEditorDialog(this);
		int res = m_blockEditorDialog->editBlock(*b, simDef.m_pathToFMU, inletSocketNames, outletSocketNames);
		// if dialog was accepted, we need to create an undo action for modifying network
		// WARNING: This function is called via signal-slot from within a member function of a block object
		//          So we must not call any function that will potentially delete this block object!
		if (res == QDialog::Accepted) {
			QTimer::singleShot(0, this, &MSIMViewSlaves::onBlockEditingCompleted);
		}
	}

}


void MSIMViewSlaves::onBlockEditingCompleted() {
	BLOCKMOD::Network n = MSIMProjectHandler::instance().sceneManager()->network(); // new network
	auto bit = n.m_blocks.begin(); std::advance(bit, m_blockEditorDialog->m_modifiedBlockIdx);
	*bit = m_blockEditorDialog->m_modifiedBlock;
	MSIMUndoSlaves * undo = new MSIMUndoSlaves(tr("Changed block definition"), project(), n);
	undo->push();
}


void MSIMViewSlaves::onNewConnectionCreated() {
	// get the last connection made from network
	const BLOCKMOD::Network & n = MSIMProjectHandler::instance().sceneManager()->network(); // access new network here!
	const BLOCKMOD::Connector & con = n.m_connectors.back();

	MASTER_SIM::Project::GraphEdge edge;
	edge.m_outputVariableRef = con.m_sourceSocket.toStdString();
	edge.m_inputVariableRef = con.m_targetSocket.toStdString();

	MASTER_SIM::Project p = project();
	p.m_graph.push_back(edge);

	// add undo action for new connection
	MSIMUndoConnections * cmd = new MSIMUndoConnections(tr("Connection added"), p, n);
	cmd->push();
}


void MSIMViewSlaves::onNetworkGeometryChanged() {
	const BLOCKMOD::Network & n = MSIMProjectHandler::instance().sceneManager()->network(); // new network
	// add undo action
	MSIMUndoNetworkGeometry * cmd = new MSIMUndoNetworkGeometry(tr("Network geometry modified"), n);
	cmd->push();
}


void MSIMViewSlaves::onBlockSelected(const QString & blockName) {
	m_selectedEdgeIdx = -1;
	// find corresponding row in table widget
	for (int i=0; i<m_ui->tableWidgetSlaves->rowCount(); ++i) {
		if (m_ui->tableWidgetSlaves->item(i,1)->text() == blockName) {
			m_ui->tableWidgetSlaves->blockSignals(true);
			m_ui->tableWidgetSlaves->selectRow(i);
			m_ui->tableWidgetSlaves->blockSignals(false);
			updatePropertyStackedWidget(SS_SlaveSelected);
			break;
		}
	}
}


void MSIMViewSlaves::onConnectorSelected(const QString & sourceSocketName, const QString & targetSocketName) {
	// find selected GraphEdge, store its index and update its properties
	m_selectedEdgeIdx = -1;
	for (unsigned int i=0; i<project().m_graph.size(); ++i) {
		const MASTER_SIM::Project::GraphEdge &edge = project().m_graph[i];
		if (sourceSocketName.toStdString() == edge.m_outputVariableRef && targetSocketName.toStdString() == edge.m_inputVariableRef) {
			m_selectedEdgeIdx = (int)i;
			updatePropertyStackedWidget(SS_ConnectorSelected);
			break;
		}
	}
}


void MSIMViewSlaves::onSelectionCleared() {
	m_selectedEdgeIdx = -1;
	updatePropertyStackedWidget(SS_NothingSelected);
}


void MSIMViewSlaves::printScene() {
	// open print preview dialog and print schematics
	QPrinter prn;
	QPrintDialog printDlg(&prn, this);
	if (printDlg.exec() == QDialog::Accepted) {
		QPainter painter;
		painter.begin(&prn);
		m_ui->blockModWidget->render(&painter);
	}
}


void MSIMViewSlaves::on_widgetProperties_itemChanged(QTableWidgetItem *item) {
	// triggered when user has just changed an item
	unsigned int currentSlaveIndex = (unsigned int)m_ui->tableWidgetSlaves->currentRow();
	const MASTER_SIM::Project::SimulatorDef & simDef = project().m_simulators[currentSlaveIndex];
	std::string slaveName = simDef.m_name;

	QString varName = m_ui->widgetProperties->item(item->row(), 0)->text();

	// replace spaces with _
	varName = varName.trimmed().replace(' ', '_');

	// create an undo-action for modifying a slave parameter
	MSIMUndoSlaveParameters * undo = new MSIMUndoSlaveParameters(tr("Parameter/variable '%1.%2' modified.")
																 .arg(QString::fromStdString(slaveName), varName),
																 currentSlaveIndex, varName.toStdString(), item->text().toStdString());
	undo->push();
}




// *** private functions ***

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
			IBK::Path projectFilePath = IBK::Path(MSIMProjectHandler::instance().projectFile().toStdString());
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
	else {
		currentSlaveIdx = -1;
	}

	blockMySignals(this, false);
}


void MSIMViewSlaves::updateSlaveParameterTable(unsigned int slaveIndex) {
	// remember current selection
	int selectedRow = m_ui->widgetProperties->currentRow();

	m_ui->widgetProperties->blockSignals(true);
	m_ui->widgetProperties->clearContents();
	m_ui->widgetProperties->setRowCount(0);
	if (slaveIndex == (unsigned int)-1) {
		m_ui->widgetProperties->blockSignals(false);
		return;
	}
	// look up slave
	Q_ASSERT(project().m_simulators.size() > slaveIndex);
	const MASTER_SIM::Project::SimulatorDef & simDef = project().m_simulators[slaveIndex];
	// and try to look up modelDescription
	IBK::Path fmuFullPath = simDef.m_pathToFMU;
	fmuFullPath = fmuFullPath.absolutePath(); // we compare by absolute path with removed /../../ etc.
	const std::map<IBK::Path, MASTER_SIM::ModelDescription>	& modelDescriptions = MSIMMainWindow::instance().modelDescriptions();
	std::map<IBK::Path, MASTER_SIM::ModelDescription>::const_iterator it = modelDescriptions.find(fmuFullPath);

	std::map<std::string, std::string> customParameters = simDef.m_parameters;

	int currentRow = 0;
	QFont f;
	f.setBold(true);
	if (it != modelDescriptions.end()) {
		for (const MASTER_SIM::FMIVariable & var : it->second.m_variables) {
			if (var.m_causality != MASTER_SIM::FMIVariable::C_PARAMETER)
				continue;
			// show parameter name and given unit
			std::string paraName = var.m_name;
			std::map<std::string, std::string>::const_iterator para_it = simDef.m_parameters.begin();
			while (para_it != simDef.m_parameters.end() && para_it->first != paraName)
				++para_it;
			bool found = (para_it != simDef.m_parameters.end());
			m_ui->widgetProperties->setRowCount(currentRow+1);
			QTableWidgetItem * item = new QTableWidgetItem(QString::fromStdString(paraName));
			item->setFlags(Qt::ItemIsEnabled);
			QTableWidgetItem * valueItem = new QTableWidgetItem("");
			valueItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable); // allow double-click to edit
			valueItem->setToolTip(QString("start=%1").arg(QString::fromStdString(var.m_startValue)));
			if (found) {
				customParameters.erase(paraName);
				item->setFont(f);

				valueItem->setText(QString::fromStdString(para_it->second));
				valueItem->setFont(f);
			}
			else {
				valueItem->setForeground(QColor(64,64,64));
				QFont fi;
				fi.setItalic(true);
				valueItem->setFont(fi);
				// special handling for result root dir
				if (paraName == "ResultsRootDir") {
					IBK::Path msimProjectPath(MSIMProjectHandler::instance().projectFile().toStdString());
					if (msimProjectPath.isValid()) {
						msimProjectPath = msimProjectPath.parentPath();
						IBK::Path slaveRootDir = msimProjectPath / "slaves" / simDef.m_name;
						valueItem->setText(QString::fromStdString(slaveRootDir.str()));
						valueItem->setToolTip(QString::fromStdString(slaveRootDir.str()));
					}
					else {
						valueItem->setText(tr("Project needs to be saved first."));
					}
				}
				else
					valueItem->setText(QString::fromStdString(var.m_startValue));
			}
			m_ui->widgetProperties->setItem(currentRow, 0, item);
			m_ui->widgetProperties->setItem(currentRow, 1, valueItem);
			++currentRow;
		}
	}

	// show remaining parameters that are specified in simulator
	f.setItalic(true);
	f.setBold(false);
	for (std::map<std::string, std::string>::const_iterator m = customParameters.begin(); m != customParameters.end(); ++m) {
		m_ui->widgetProperties->setRowCount(currentRow+1);
		QTableWidgetItem * item = new QTableWidgetItem(QString::fromStdString(m->first));
		item->setFlags(Qt::ItemIsEnabled);
		item->setFont(f);
		m_ui->widgetProperties->setItem(currentRow, 0, item);
		item = new QTableWidgetItem(QString::fromStdString(m->second));
		item->setFlags(Qt::ItemIsEnabled);
		item->setFont(f);
		m_ui->widgetProperties->setItem(currentRow, 1, item);
		++currentRow;
	}
	m_ui->widgetProperties->blockSignals(false);

	// reselect item that was selected/current before updating the table
	if (selectedRow < m_ui->widgetProperties->rowCount()) {
		m_ui->widgetProperties->setCurrentCell(selectedRow, 1);
	}
}


void MSIMViewSlaves::updateGraphProperties() {
	const MASTER_SIM::Project &p = project();
	Q_ASSERT(p.m_graph.size() > (unsigned int)m_selectedEdgeIdx);
	const MASTER_SIM::Project::GraphEdge &edge = p.m_graph[(unsigned int)m_selectedEdgeIdx];

	m_ui->widgetConnectors->blockSignals(true);
	m_ui->widgetConnectors->clearContents();
	m_ui->widgetConnectors->setRowCount(3);

	QTableWidgetItem *itemOffsetLabel = new QTableWidgetItem(tr("Offset"));
	itemOffsetLabel->setFlags(Qt::ItemIsEnabled);
	m_ui->widgetConnectors->setItem(0, 0, itemOffsetLabel);
	QTableWidgetItem *itemOffset = new QTableWidgetItem(QString("%L1").arg(edge.m_offset));
	m_ui->widgetConnectors->setItem(0, 1, itemOffset);

	QTableWidgetItem *itemFactorLabel = new QTableWidgetItem(tr("Factor"));
	itemFactorLabel->setFlags(Qt::ItemIsEnabled);
	m_ui->widgetConnectors->setItem(1, 0, itemFactorLabel);
	QTableWidgetItem *itemFactor = new QTableWidgetItem(QString("%L1").arg(edge.m_scaleFactor));
	m_ui->widgetConnectors->setItem(1, 1, itemFactor);

	QTableWidgetItem *itemDividerLabel = new QTableWidgetItem(tr("Divider"));
	itemDividerLabel->setFlags(Qt::ItemIsEnabled);
	m_ui->widgetConnectors->setItem(2, 0, itemDividerLabel);
	QTableWidgetItem *itemDivider = new QTableWidgetItem(QString("%L1").arg(1./edge.m_scaleFactor));
	m_ui->widgetConnectors->setItem(2, 1, itemDivider);

	m_ui->widgetConnectors->blockSignals(false);

	// set color in push button
	m_ui->pushButtonSelectColor->setColor(p.m_graph[(unsigned int)m_selectedEdgeIdx].m_color.toQRgb());

	m_ui->doubleSpinBoxLinewidth->blockSignals(true);
	m_ui->doubleSpinBoxLinewidth->setValue(edge.m_linewidth);
	m_ui->doubleSpinBoxLinewidth->blockSignals(false);

}


void MSIMViewSlaves::updatePropertyStackedWidget(SelectionState selectionState) {

	m_ui->pushButtonDeleteConnection->setEnabled(false);
	m_ui->toolButtonRemoveSlave->setEnabled(false);

	switch (selectionState) {
		case SS_SlaveSelected:
			m_ui->stackedWidget->setCurrentIndex(0);
			m_ui->toolButtonRemoveSlave->setEnabled(true);
			updateSlaveParameterTable((unsigned int)m_ui->tableWidgetSlaves->currentRow());
		break;
		case SS_ConnectorSelected:
			m_ui->stackedWidget->setCurrentIndex(1);
			m_ui->tableWidgetSlaves->clearSelection();
			m_ui->pushButtonDeleteConnection->setEnabled(true);
			updateGraphProperties();
		break;
		case SS_NothingSelected:
			m_ui->stackedWidget->setCurrentIndex(2);
			m_ui->tableWidgetSlaves->clearSelection();
		break;
	}
}


void MSIMViewSlaves::on_widgetConnectors_itemChanged(QTableWidgetItem *item) {
	if (m_selectedEdgeIdx<0)
		return;
	MASTER_SIM::Project p = project();
	Q_ASSERT(p.m_graph.size() > (unsigned int)m_selectedEdgeIdx);
	Q_ASSERT(item->row()<3);
	Q_ASSERT(item->column()==1);

	bool ok;
	double value = QLocale().toDouble(item->text(), &ok);
	if (!ok)
		value = item->text().toDouble(&ok);
	if (!ok) {
		// reset value
		if (item->row()==0)
			item->setText(QString("%1").arg(p.m_graph[(unsigned int)m_selectedEdgeIdx].m_offset));
		else
			item->setText(QString("%1").arg(p.m_graph[(unsigned int)m_selectedEdgeIdx].m_scaleFactor));
		QMessageBox::critical(this, tr("Connection parameter error"), tr("Invalid value for offset."));
		return;
	}

	// set value
	if (item->row()==0)
		p.m_graph[(unsigned int)m_selectedEdgeIdx].m_offset = value;
	else if (item->row()==1)
		p.m_graph[(unsigned int)m_selectedEdgeIdx].m_scaleFactor = value;
	else
		p.m_graph[(unsigned int)m_selectedEdgeIdx].m_scaleFactor = 1. / value;

	MSIMUndoConnectionModified * undo = new MSIMUndoConnectionModified(tr("Changed connection properties"), p);
	undo->push();

	// needed for consistent divider / factor value
	updateGraphProperties();
}


void MSIMViewSlaves::on_pushButtonDeleteConnection_clicked() {
	if (m_selectedEdgeIdx<0)
		return;
	// erase edge from graph
	MASTER_SIM::Project p = project();
	Q_ASSERT(p.m_graph.size() > (unsigned int)m_selectedEdgeIdx);
	p.m_graph.erase(p.m_graph.begin() + m_selectedEdgeIdx);

	const BLOCKMOD::Network & n = MSIMProjectHandler::instance().sceneManager()->network(); // access new network here!
	MSIMUndoConnections * cmd = new MSIMUndoConnections(tr("Connection removed"), p, n);
	cmd->push();
}


void MSIMViewSlaves::on_doubleSpinBoxLinewidth_valueChanged(double arg1) {
	if (m_selectedEdgeIdx<0)
		return;
	MASTER_SIM::Project p = project();
	Q_ASSERT(p.m_graph.size() > (unsigned int)m_selectedEdgeIdx);
	p.m_graph[(unsigned int)m_selectedEdgeIdx].m_linewidth = arg1;

	MSIMUndoConnectionModified * undo = new MSIMUndoConnectionModified(tr("Changed connection properties"), p);
	undo->push();
}


void MSIMViewSlaves::on_pushButtonSelectColor_colorChanged() {
	Q_ASSERT(m_selectedEdgeIdx >= 0);
	MASTER_SIM::Project p = project();
	Q_ASSERT((unsigned int)m_selectedEdgeIdx < p.m_graph.size());

	QColor col = m_ui->pushButtonSelectColor->color();
	p.m_graph[(unsigned int)m_selectedEdgeIdx].m_color = IBK::Color((unsigned int)col.red(), (unsigned int)col.green(),
																	(unsigned int)col.blue(), (unsigned int)col.alpha());

	MSIMUndoConnectionModified * undo = new MSIMUndoConnectionModified(tr("Changed connection properties"), p);
	undo->push();
}


void MSIMViewSlaves::on_checkBoxShowEquations_clicked(bool checked) {
	MSIMSettings::instance().m_drawConnectorEquations = checked;
	MSIMProjectHandler::instance().syncCoSimNetworkToBlocks();
}


void MSIMViewSlaves::on_textEditDescription_editingFinished() {
	std::string newComment = m_ui->textEditDescription->toPlainText().toStdString();
//	size_t i = newComment.find("</head><body");
//	if (i != std::string::npos) {
//		// find closing >
//		i = newComment.find(">",i+8);
//		if (i != std::string::npos && newComment.length() > i+1)
//			newComment = newComment.substr(i+1);
//		i = newComment.find("</body></html>");
//		newComment = newComment.substr(0,i);
//	}

	if (newComment == project().m_comment)
		return;
	qDebug() << "new comment";
	MASTER_SIM::Project p = project();
	p.m_comment = newComment;
	MSIMUndoProject * undo = new MSIMUndoProject(tr("Modified project description"), p);
	undo->push();
}

