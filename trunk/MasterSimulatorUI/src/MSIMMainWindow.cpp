#include "MSIMMainWindow.h"
#include "ui_MSIMMainWindow.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QProcess> // for starting the external editor
#include <QFileInfo>
#include <QUndoStack>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QDockWidget>
#include <QSettings>
#include <QFile>
#include <QTimer>
#include <QTextStream>
#include <QDebug>

#include <numeric>

#include <IBK_FileUtils.h>
#include <IBK_messages.h>

#include <MSIM_Project.h>

#include "MSIMMessageHandler.h"
#include "MSIMUIConstants.h"
#include "MSIMSettings.h"
#include "MSIMWelcomeScreen.h"
#include "MSIMDirectories.h"
#include "MSIMLogWidget.h"
#include "MSIMUndoProject.h"
#include "MSIMConversion.h"
#include "MSIMPreferencesDialog.h"
#include "MSIMViewSlaves.h"
#include "MSIMViewConnections.h"

MSIMMainWindow * MSIMMainWindow::m_self = NULL;

MSIMMainWindow & MSIMMainWindow::instance() {
	Q_ASSERT_X(m_self != NULL, "[MSIMMainWindow::instance]",
		"You must not access MSIMMainWindow::instance() when the is no MSIMMainWindow "
		"instance (anylonger).");
	return *m_self;
}


void MSIMMainWindow::addUndoCommand(QUndoCommand * command) {
	MSIMMainWindow::instance().m_undoStack->push(command);
	// mark project as modified
	MSIMMainWindow::instance().updateWindowTitle();
}


#if QT_VERSION >= 0x050000
// Qt5 code
MSIMMainWindow::MSIMMainWindow(QWidget * /*parent*/, Qt::WindowFlags /*flags*/) :
#else
// Qt4 code
MSIMMainWindow::MSIMMainWindow(QWidget * /*parent*/, Qt::WFlags /*flags*/) :
#endif
	m_ui(new Ui::MSIMMainWindow),
	m_undoStack(new QUndoStack(this)),
	m_recentProjectsMenu(NULL),
	m_preferencesDialog(NULL),
	m_stackedWidget(NULL),
	m_viewSlaves(NULL),
	m_viewConnections(NULL)
{
//	const char * const FUNC_ID = "[MSIMMainWindow::MSIMMainWindow]";

	// store pointer to this object for global access
	m_self = this;

	m_ui->setupUi(this);


	// *** setup welcome widget ***

	QHBoxLayout * lay = new QHBoxLayout;
	m_welcomeScreen = new MSIMWelcomeScreen(this);
	lay->addWidget(m_welcomeScreen);
	lay->setMargin(0);
	lay->setSpacing(0);

	m_ui->centralWidget->setLayout(lay);
	m_welcomeScreen->updateWelcomePage();

	connect(m_welcomeScreen, SIGNAL(newProjectClicked()), this, SLOT(on_actionFileNew_triggered()));
	connect(m_welcomeScreen, SIGNAL(openProjectClicked()), this, SLOT(on_actionFileOpen_triggered()));
	connect(m_welcomeScreen, SIGNAL(openProject(QString)), this, SLOT(onOpenProjectByFilename(QString)));
	connect(m_welcomeScreen, SIGNAL(updateRecentList()), this, SLOT(onUpdateRecentProjects()));

	// *** create views ***

	m_stackedWidget = new QStackedWidget(this);
	lay->addWidget(m_stackedWidget);
	m_viewSlaves = new MSIMViewSlaves(this);
	m_stackedWidget->insertWidget(0, m_viewSlaves);
	m_viewConnections = new MSIMViewConnections(this);
	m_stackedWidget->insertWidget(1, m_viewConnections);

	// set FMU page as default
	m_ui->actionViewSimulators->setChecked(true);

	// *** connect to ProjectHandler signals ***

	connect(&m_projectHandler, SIGNAL(updateActions()), this, SLOT(onUpdateActions()));
	connect(&m_projectHandler, SIGNAL(updateRecentProjects()), this, SLOT(onUpdateRecentProjects()));


	// *** Setup log widget ***

	m_logWidget = new MSIMLogWidget(this);
	MSIMMessageHandler * msgHandler = dynamic_cast<MSIMMessageHandler *>(IBK::MessageHandlerRegistry::instance().messageHandler());
	connect(msgHandler, SIGNAL(msgReceived(int,QString)), m_logWidget, SLOT(onMsgReceived(int, QString)));
	m_logWidget->setVisible(false);


	// *** create menu for recent files ***

	m_recentProjectsMenu = new QMenu(this);
	m_ui->actionFileRecentProjects->setMenu(m_recentProjectsMenu);
	onUpdateRecentProjects();


	// *** add actions for undo and redo ***

	QAction * undoAction = m_undoStack->createUndoAction(this, tr("Undo"));
	undoAction->setIcon(QIcon(":/gfx/actions/24x24/undo.png"));
	undoAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Z));
	QAction * redoAction = m_undoStack->createRedoAction(this, tr("Redo"));
	redoAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Z));
	redoAction->setIcon(QIcon(":/gfx/actions/24x24/redo.png"));

	// this is a bit messy, but there seems to be no other way, unless we create the whole menu ourselves
	QList<QAction*> acts = m_ui->menu_Edit->actions();
	m_ui->menu_Edit->addAction(undoAction);
	m_ui->menu_Edit->addAction(redoAction);
	for (int i=0; i<acts.count(); ++i)
		m_ui->menu_Edit->addAction(acts[i]);

	m_ui->toolBar->addAction(undoAction);
	m_ui->toolBar->addAction(redoAction);

	// *** restore state of UI ***
	QByteArray geometry, state;
	MSIMSettings::instance().readMainWindowSettings(geometry,state);
	if (!state.isEmpty())
		restoreState(state);
	if (!geometry.isEmpty())
		restoreGeometry(geometry);


	// *** update actions/UI State depending on project ***
	onUpdateActions();


	// *** Populate language menu ***
	addLanguageAction("en", "English");
	addLanguageAction("de", "Deutsch");


	// *** read last loaded project/project specified on command line ***

	if (!MSIMSettings::instance().m_initialProjectFile.isEmpty()) {
		// try to load the project - silently
		m_projectHandler.loadProject(this, MSIMSettings::instance().m_initialProjectFile, false);
		if (m_projectHandler.isValid())
			saveThumbNail();
	}

}


MSIMMainWindow::~MSIMMainWindow() {
	delete m_ui;
	delete m_undoStack;

	// kill any thread that still runs

	m_self = NULL;
}


bool MSIMMainWindow::saveProject() {
	on_actionFileSave_triggered();
	return	!MSIMProjectHandler::instance().isModified() &&
			!MSIMProjectHandler::instance().projectFile().isEmpty();
}


void MSIMMainWindow::changeEvent(QEvent *event) {
	if (event->type() == QEvent::ActivationChange && this->isActiveWindow()){
		if (MSIMProjectHandler::instance().isValid() && !MSIMProjectHandler::instance().projectFile().isEmpty()) {
			// check for externally modified project file and trigger "reload" action
			if (QFileInfo(MSIMProjectHandler::instance().projectFile()).lastModified() >
				MSIMProjectHandler::instance().lastReadTime())
			{
				int res = QMessageBox::question(this, tr("Reload project file"),
									  tr("The project file has been modified by an external application. "
										 "When reloading this project file all unsaved changes will be lost. "
										 "Reload modified project file? "), QMessageBox::Yes | QMessageBox::No);
				if (res == QMessageBox::Yes) {
					QMainWindow::changeEvent(event);
					// reload project
					m_projectHandler.reloadProject(this);

					if (m_projectHandler.isValid())
						saveThumbNail();
					return;
				}
				else {
					MSIMProjectHandler::instance().updateLastReadTime();
				}
			}
		}
	}
	QMainWindow::changeEvent(event);
}


void MSIMMainWindow::closeEvent(QCloseEvent * event) {
//	const char * const FUNC_ID = "[MSIMMainWindow::closeEvent]";
	// move input focus away from any input fields (to allow editingFinished() events to fire)
	setFocus();

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

	// save user config and recent file list
	MSIMSettings::instance().write(saveGeometry(), saveState());

	event->accept();
}


void MSIMMainWindow::on_actionFileNew_triggered() {
	// move input focus away from any input fields (to allow editingFinished() events to fire)
	setFocus();
	// close project if we have one
	if (!m_projectHandler.closeProject(this)) // emits updateActions() if project was closed
		return;

	// create new project
	m_projectHandler.newProject(this); // emits updateActions()
}


void MSIMMainWindow::on_actionFileOpen_triggered() {
	// move input focus away from any input fields (to allow editingFinished() events to fire)
	setFocus();
	// close project if we have one
	if (!m_projectHandler.closeProject(this)) // emits updateActions() if project was closed
		return;

	// request file name
	QString filename = QFileDialog::getOpenFileName(
							this,
							tr("Select project file"),
							MSIMSettings::instance().m_propertyMap[MSIMSettings::PT_LastFileOpenDirectory].toString(),
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
	if (m_projectHandler.isValid()) {
		saveThumbNail();
		MSIMSettings::instance().m_propertyMap[MSIMSettings::PT_LastFileOpenDirectory] = QFileInfo(filename).absoluteDir().absolutePath();
	}
}


void MSIMMainWindow::on_actionFileSave_triggered() {
	// move input focus away from any input fields (to allow editingFinished() events to fire)
	setFocus();
	// check if we have a file name
	if (m_projectHandler.projectFile().isEmpty()) {
		m_projectHandler.saveWithNewFilename(this);
	}
	else {
		// Note: should not be possible of project hasn't been modified
		m_projectHandler.saveProject(this, m_projectHandler.projectFile()); // emits updateActions() if project was successfully saved
	}
	saveThumbNail();
}


void MSIMMainWindow::on_actionFileSaveAs_triggered() {
	// move input focus away from any input fields (to allow editingFinished() events to fire)
	setFocus();
	m_projectHandler.saveWithNewFilename(this); // emits updateActions() if project was successfully saved
	saveThumbNail();
}


void MSIMMainWindow::on_actionFileReload_triggered() {
	// move input focus away from any input fields (to allow editingFinished() events to fire)
	setFocus();
	// if project has not yet been saved, it cannot be reloaded
	if (m_projectHandler.projectFile().isEmpty()) {
		QMessageBox::information(this, tr("Reload project"), tr("The project has not yet been saved."));
		return;
	}
	// if modified, ask user to confirm loosing changes
	if (m_projectHandler.isModified()) {
		if (QMessageBox::question(this, tr("Reload project"), tr("The project has been modified. Discard those changes?"),
								  QMessageBox::Discard | QMessageBox::Abort) == QMessageBox::Abort)
		{
			return;
		}
	}
	// reload project
	m_projectHandler.reloadProject(this);

	if (m_projectHandler.isValid())
		saveThumbNail();
}


void MSIMMainWindow::on_actionFileExport_triggered() {
	// project must have been saved once already
	if (m_projectHandler.projectFile().isEmpty() || m_projectHandler.isModified()) {
		QMessageBox::information(this, tr("Project file must be saved"),
								 tr("The project must be saved before it can be exported."));
		on_actionFileSave_triggered();
		// no save, no export
		if (m_projectHandler.projectFile().isEmpty() || m_projectHandler.isModified()) {
			return;
		}
	}

	// request export directory
	QFileInfo finfo(m_projectHandler.projectFile());
	QString fnameSuggestion = finfo.absoluteDir().absolutePath() + "/" + finfo.baseName() + ".d6pp";
	QString filename = QFileDialog::getSaveFileName(
							this,
							tr("Specify MSIMphin project package file"),
							fnameSuggestion,
							tr("MSIMphin Project Packages (*.d6pp);;All files (*.*)"));

	if (filename.isEmpty())
		return;

	// ensure that we have the proper extension
	if (!filename.endsWith(".d6pp")) {
		filename.append(".d6pp");
	}

	QString dirName = QFileInfo(filename).baseName();
	if (dirName.isEmpty()) {
		QMessageBox::critical(this, tr("Invalid file name"), tr("Please enter a valid file name!"));
		return;
	}

	exportProjectPackage(filename);
}


void MSIMMainWindow::on_actionFileClose_triggered() {
	// move input focus away from any input fields (to allow editingFinished() events to fire)
	setFocus();
	m_projectHandler.closeProject(this);
}


void MSIMMainWindow::on_actionFileQuit_triggered() {
	close();
}


void MSIMMainWindow::on_actionEditTextEditProject_triggered() {
	// check if editor has been set in preferences
	if (MSIMSettings::instance().m_textEditorExecutable.isEmpty()) {
		QMessageBox::critical(this, tr("Missing user preferences"), tr("Please open the preferences dialog and specify "
																	   "a text editor first!"));
		return;
	}

	bool res = QProcess::startDetached( MSIMSettings::instance().m_textEditorExecutable, QStringList() << m_projectHandler.projectFile() );
	if (!res) {
		QMessageBox::critical(this, tr("Error starting external application"), tr("Text editor '%1' could not be started.")
							  .arg(MSIMSettings::instance().m_textEditorExecutable));
	}

}


void MSIMMainWindow::on_actionEditPreferences_triggered() {
	// spawn preferences dialog
	if (m_preferencesDialog == NULL)
		m_preferencesDialog = new MSIMPreferencesDialog(this);

	if (m_preferencesDialog->edit()) {
		/// \todo update views based on prefs
	}
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
		onOpenProjectByFilename(fname);
	}
}


void MSIMMainWindow::onActionSwitchLanguage() {
	QAction * a = (QAction *)sender();
	QString langId = a->data().toString();
	MSIMSettings::instance().m_langId = langId;
	QMessageBox::information(this, tr("Languange changed"), tr("Please restart the software to activate the new language!"));
}



void MSIMMainWindow::onUpdateActions() {
	// do we have a project?
	bool have_project = m_projectHandler.isValid();
	// enable/disable all actions that require a project

	// *** Project-dependent actions ***

	m_ui->actionFileSave->setEnabled(have_project);
	m_ui->actionFileSaveAs->setEnabled(have_project);
	m_ui->actionFileExport->setEnabled(have_project);
	m_ui->actionFileReload->setEnabled(have_project);
	m_ui->actionFileClose->setEnabled(have_project);
	m_ui->actionEditTextEditProject->setEnabled(have_project);

	// no project, no undo actions -> clearing undostack also disables undo actions
	if (!have_project)
		m_undoStack->clear();

	// *** View configuration ***

	// show welcome page only when we have no project
	m_welcomeScreen->setVisible(!have_project);
	// toolbar is only visible when we have a project
	m_ui->toolBar->setVisible(have_project);
	// views are only visible when we have a project
	m_stackedWidget->setVisible(have_project);

	// also update window caption and status bar
	if (have_project) {
		updateWindowTitle();
	}
	else {
		setWindowTitle(PROGRAM_VERSION_NAME());
		m_welcomeScreen->updateWelcomePage();
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
		m_ui->actionFileRecentProjects->setEnabled(false);
		return;
	}
	else {
		m_ui->actionFileRecentProjects->setEnabled(true);
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


void MSIMMainWindow::onOpenProjectByFilename(const QString & filename) {
	QFile f1(filename);
	if (!f1.exists()) {
		QMessageBox::critical(this, tr("File not found"), tr("The file '%1' cannot be found or does not exist.").arg(filename));
		return;
	}
	// move input focus away from any input fields (to allow editingFinished() events to fire)
	setFocus();
	// we first need to close the current project
	if (!m_projectHandler.closeProject(this)) return;
	// then create a new project and try to load the file
	m_projectHandler.loadProject(this, filename, false);
	if (m_projectHandler.isValid())
		saveThumbNail();
	// if failed, no view state change needed
}



void MSIMMainWindow::updateWindowTitle() {
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

bool MSIMMainWindow::removeDirRecursively(const QString & dirName) {
	bool result = true;
	QDir dir(dirName);

	if (dir.exists(dirName)) {
		Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
			if (info.isDir()) {
				result = removeDirRecursively(info.absoluteFilePath());
			}
			else {
				result = QFile::remove(info.absoluteFilePath());
			}

			if (!result) {
				return result;
			}
		}
		result = dir.rmdir(dirName);
	}
	return result;
}


bool MSIMMainWindow::exportProjectPackage(const QString & exportFilePath) {
	return true;
}


bool MSIMMainWindow::importProjectPackage(const QString & packageFilePath, const QString & targetDirectory,
										 QString & projectFilePath, bool packageContainsMSIM)
{
	return true;
}


void MSIMMainWindow::saveThumbNail() {
	// compose temporary file path
	QString thumbPath = MSIMDirectories::userDataDir()  + "/thumbs/" + QFileInfo(m_projectHandler.projectFile() + ".png").fileName();
	QFileInfo finfo(thumbPath);
	if (finfo.exists()) {
		// only update thumbnail if project file is newer than thumbnail file
		QFileInfo prjFInfo(m_projectHandler.projectFile());
		if (finfo.lastModified() >= prjFInfo.lastModified())
			return;
	}

	// \todo render preview image
}


void MSIMMainWindow::addLanguageAction(const QString &langId, const QString &actionCaption) {
	QString languageFilename = QString("%1/MasterSimulatorUI_%3.qm").arg(MSIMDirectories::translationsDir()).arg(langId);
	if (langId == "en" || QFile(languageFilename).exists()) {
		QAction * a = new QAction(actionCaption, this);
		a->setData(langId);
		a->setIcon( QIcon( QString(":/gfx/icons/%1.png").arg(langId)) );
		connect(a, SIGNAL(triggered()),
				this, SLOT(onActionSwitchLanguage()));
		m_ui->menuLanguage->insertAction(NULL, a);
	}
	else {
		IBK::IBK_Message( IBK::FormatString("Language file '%1' missing.").arg(languageFilename.toUtf8().data()),
						  IBK::MSG_WARNING, "[MSIMMainWindow::addLanguageAction]");
	}
}


void MSIMMainWindow::on_actionViewSimulators_toggled(bool arg1) {
	// toggle off all other view actions
	m_ui->actionViewConnections->blockSignals(true);
	m_ui->actionViewConnections->setChecked(false);
	m_ui->actionViewConnections->blockSignals(false);
	m_stackedWidget->setCurrentIndex(0);
}

void MSIMMainWindow::on_actionViewConnections_toggled(bool arg1) {
	m_ui->actionViewSimulators->blockSignals(true);
	m_ui->actionViewSimulators->setChecked(false);
	m_ui->actionViewSimulators->blockSignals(false);
	m_stackedWidget->setCurrentIndex(1);
}
