#include "MSIMProjectHandler.h"

#include <QStringList>
#include <QString>
#include <QMessageBox>
#include <QFileDialog>

#include <QDebug>

#include <IBK_Exception.h>
#include <IBK_FileUtils.h>
#include <IBK_assert.h>

#include "MSIMSettings.h"
#include "MSIMUIConstants.h"
#include "MSIMDirectories.h"

MSIMProjectHandler::MSIMProjectHandler() :
	m_project(NULL),
	m_modified(false)
{
}


MSIMProjectHandler::~MSIMProjectHandler( ){
	// free owned project, if any
	delete m_project;
}


void MSIMProjectHandler::newProject() {
	createProject();

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

	// we must not have a project loaded
	IBK_ASSERT(!isValid());

	// create a new project
	createProject();

	if (!read(fileName)) {
		if (!silent) {

			QMessageBox::critical(
					parent,
					tr("Error loading project"),
					tr("Error loading project file '%1', see error log file '%2' for details.")
						.arg(fileName)
						.arg(MSIMDirectories::globalLogFile())
			);

		}
		// remove project again
		destroyProject();

		// Note: no need to emit updateActions() here since view state hasn't finished.
		return;
	}
	// project read successfully


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
					tr("Error loading project."),
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

	// relay to saveProject() which updates modified flag and emits corresponding signals.
	if (saveProject(parent, filename) != SaveOK)
		return SaveFailed; // saving failed

	return SaveOK;
}


MSIMProjectHandler::SaveResult MSIMProjectHandler::saveProject(QWidget * parent, const QString & fileName) {

	// check project file ending, if there is none append it
	QString fname = fileName;
	if (!fname.endsWith(DOT_FILE_EXTENSION))
		fname.append(DOT_FILE_EXTENSION);

	// save project file
	if (!write(fname)) {

		QMessageBox::critical(
				parent,
				tr("Saving failed"),
				tr("Error while saving project file, see error log file '%1' for details.").arg(MSIMDirectories::globalLogFile())
				);

		return SaveFailed;
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
	DefaultModificationTypes modType = static_cast<DefaultModificationTypes>(modificationType);
	switch (modType) {

		default: ; // skip all others
	}
	m_modified = true;

	emit modified(modificationType, data);
}


// *** PRIVATE MEMBER FUNCTIONS ***

void MSIMProjectHandler::createProject() {
	Q_ASSERT(m_project == NULL);

	m_project = new MASTER_SIM::Project;
	m_projectFile.clear();
	m_modified = false; // new projects are never modified
}


void MSIMProjectHandler::destroyProject() {
	Q_ASSERT(m_project != NULL);

	delete m_project;
	m_project = NULL;
	m_projectFile.clear();
}


bool MSIMProjectHandler::read(const QString & fname) {
	const char * const FUNC_ID = "[MSIMProjectHandler::read]";

	// check that we have a project, should be newly created
	Q_ASSERT(isValid());

	if (!QFileInfo(fname).exists()) {
		IBK::IBK_Message(IBK::FormatString("File '%1' does not exist or permissions are missing for accessing the file.")
						 .arg(IBK::Path(fname.toUtf8().data())), IBK::MSG_ERROR, FUNC_ID);
		return false;
	}

	try {

		m_project->read(IBK::Path(fname.toUtf8().data()));
		m_projectFile = fname;

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
						 .arg(IBK::Path(fname.toUtf8().data())), IBK::MSG_ERROR, FUNC_ID);
		return false;
	}
	file.close();

	try {
		m_project->write(IBK::Path(fname.toUtf8().data()));

		// also set the project file name
		m_projectFile = fname;
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

	// check if recent project file is already in the list
	int i = si.m_recentProjects.indexOf(fname);

	if (i != -1) {
		// already there, move it to front
		si.m_recentProjects.removeAt(i);
		si.m_recentProjects.push_front(fname);
	}
	else {
		si.m_recentProjects.push_front(fname);
		while (static_cast<unsigned int>(si.m_recentProjects.count()) > si.m_maxRecentProjects)
			si.m_recentProjects.pop_back();
	}

	// update recent project list
	emit updateRecentProjects();
}

