#include "MSIMMainWindow.h"
#include "ui_MSIMMainWindow.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QProcess> // for starting the external editor
#include <QFileInfo>
#include <QUndoStack>
#include <QFileDialog>

#include "MSIMUIConstants.h"
#include "MSIMSettings.h"


MSIMMainWindow * MSIMMainWindow::m_self = NULL;

MSIMMainWindow & MSIMMainWindow::instance() {
	Q_ASSERT_X(m_self != NULL, "[MSIMMainWindow::instance]",
		"You must not access MSIMMainWindow::instance() when the is no MSIMMainWindow "
		"instance (anylonger).");
	return *m_self;
}


void MSIMMainWindow::addUndoCommand(QUndoCommand * command) {
	MSIMMainWindow::instance().m_undoStack->push(command);
}


#if QT_VERSION >= 0x050000
// Qt5 code
MSIMMainWindow::MSIMMainWindow(QWidget *parent, Qt::WindowFlags flags) :
#else
// Qt4 code
MSIMMainWindow::MSIMMainWindow(QWidget * /*parent*/, Qt::WFlags /*flags*/) :
#endif
	ui(new Ui::MSIMMainWindow),
	m_undoStack(new QUndoStack(this)),
	m_recentProjectsMenu(NULL)
{
	// store pointer to this object for global access
	m_self = this;

	ui->setupUi(this);


	// *** connect to ProjectHandler signals ***

	connect(&m_projectHandler, SIGNAL(updateActions()), this, SLOT(onUpdateActions()));
	connect(&m_projectHandler, SIGNAL(updateRecentProjects()), this, SLOT(onUpdateRecentProjects()));


	// *** create menu for recent files ***

	m_recentProjectsMenu = new QMenu(this);
	ui->actionFileRecentProjects->setMenu(m_recentProjectsMenu);
	onUpdateRecentProjects();


	// *** add actions for undo and redo ***

	QAction * undoAction = m_undoStack->createUndoAction(this, tr("Undo"));
	undoAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Z));
	QAction * redoAction = m_undoStack->createRedoAction(this, tr("Redo"));
	redoAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Z));

	// this is a bit messy, but there seems to be no other way, unless we create the whole menu ourselves
	QList<QAction*> acts = ui->menu_Edit->actions();
	ui->menu_Edit->addAction(undoAction);
	ui->menu_Edit->addAction(redoAction);
	for (int i=0; i<acts.count(); ++i)
		ui->menu_Edit->addAction(acts[i]);


	// *** read last loaded project/project specified on command line ***
	if (!MSIMSettings::instance().m_initialProjectFile.isEmpty()) {
		// try to load the project - silently
		m_projectHandler.loadProject(this, MSIMSettings::instance().m_initialProjectFile, false);
	}

	// update actions/UI State depending on project
	onUpdateActions();
}


MSIMMainWindow::~MSIMMainWindow() {
	delete ui;
}


void MSIMMainWindow::closeEvent(QCloseEvent * event) {

	// remember current project
	if ( m_projectHandler.isValid() ) {
		// make sure we have saved and closed our project
		if (!m_projectHandler.closeProject(this)) {
			// user must have canceled, so prevent closing of the application
			event->ignore();
			return;
		}
		MSIMSettings::instance().m_lastProjectFile = m_projectHandler.projectFile();
	}
	else {
		MSIMSettings::instance().m_lastProjectFile.clear();
	}

	MSIMSettings::instance().m_xSizeAtProgrammClose = width();
	MSIMSettings::instance().m_ySizeAtProgrammClose = height();

	// save user config and recent file list
	MSIMSettings::instance().write();

	event->accept();
}


void MSIMMainWindow::on_actionFileNew_triggered() {
	// close project if we have one
	if (!m_projectHandler.closeProject(this)) // emits updateActions() if project was closed
		return;

	// create new project
	m_projectHandler.newProject(); // emits updateActions()
}


void MSIMMainWindow::on_actionFileOpen_triggered() {
	// close project if we have one
	if (!m_projectHandler.closeProject(this)) // emits updateActions() if project was closed
		return;

	// request file name
	QString filename = QFileDialog::getOpenFileName(
							this,
							tr("Select project file"),
							QString(),
							tr("Projects (*%1);;All files (*.*)").arg(DOT_FILE_EXTENSION)
						);

	if (filename.isEmpty()) return;

	QFile f1(filename);
	if (!f1.exists()) {
		QMessageBox::critical(
					this,
					tr("File not found"),
					tr("The file '%1' does not exist or cannot be accessed.").arg(filename)
			);
		return;
	}

	m_projectHandler.loadProject(this, filename, false); // emits updateActions() if project was successfully loaded
}


void MSIMMainWindow::on_actionFileSave_triggered() {
	// check if we have a file name
	if (m_projectHandler.projectFile().isEmpty()) {
		m_projectHandler.saveWithNewFilename(this);
	}
	else {
		// Note: should not be possible of project hasn't been modified
		m_projectHandler.saveProject(this, m_projectHandler.projectFile()); // emits updateActions() if project was successfully saved
	}
}


void MSIMMainWindow::on_actionFileSaveAs_triggered() {
	m_projectHandler.saveWithNewFilename(this); // emits updateActions() if project was successfully saved
}


void MSIMMainWindow::on_actionFileExport_triggered() {
	// TODO : add ssp-format-export
}


void MSIMMainWindow::on_actionFileClose_triggered() {
	m_projectHandler.closeProject(this);
}



void MSIMMainWindow::on_actionFileQuit_triggered() {
	close();
}


void MSIMMainWindow::on_actionHelpAboutQt_triggered() {
	QMessageBox::aboutQt(this, tr("About Qt..."));
}


void MSIMMainWindow::on_actionHelpAboutMasterSim_triggered() {
	QMessageBox::about(this, tr("About MasterSim"),
	tr("This is the placeholder for the MasterSim aboutbox, see https://sourceforge.net/projects/mastersim."));
}


void MSIMMainWindow::onActionOpenRecentFile() {
	QAction *action = qobject_cast<QAction *>(sender());
	if (action) {
		// before closing the project, check if the project file exists
		QString fname = action->data().toString();
		QFile f1(fname);
		if (!f1.exists()) {
			QMessageBox::critical(this, tr("File not found"), tr("The file '%1' cannot be found or does not exist.").arg(fname));
			return;
		}
		// we first need to close the current project
		if (!m_projectHandler.closeProject(this)) return;
		// then create a new project and try to load the file
		m_projectHandler.loadProject(this, fname, false);
		// if failed, no view state change needed
	}
}


void MSIMMainWindow::onActionSwitchLanguage() {
}


void MSIMMainWindow::onUpdateActions() {
	// do we have a project?
	bool have_project = m_projectHandler.isValid();
	// enable/disable all actions that require a project
	ui->actionFileSave->setEnabled(have_project);
	ui->actionFileSaveAs->setEnabled(have_project);
	ui->actionFileExport->setEnabled(have_project);
	ui->actionFileClose->setEnabled(have_project);
	ui->actionEditTextEditProject->setEnabled(have_project);
	// also update window caption and status bar
	if (have_project) {
		// no project file given?
		QString shortFileName, longFileName;
		if (m_projectHandler.projectFile().isEmpty()) {
			shortFileName = tr("unnamed%1").arg(DOT_FILE_EXTENSION);
			longFileName = shortFileName;
		}
		else {
			shortFileName = QFileInfo(m_projectHandler.projectFile()).fileName();
			longFileName = m_projectHandler.projectFile();
		}
		if (m_projectHandler.isModified())
			shortFileName += "*";
		setWindowTitle(QString("%1 - %2").arg(PROGRAM_VERSION_NAME()).arg(shortFileName));
	}
	else {
		setWindowTitle(PROGRAM_VERSION_NAME());
	}
}


void MSIMMainWindow::onUpdateRecentProjects() {
	// create actions for recent files if number of max. recent projects in settings
	// differs from current number
	if (m_recentProjectActions.count() != (int)MSIMSettings::instance().m_maxRecentProjects) {
		qDeleteAll(m_recentProjectActions);
		m_recentProjectActions.clear();
		for (unsigned int i = 0; i < MSIMSettings::instance().m_maxRecentProjects; ++i) {
			QAction * a = new QAction(this);
			m_recentProjectActions.push_back(a);
			connect(m_recentProjectActions[i], SIGNAL(triggered()), this, SLOT(onActionOpenRecentFile()));
			m_recentProjectsMenu->addAction(m_recentProjectActions[i]);
		}
	}

	// disable recent file actions
	if (MSIMSettings::instance().m_recentProjects.isEmpty()) {
		ui->actionFileRecentProjects->setEnabled(false);
		return;
	}
	else {
		ui->actionFileRecentProjects->setEnabled(true);
		for ( int i = 0, count = MSIMSettings::instance().m_recentProjects.count(); i < count; ++i) {
			m_recentProjectActions[i]->setText(MSIMSettings::instance().m_recentProjects[i]);
			m_recentProjectActions[i]->setData(MSIMSettings::instance().m_recentProjects[i]);
			m_recentProjectActions[i]->setVisible(true);
		}

		for (unsigned int i = MSIMSettings::instance().m_recentProjects.count();
			i < MSIMSettings::instance().m_maxRecentProjects; ++i)
		{
			m_recentProjectActions[i]->setVisible(false);
		}
	}
}
