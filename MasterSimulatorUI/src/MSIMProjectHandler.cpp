#include "MSIMProjectHandler.h"

#include <QStringList>
#include <QString>
#include <QMessageBox>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QDialogButtonBox>

#include <IBK_Exception.h>
#include <IBK_FileUtils.h>
#include <IBK_Path.h>
#include <IBK_assert.h>

#include <MSIM_Project.h>

#include <BM_Network.h>
#include <BM_Globals.h>

#include "MSIMSettings.h"
#include "MSIMUIConstants.h"
#include "MSIMDirectories.h"
#include "MSIMLogWidget.h"
#include "MSIMUndoProject.h"
#include "MSIMSceneManager.h"
#include "MSIMSlaveBlock.h"
#include "MSIMMainWindow.h"

MSIMProjectHandler * MSIMProjectHandler::m_self = nullptr;

MSIMProjectHandler & MSIMProjectHandler::instance() {
	Q_ASSERT_X(m_self != nullptr, "[MSIMProjectHandler::instance]",
		"You must not access MSIMProjectHandler::instance() when the is no MSIMProjectHandler "
		"instance (anylonger).");
	return *m_self;
}


MSIMProjectHandler::MSIMProjectHandler() :
	m_project(nullptr),
	m_sceneManager(nullptr),
	m_modified(false)
{
	IBK_ASSERT(m_self == nullptr);
	m_self = this;

	// initialization of other globals
	BLOCKMOD::Globals::GridSpacing = 16;
	BLOCKMOD::Globals::LabelFontSize = 8;
}


MSIMProjectHandler::~MSIMProjectHandler( ){
	// free owned project, if any
	delete m_project;
	delete m_sceneManager;
	m_self = nullptr;
}


void MSIMProjectHandler::newProject(QWidget * /* parent */) {

	createProject();

	setModified(AllModified);

	// signal UI that we now have a project
	emit updateActions();
}


bool MSIMProjectHandler::closeProject(QWidget * parent) {

	// if no project exists, simply return true
	if (!isValid())
		return true;

	// ask user for confirmation to save, if project was modified
	if (isModified()) {

		// ask user for confirmation to save
		int result = QMessageBox::question(
				parent,
				tr("Save project before closing"),
				tr("Would you like to save the project before closing it?"),
				QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
				);

		// user bails out?
		if (result == QMessageBox::Cancel)
			return false; // project not closed

		// saving requested by user
		if (result == QMessageBox::Save) {

			SaveResult res;

			// let user pick new filename
			if (m_projectFile.isEmpty())
				res = saveWithNewFilename(parent);
			else
				res = saveProject(parent, m_projectFile);

			// saving failed ?
			if (res != SaveOK)
				return false;

		}

	} // if (isModified())

	// saving succeeded, now we can close the project
	destroyProject();

	// signal application that we have no longer a project, and thus all project-related
	// actions should be disabled
	emit updateActions();

	// also clear the last FMU import directory
	MSIMSettings::instance().m_propertyMap.remove(MSIMSettings::PT_LastFMUImportDirectory);

	return true;
}


void MSIMProjectHandler::loadProject(QWidget * parent, const QString & fileName,	bool silent) {
	const char * const FUNC_ID = "[MSIMProjectHandler::loadProject]";

	// we must not have a project loaded
	IBK_ASSERT(!isValid());

	// create a new project
	createProject();

	try {
		if (!read(fileName))
			throw IBK::Exception("Error reading project file.", FUNC_ID);
		// project read successfully
		// m_projectFile now holds current project file

		// convert all slave references to absolute file paths
		IBK::Path absoluteProjectFilePath = IBK::Path(m_projectFile.toStdString()).parentPath();
		for (unsigned int i=0; i<m_project->m_simulators.size(); ++i) {
			IBK::Path p = m_project->m_simulators[i].m_pathToFMU; // may be a relative path
			if (!p.isAbsolute()) {
				m_project->m_simulators[i].m_pathToFMU = (absoluteProjectFilePath / p).absolutePath();
			}
		}

		// now silently parse the FMUs

	}
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		if (!silent) {

			QMessageBox::critical(
					parent,
					tr("Error loading project"),
					tr("Error loading project file '%1', see error log file '%2' for details.")
						.arg(fileName)
						.arg(MSIMDirectories::globalLogFile())
			);
			QDialog dlg;
			QVBoxLayout * lay = new QVBoxLayout;
			MSIMLogWidget * logWidget = new MSIMLogWidget;
			logWidget->showLogFile(MSIMDirectories::globalLogFile());
			lay->addWidget(logWidget);
			QDialogButtonBox * btnBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, &dlg);
			lay->addWidget(btnBox);
			connect(btnBox, SIGNAL(accepted()), &dlg, SLOT(accept()));
			dlg.setLayout(lay);
			dlg.resize(1025,600);
			dlg.exec();
		}
		// remove project again
		destroyProject();

		// Note: no need to emit updateActions() here since view state hasn't finished.
		return;
	}


	// first tell project and thus all connected views that the
	// structure of the project has changed
	try {
		setModified(AllModified);
		m_modified = false; // clear modified flag again, since we just read the project
		emit updateActions();
	}
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();

		// project data was incomplete, we show an error message and default to empty project
		if (!silent) {
			QMessageBox::critical(
					parent,
					tr("Error loading project"),
					tr("Data in project was missing/invalid, see error log '%1' for details.").arg(MSIMDirectories::globalLogFile())
			);
		}
		// remove project again
		destroyProject();
		return;
	}

	// If we have read an old project file, the fileName and project().projectFile
	// will be different, because the extension was changed. In this case
	// we leave the modification state to modified and do not add the file to the
	// recent file list.

	// if the filenames are the same, we can savely assume that the project is not
	// modified and we add the file to the recent file list
	if (fileName == m_projectFile) {
		// add project file name to recent file list
		addToRecentFiles(fileName);
	} // if (fileName == m_projectFile)

}


void MSIMProjectHandler::reloadProject(QWidget * parent) {
	QString projectFileName = projectFile();
	m_modified = false; // so that closeProject doesn't ask questions
	closeProject(parent);
	loadProject(parent, projectFileName, false); // emits updateActions() if project was successfully loaded
}


MSIMProjectHandler::SaveResult MSIMProjectHandler::saveWithNewFilename(QWidget * parent) {

	// determine default path from current project file
	QString currentPath = QFileInfo(m_projectFile).filePath();

	// ask user for filename
	QString filename = QFileDialog::getSaveFileName(
			parent,
			tr("Specify project file"),
			currentPath,
			tr("Projects (*%1);;All files (*.*)").arg(DOT_FILE_EXTENSION),
			nullptr, QFileDialog::DontUseNativeDialog
		);

	if (filename.isEmpty()) return SaveCanceled; // canceled

	QString fnamebase = QFileInfo(filename).baseName();
	if (fnamebase.isEmpty()) {
		QMessageBox::critical(parent, tr("Invalid file name"), tr("Please enter a valid file name!"));
		return SaveCanceled;
	}

	// relay to saveProject() which updates modified flag and emits corresponding signals.
	if (saveProject(parent, filename) != SaveOK)
		return SaveFailed; // saving failed

	return SaveOK;
}


MSIMProjectHandler::SaveResult MSIMProjectHandler::saveProject(QWidget * parent, const QString & fileName) {

	// check project file ending, if there is none append it
	QString fname = fileName;
	if (!fname.endsWith(DOT_FILE_EXTENSION)) {
		fname.append(DOT_FILE_EXTENSION);
	}

	// updated created and lastEdited tags
	if (m_project->m_created.empty())
		m_project->m_created = QDateTime::currentDateTime().toString(Qt::TextDate).toStdString();
	m_project->m_lastEdited = QDateTime::currentDateTime().toString(Qt::TextDate).toStdString();

	QString lastFileName = m_projectFile;

	// save project file
	if (!write(fname)) {

		QMessageBox::critical(
				parent,
				tr("Saving failed"),
				tr("Error while saving project file, see error log file '%1' for details.").arg(MSIMDirectories::globalLogFile())
				);

		return SaveFailed;
	}

	if (fileName != lastFileName) {
		// signal that project file has changed
		emit modified(ProjectPathModified, nullptr);
	}

	// clear modified flag
	m_modified = false;
	// signal UI to update project status
	emit updateActions();

	// add project file name to recent file list
	addToRecentFiles(fname);

	// remember path to project
	MSIMSettings::instance().m_propertyMap[MSIMSettings::PT_LastFileOpenDirectory] = QFileInfo(fname).absoluteDir().absolutePath();

	return SaveOK; // saving succeeded
}


void MSIMProjectHandler::setModified(unsigned int modificationType, void * data) {
	// special case:  modification type = NotModified
	ModificationTypes modType = static_cast<ModificationTypes>(modificationType);
	switch (modType) {
		case ConnectionsModified :
			syncCoSimNetworkToBlocks(); break;
		default: ; // skip all others
	}
	m_modified = true;

	emit modified(modificationType, data);
}


const MASTER_SIM::Project & MSIMProjectHandler::project() const {
	const char * const FUNC_ID = "[MSIMProjectHandler::project]";

	if (m_project == nullptr)
		throw IBK::Exception("Must not call project() on invalid ProjectHandler.", FUNC_ID);
	return *m_project;
}


void MSIMProjectHandler::fmiVariableInfosForSocket(const QString & flatname, bool inlet, QString & description, QString & unit) const {
	QString blockName, socketName;
	BLOCKMOD::Network::splitFlatName(flatname, blockName, socketName);

	// lookup simulation definition by block name
	for (const MASTER_SIM::Project::SimulatorDef & s : m_project->m_simulators) {
		if (s.m_name == blockName.toStdString()) {
			// retrieve FMU for given slave
			for (const auto & fmuDesc : MSIMMainWindow::instance().modelDescriptions()) {
				if (fmuDesc.first == s.m_pathToFMU) {
					const MASTER_SIM::ModelDescription & modDesc = fmuDesc.second;
					for (const MASTER_SIM::FMIVariable & var : modDesc.m_variables) {
						if (inlet && var.m_causality == MASTER_SIM::FMIVariable::C_INPUT && var.m_name == socketName.toStdString()) {
							description = QString::fromStdString(var.m_description);
							unit = QString::fromStdString(var.m_unit);
							return;
						}
						else if (!inlet && var.m_causality == MASTER_SIM::FMIVariable::C_OUTPUT && var.m_name == socketName.toStdString()) {
							description = QString::fromStdString(var.m_description);
							unit = QString::fromStdString(var.m_unit);
							return;
						}
					}
				}
			}
			break;
		}
	}
}


void MSIMProjectHandler::updateLastReadTime() {
	const char * const FUNC_ID = "[MSIMProjectHandler::updateLastReadTime]";
	if (!isValid())
		throw IBK::Exception("Must not call updateLastReadTime() on invalid project.", FUNC_ID);
	m_lastReadTime = QFileInfo(projectFile()).lastModified();
}


void MSIMProjectHandler::syncCoSimNetworkToBlocks() {
	// get a copy of the network
	BLOCKMOD::Network n = m_network;

	// remove superfluous blocks (only needed when someone manually edited the graph/simulator
	// defs in project file)
	const MASTER_SIM::Project & prj = *m_project;
	while (n.m_blocks.size() > prj.m_simulators.size()) {
		n.removeBlock(n.m_blocks.size()-1);
	}

	// add missing blocks - newly added blocks will be configured later on
	while (n.m_blocks.size() < prj.m_simulators.size()) {
		BLOCKMOD::Block b;
		b.m_size = QSizeF(BLOCKMOD::Globals::GridSpacing*6, BLOCKMOD::Globals::GridSpacing*8);
		unsigned int blockCount = n.m_blocks.size();
		b.m_pos = QPointF(BLOCKMOD::Globals::GridSpacing*blockCount,
						  BLOCKMOD::Globals::GridSpacing*blockCount);
		n.m_blocks.push_back(b);
	}

	// loop over all simulation slaves
	auto bit = n.m_blocks.begin();
	for (unsigned int i=0; i<prj.m_simulators.size(); ++i, ++bit) {
		const MASTER_SIM::Project::SimulatorDef & simDef = prj.m_simulators[i];
		BLOCKMOD::Block & b = *bit;
		b.m_properties["state"] = MSIMSlaveBlock::StateNoFMU; // assume noFMU - worst case scenario

		// check if name matches the block with the same index in the network
		if (QString::fromStdString(simDef.m_name) != b.m_name) {
			// this block does not match the simulator slave name -> adjust
			b.m_name = QString::fromStdString(simDef.m_name);
		}

		// retrieve FMU for given slave
		const std::map<IBK::Path, MASTER_SIM::ModelDescription> & modelDescriptions = MSIMMainWindow::instance().modelDescriptions();
		if (modelDescriptions.find(simDef.m_pathToFMU) == modelDescriptions.end()) {
			continue;
		}

		const MASTER_SIM::ModelDescription & modDesc = modelDescriptions.at(simDef.m_pathToFMU);

		// generate list of published inlet/outlets from FMU description
		QList<QString> inletSocketNames;
		QList<QString> outletSocketNames;
		for (const MASTER_SIM::FMIVariable & var : modDesc.m_variables) {
			if (var.m_causality == MASTER_SIM::FMIVariable::C_INPUT) {
				inletSocketNames.append( QString::fromStdString(var.m_name));
			}
			else if (var.m_causality == MASTER_SIM::FMIVariable::C_OUTPUT) {
				outletSocketNames.append( QString::fromStdString(var.m_name));
			}
		}

		// now process current block definition and remove all defined sockets from lists - remaining
		// items must
		bool missingSocket = false;
		for (const BLOCKMOD::Socket & s : qAsConst(b.m_sockets)) {
			if (s.m_inlet) {
				if (inletSocketNames.contains(s.m_name))
					inletSocketNames.removeOne(s.m_name);
				else
					missingSocket = true;
			}
			else {
				if (outletSocketNames.contains(s.m_name))
					outletSocketNames.removeOne(s.m_name);
				else
					missingSocket = true;
			}
		}

		if (missingSocket || !inletSocketNames.isEmpty() || !outletSocketNames.isEmpty()) {
			b.m_properties["state"] = MSIMSlaveBlock::StateUnsynced;
		}
		else {
			// check that block socket names match those of the inputs and outputs
			b.m_properties["state"] = MSIMSlaveBlock::StateCorrect;
			// if block has ShowPixmap property set, try to load the pixmap from file
			if (b.m_properties.contains("ShowPixmap") && b.m_properties.value("ShowPixmap").toBool()) {
				QPixmap p = MSIMMainWindow::instance().modelPixmap(b.m_name.toStdString());
				if (p.isNull())
					b.m_properties.remove("Pixmap");
				else
					b.m_properties["Pixmap"] = QVariant(p);
			}
		}

		// now process again all defined sockets and update their description and unit
		for (BLOCKMOD::Socket & s : b.m_sockets) {
			// loop over all FMI variables
			for (const MASTER_SIM::FMIVariable & var : modDesc.m_variables) {
				if (var.m_causality == MASTER_SIM::FMIVariable::C_INPUT && s.m_inlet && s.m_name.toStdString() == var.m_name) {
					s.m_description = QString::fromStdString(var.m_description);
					s.m_unit = QString::fromStdString(var.m_unit);
				}
				else if (var.m_causality == MASTER_SIM::FMIVariable::C_OUTPUT && !s.m_inlet && s.m_name.toStdString() == var.m_name) {
					s.m_description = QString::fromStdString(var.m_description);
					s.m_unit = QString::fromStdString(var.m_unit);
				}
			}
		}
	}

	// now that all blocks are updated, we look for connections

	std::vector<MASTER_SIM::Project::GraphCheckErrorCodes> graphCheckResults;
	prj.checkGraphs(MSIMMainWindow::instance().modelDescriptions(), graphCheckResults);

	// process all connections in connection graph that do not show an error

	// we have to deal with the following situations:
	// 1. no model descriptions read yet for some slaves -> connections to these slaves are marked "Undetermined". Such connections
	//    cannot be shown in the graph and must not appear in the network. However, if we remove them, the routining information is also
	//    lost and we cannot add them later again. So, to avoid loosing information, we simply disable these connections so
	//    that there will no graphics items be created.
	// 2. connections may be invalid, because blocks have been removed, renamed, or variables have changed since last FMU export. In
	//    any of these cases, the connections must be removed because even with updating model descriptions, still no valid connections
	//    can be shown.
	// 3. graph-connections are valid, but do not exist in the network yet. Then, we have to create new connections with default routing.
	// 4. there are connections in the network, but no matching graph connections. We have to remove these from the network.

	// algorith: 1. loop over all graphs defined in project and look up matching connections (via source/target variable references) in network
	//              and create a list of these connection. For missing connections in network, create default invalid connections.
	//           2. then process all graphs and handle the already obtained error codes for each graph connection

	std::list<BLOCKMOD::Connector> connections;
	for (unsigned int i=0; i<prj.m_graph.size(); ++i) {
		const MASTER_SIM::Project::GraphEdge & edge = prj.m_graph[i];
		// if check state of this connection is anything but NoError or Undetermined, ignore this connection
		if (graphCheckResults[i] != MASTER_SIM::Project::GEC_NoError && graphCheckResults[i] != MASTER_SIM::Project::GEC_Undetermined)
			continue;
		// search for this connection in current network
		BLOCKMOD::Connector newCon;
		for (const BLOCKMOD::Connector & con : n.m_connectors) {
			if (con.m_sourceSocket.toStdString() == edge.m_outputVariableRef &&
				con.m_targetSocket.toStdString() == edge.m_inputVariableRef)
			{
				// remember this matching connection
				newCon = con;
				break;
			}
		}
		// in case we didn't find a matching connection, this will be an empty connection and we have to create a new connection now
		if (newCon.m_sourceSocket.isEmpty()) {
			newCon.m_name = "auto-named";
			newCon.m_sourceSocket = QString::fromStdString(edge.m_outputVariableRef);
			newCon.m_targetSocket = QString::fromStdString(edge.m_inputVariableRef);
		}

		// add equation text, if factor or offset given
		if ( MSIMSettings::instance().m_drawConnectorEquations && (edge.m_scaleFactor != 1.0 || edge.m_offset != 0.0) ) {
			std::string outputName, inputName, dummy;
			edge.splitReference(edge.m_outputVariableRef, dummy, outputName);
			edge.splitReference(edge.m_inputVariableRef, dummy, inputName);
			// special handling
			if (edge.m_scaleFactor == 1.0)
				newCon.m_text = QString("%1<sub>(in)</sub> = %2<sub>(out)</sub> + %3")
						.arg(QString::fromStdString(inputName), QString::fromStdString(outputName))
						.arg(edge.m_offset);
			else if (edge.m_offset == 0.0)
				newCon.m_text = QString("%1<sub>(in)</sub> = %3 * %2<sub>(out)</sub>")
						.arg(QString::fromStdString(inputName), QString::fromStdString(outputName))
						.arg(edge.m_scaleFactor);
			else
				newCon.m_text = QString("%1<sub>(in)</sub> = %3 * %2<sub>(out)</sub> + %4")
						.arg(QString::fromStdString(inputName), QString::fromStdString(outputName))
						.arg(edge.m_scaleFactor).arg(edge.m_offset);
		}
		else
			newCon.m_text.clear();
		newCon.m_linewidth = edge.m_linewidth;
		newCon.m_color = QColor(edge.m_color.m_red, edge.m_color.m_green, edge.m_color.m_blue, edge.m_color.m_alpha);

		// one last thing: we can only keep the connection in the network, if the connected blocks have
		// a valid layout and geometry

		// does the network have the referenced blocks and sockets (the latter may not be the case, if
		// the block hasn't been updated with the block editor yet).
		if (n.haveSocket(newCon.m_sourceSocket, false) &&
			n.haveSocket(newCon.m_targetSocket, true))
		{
			connections.push_back(newCon);
		}
	}
	n.m_connectors = connections;

	// now process all connections and update their status based on the
	n.adjustConnectors();

	// update network without undo-action
	m_network = n;
	m_sceneManager->setNetwork(n);
}


// *** PRIVATE MEMBER FUNCTIONS ***

void MSIMProjectHandler::createProject() {
	Q_ASSERT(m_project == nullptr);

	m_project = new MASTER_SIM::Project;
	m_sceneManager = new MSIMSceneManager;
	m_projectFile.clear();
	m_modified = false; // new projects are never modified
}


void MSIMProjectHandler::destroyProject() {
	Q_ASSERT(m_project != nullptr);

	delete m_project;
	delete m_sceneManager;
	m_project = nullptr;
	m_sceneManager = nullptr;
	m_projectFile.clear();
}


bool MSIMProjectHandler::read(const QString & fname) {
	const char * const FUNC_ID = "[MSIMProjectHandler::read]";

	// check that we have a project, should be newly created
	Q_ASSERT(isValid());

	if (!QFileInfo::exists(fname)) {
		IBK::IBK_Message(IBK::FormatString("File '%1' does not exist or permissions are missing for accessing the file.")
						 .arg(fname.toStdString()), IBK::MSG_ERROR, FUNC_ID);
		return false;
	}

	try {

		// filename is converted to utf8 before calling readXML
		IBK::Path fpath(fname.toStdString());
		m_project->read(fpath, false);
		m_projectFile = fname;

		IBK::Path bmPath(fpath.withoutExtension().str() + ".bm");
		BLOCKMOD::Network network;
		if (bmPath.exists()) {
			try {
				network.readXML(QString::fromStdString(bmPath.str()));
				// sanity checks (network level, not synced with slaves yet)
				network.checkNames();
			}
			catch (...) {
				IBK::IBK_Message(IBK::FormatString("Error reading network representation file '%1'.")
								 .arg(bmPath), IBK::MSG_ERROR, FUNC_ID);
				network = BLOCKMOD::Network();
			}
			// now check, if the network contains blocks that do not match slave names
			std::list<BLOCKMOD::Block> checkedBlocks; // this list will hold all blocks that have matching simulators
			for (BLOCKMOD::Block & b : network.m_blocks) {
				std::vector<MASTER_SIM::Project::SimulatorDef>::const_iterator it;
				for ( it = m_project->m_simulators.begin(); it != m_project->m_simulators.end(); ++it) {
					if (it->m_name == b.m_name.toStdString())
						break;
				}
				if (it == m_project->m_simulators.end()) {
					IBK::IBK_Message(IBK::FormatString("Invalid block '%1' in network representation file '%2'.")
									 .arg(b.m_name.toStdString()).arg(bmPath), IBK::MSG_ERROR, FUNC_ID);
					continue; // skip block
				}
				checkedBlocks.push_back(b);
			}
			// swap out block with checkedBlocks
			network.m_blocks.swap(checkedBlocks);

			checkedBlocks.clear(); // start over with empty list, now create list of blocks that matches the list
								   // of simulators
			// add dummy blocks for each simulator, that is not yet in the network
			for (MASTER_SIM::Project::SimulatorDef & simdef : m_project->m_simulators) {
				// look for existing block
				std::list<BLOCKMOD::Block>::iterator it;
				for (it = network.m_blocks.begin(); it != network.m_blocks.end(); ++it) {
					if (it->m_name.toStdString() == simdef.m_name)
						break;
				}
				if (it == network.m_blocks.end()) {
					int blockCount = network.m_blocks.size();
					BLOCKMOD::Block b( QString::fromStdString(simdef.m_name), BLOCKMOD::Globals::GridSpacing*blockCount,
									   BLOCKMOD::Globals::GridSpacing*blockCount);

					b.m_size = QSizeF(BLOCKMOD::Globals::GridSpacing*5,
								 BLOCKMOD::Globals::GridSpacing*10);
					checkedBlocks.push_back(b);
				}
				else {
					checkedBlocks.push_back(*it);
				}
			}
			// swap out block with checkedBlocks
			network.m_blocks.swap(checkedBlocks); // we now have as many blocks as simulators, and correct existing blocks are kept in sync with slaves
		}

		m_network = network; // data is copied into project's own network
		m_sceneManager->setNetwork(network); // data is copied into the scene manager
		m_lastReadTime = QFileInfo(fname).lastModified();

		// after reading the project file, we should update the views
		// this is done in a subsequent call to setModified() from the calling function
		return true;
	}
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
	}
	catch (std::exception & ex) {
		// this shouldn't happen, unless we have something weird going on
		IBK::IBK_Message(IBK::FormatString("std::exception caught: %1").arg(ex.what()), IBK::MSG_ERROR, FUNC_ID);
	}

	return false;
}


bool MSIMProjectHandler::write(const QString & fname) const {
	const char * const FUNC_ID = "[MSIMProjectHandler::write]";
	Q_ASSERT(isValid());

	// create file
	QFile file(fname);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		IBK::IBK_Message(IBK::FormatString("Cannot create/write file '%1' (path does not exists or missing permissions).")
						 .arg(fname.toStdString()), IBK::MSG_ERROR, FUNC_ID);
		return false;
	}
	file.close();

	try {

		// save thumbnail of project scematics
		QString thumbPath = MSIMDirectories::userDataDir()  + "/thumbs/" + QFileInfo(fname + ".png").fileName();
		QFileInfo thumbDir(thumbPath);
		if (!thumbDir.dir().exists())
			QDir().mkpath(thumbDir.dir().absolutePath());
		QPixmap p = m_sceneManager->generatePixmap(QSize(300,300));
		p.save(thumbPath);

		IBK::Path fpath = IBK::Path(fname.toStdString());
		// create a copy of the project file
		MASTER_SIM::Project pCopy = *m_project;

		// convert all slave references to relative file paths
		IBK::Path absoluteProjectFilePath = fpath.parentPath();
		for (unsigned int i=0; i<m_project->m_simulators.size(); ++i) {
			IBK::Path p = m_project->m_simulators[i].m_pathToFMU; // should be an absolute
			if (p.isAbsolute()) {
				try {
					IBK::Path relPath = p.relativePath(absoluteProjectFilePath);
					m_project->m_simulators[i].m_pathToFMU = relPath;
				}
				catch (...) {}
			}
		}
		m_project->write(fpath);

		// write network representation to file
		IBK::Path bmPath(fpath.withoutExtension().str() + ".bm");
		m_network.writeXML(QString::fromStdString(bmPath.str()));

		// and now restore filepath from copy
		for (unsigned int i=0; i<m_project->m_simulators.size(); ++i) {
			m_project->m_simulators[i].m_pathToFMU = pCopy.m_simulators[i].m_pathToFMU;
		}
		// also set the project file name
		m_projectFile = fname;
		*const_cast<QDateTime*>(&m_lastReadTime) = QFileInfo(fname).lastModified();
		return true;
	}
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
	}
	catch (std::exception & ex) {
		// this shouldn't happen, unless we have something weird going on
		IBK::IBK_Message(IBK::FormatString("std::exception caught: %1").arg(ex.what()), IBK::MSG_ERROR, FUNC_ID);
	}
	return false;
}


void MSIMProjectHandler::addToRecentFiles(const QString& fname) {

	MSIMSettings & si = MSIMSettings::instance();
	//qDebug() << si.m_recentProjects;

	// compose absolute file name
	QFileInfo finfo(fname);
	QString filePath =  finfo.absoluteFilePath();

	// check if recent project file is already in the list
	int i = si.m_recentProjects.indexOf(filePath);

	if (i != -1) {
		// already there, move it to front
		si.m_recentProjects.removeAt(i);
		si.m_recentProjects.push_front(filePath);
	}
	else {
		si.m_recentProjects.push_front(filePath);
		while (static_cast<unsigned int>(si.m_recentProjects.count()) > si.m_maxRecentProjects)
			si.m_recentProjects.pop_back();
	}

	// update recent project list
	emit updateRecentProjects();
}


