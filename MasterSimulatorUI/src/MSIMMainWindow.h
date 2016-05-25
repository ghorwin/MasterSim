#ifndef MSIMMainWindowH
#define MSIMMainWindowH

#include <QMainWindow>
#include <QUndoStack>

#include "MSIMProjectHandler.h"

namespace Ui {
	class MSIMMainWindow;
}

/*! Main window class. */
class MSIMMainWindow : public QMainWindow {
	Q_OBJECT

public:

	/*! Returns a pointer to the MSIMMainWindow instance.
		Only access this function during the lifetime of the
		MSIMMainWindow instance.
	*/
	static MSIMMainWindow & instance();

	/*! Adds an undo command to the global undo stack.
		Ownership of the command object will be transferred to the stack.
	*/
	static void addUndoCommand(QUndoCommand * command);


	/*! Default MSIMMainWindow constructor. */
#if QT_VERSION >= 0x050000
// Qt5 code
	explicit MSIMMainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
#else
// Qt4 code
	explicit MSIMMainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
#endif

	/*! Default destructor. */
	~MSIMMainWindow();

protected:
	/*! Does the confirm-saving-before-close stuff. */
	void closeEvent(QCloseEvent * event);

private slots:
	void on_actionFileNew_triggered();
	void on_actionFileOpen_triggered();
	void on_actionFileSave_triggered();
	void on_actionFileSaveAs_triggered();
	void on_actionFileExport_triggered();
	void on_actionFileClose_triggered();
	void on_actionFileQuit_triggered();

	void on_actionHelpAboutQt_triggered();
	void on_actionHelpAboutMasterSim_triggered();

	/*! Triggered when a recent file menu entry was clicked. */
	void onActionOpenRecentFile();

	/*! Triggered when a language menu entry was clicked. */
	void onActionSwitchLanguage();

	/*! Updates the state of all actions based on the current condition of the project.
		This slot is connected to the signal updateActions() from ProjectHandler.
	*/
	void onUpdateActions();

	/*! Updates the menu entries in the 'Recent Projects' submenu.
		This is a slot because we need to update the menu with the actions
		a bit delayed. When the user clicks on a recent project menu entry, the
		loadProject() function is indirectly called which in turn calls
		updateRecentProjects(). Since the menu actions in the recent projects
		menu are deleted, this would mean that the action currently process is
		being deleted - causing a crash. Therefore we don't call updateRecentProjects()
		directly, but via a QTimer::singleShot() and thus ensure that the
		action handler function is completed before the action is touched.
	*/
	void onUpdateRecentProjects();

private:
	/*! Global pointer to main window instance.
		Initialized in the constructor of MSIMMainWindow and
		reset to NULL in the destructor. So be sure that the main window
		exists before accessing MSIMMainWindow::instance()
	*/
	static MSIMMainWindow		*m_self;


	/*! Main user interface pointer. */
	Ui::MSIMMainWindow			*ui;
	/*! The global undo stack in the program. */
	QUndoStack					*m_undoStack;
	/*! Menu for the recent projects entries. */
	QMenu						*m_recentProjectsMenu;
	/*! List with action objects for each recent project in the main menu. */
	QList<QAction*>				m_recentProjectActions;
	/*! List of language actions shown in the languages menu. */
	QList<QAction*>				m_languageActions;
	/*! The project handler that manages the actual project. */
	MSIMProjectHandler			m_projectHandler;
};

#endif // MSIMMainWindowH
