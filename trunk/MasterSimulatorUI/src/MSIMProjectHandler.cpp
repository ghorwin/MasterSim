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

MSIMProjectHandler * MSIMProjectHandler::m_self = NULL;

MSIMProjectHandler & MSIMProjectHandler::instance() {
	Q_ASSERT_X(m_self != NULL, "[MSIMProjectHandler::instance]",
		"You must not access MSIMProjectHandler::instance() when the is no MSIMProjectHandler "
		"instance (anylonger).");
	return *m_self;
}


MSIMProjectHandler::MSIMProjectHandler() :
	m_project(NULL),
	m_sceneManager(NULL),
	m_modified(false)
{
	IBK_ASSERT(m_self == NULL);
	m_self = this;

	// initialization of other globals
	BLOCKMOD::Globals::GridSpacing = 16;
	BLOCKMOD::Globals::LabelFontSize = 8;
}


MSIMProjectHandler::~MSIMProjectHandler( ){
	// free owned project, if any
	delete m_project;
	delete m_sceneManager;
	m_self = NULL;
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
		IBK::Path absoluteProjectFilePath = IBK::Path(m_projectFile.toUtf8().data()).parentPath();
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
			tr("Projects (*%1);;All files (*.*)").arg(DOT_FILE_EXTENSION)
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
		m_project->m_created = QDateTime::currentDateTime().toString(Qt::TextDate).toUtf8().data();
	m_project->m_lastEdited = QDateTime::currentDateTime().toString(Qt::TextDate).toUtf8().data();

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
		emit modified(ProjectPathModified, NULL);
	}

	// clear modified flag
	m_modified = false;
	// signal UI to update project status
	emit updateActions();

	// add project file name to recent file list
	addToRecentFiles(fname);

	return SaveOK; // saving succeeded
}


void MSIMProjectHandler::setModified(int modificationType, void * data) {
	// special case:  modification type = NotModified
	ModificationTypes modType = static_cast<ModificationTypes>(modificationType);
	switch (modType) {

		default: ; // skip all others
	}
	m_modified = true;

	emit modified(modificationType, data);
}


const MASTER_SIM::Project & MSIMProjectHandler::project() const {
	const char * const FUNC_ID = "[MSIMProjectHandler::project]";

	if (m_project == NULL)
		throw IBK::Exception("Must not call project() on invalid ProjectHandler.", FUNC_ID);
	return *m_project;
}


void MSIMProjectHandler::updateLastReadTime() {
	const char * const FUNC_ID = "[MSIMProjectHandler::updateLastReadTime]";
	if (!isValid())
		throw IBK::Exception("Must not call updateLastReadTime() on invalid project.", FUNC_ID);
	m_lastReadTime = QFileInfo(projectFile()).lastModified();
}


// *** PRIVATE MEMBER FUNCTIONS ***

void MSIMProjectHandler::createProject() {
	Q_ASSERT(m_project == NULL);

	m_project = new MASTER_SIM::Project;
	m_sceneManager = new MSIMSceneManager;
	m_projectFile.clear();
	m_modified = false; // new projects are never modified
}


void MSIMProjectHandler::destroyProject() {
	Q_ASSERT(m_project != NULL);

	delete m_project;
	delete m_sceneManager;
	m_project = NULL;
	m_sceneManager = NULL;
	m_projectFile.clear();
}


bool MSIMProjectHandler::read(const QString & fname) {
	const char * const FUNC_ID = "[MSIMProjectHandler::read]";

	// check that we have a project, should be newly created
	Q_ASSERT(isValid());

	if (!QFileInfo(fname).exists()) {
		IBK::IBK_Message(IBK::FormatString("File '%1' does not exist or permissions are missing for accessing the file.")
						 .arg(fname.toUtf8().data()), IBK::MSG_ERROR, FUNC_ID);
		return false;
	}

	try {

		// filename is converted to utf8 before calling readXML
		IBK::Path fpath(fname.toUtf8().data());
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
			for (BLOCKMOD::Block & b : network.m_blocks) {
				std::vector<MASTER_SIM::Project::SimulatorDef>::const_iterator it;
				for ( it = m_project->m_simulators.begin(); it != m_project->m_simulators.end(); ++it) {
					if (it->m_name == b.m_name.toStdString())
						break;
				}
				if (it == m_project->m_simulators.end()) {
					IBK::IBK_Message(IBK::FormatString("Invalid block in network representation file '%1'.")
									 .arg(bmPath), IBK::MSG_ERROR, FUNC_ID);
					network = BLOCKMOD::Network();
				}
			}
			// add dummy blocks for each simulator, that is not yet in the network
			for (MASTER_SIM::Project::SimulatorDef & simdef : m_project->m_simulators) {
				// look for existing block
				QList<BLOCKMOD::Block>::iterator it;
				for (it = network.m_blocks.begin(); it != network.m_blocks.end(); ++it) {
					if (it->m_name.toStdString() == simdef.m_name)
						break;
				}
				if (it == network.m_blocks.end()) {
					int blockCount = network.m_blocks.count();
					BLOCKMOD::Block b( QString::fromStdString(simdef.m_name), BLOCKMOD::Globals::GridSpacing*blockCount,
									   BLOCKMOD::Globals::GridSpacing*blockCount);

					b.m_size = QSizeF(BLOCKMOD::Globals::GridSpacing*5,
								 BLOCKMOD::Globals::GridSpacing*10);
					network.m_blocks.append(b);
				}
			}
		}

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
						 .arg(fname.toUtf8().data()), IBK::MSG_ERROR, FUNC_ID);
		return false;
	}
	file.close();

	try {
		// filename is converted to utf8 before calling write
		IBK::Path fpath = IBK::Path(fname.toUtf8().data());

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
		m_sceneManager->network().writeXML(QString::fromStdString(bmPath.str()));

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


