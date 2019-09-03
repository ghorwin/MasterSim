#ifndef MSIMProjectHandlerH
#define MSIMProjectHandlerH

#include <QObject>
#include <QCoreApplication>
#include <QString>
#include <QDateTime>

#include <MSIM_Project.h>

namespace BLOCKMOD {
	class Network;
};

/*! Utility class that manages common project-related functions, in particular
	modification state setting, project file reading/writing and other.
	A project handler maintains a project instance with all project-related data.

	It provides GUI functionality like loadProject(), saveProject() and
	saveWithNewFilename(). These functions require a parent widget as function
	argument.
*/
class MSIMProjectHandler : public QObject {
	Q_OBJECT
	Q_DISABLE_COPY(MSIMProjectHandler)
public:

	/*! Returns singleton instance of project handler. */
	static MSIMProjectHandler & instance();

	/*! Modification types used in undo actions.
		\see setModified()
	*/
	enum ModificationTypes {
		/*! When project file path has changed. */
		ProjectPathModified,
		/*! Used whenever simulator/slave definitions are added/modified/removed (alongside with their connections). */
		SlavesModified,
		/*! Used whenever a parameter of a slave has been modified. */
		SlaveParameterModified,
		/*! Used whenever connections have been modified. */
		ConnectionsModified,
		/*! Used when simulation settings have been modified.
			Only updates simulation settings page, all other views remain unmodified. */
		SimulationSettingsModified,
		/*! Used whenever the project data changes completely (new project created, project loaded etc.)
			and a complete reset of all views and models is needed. */
		AllModified = 0xFFFF0001
	};

	/*! Possible outcomes of the saveWithNewFilename() function. */
	enum SaveResult {
		SaveOK,			///< All ok.
		SaveFailed,		///< Couldn't write file.
		SaveCanceled	///< User canceled file name request dialog.
	};


	/*! Constructor, only to be used by MSIMMainWindow (but MSIMMainWindow must not be a friend of us). */
	MSIMProjectHandler();

	/*! Destructor, destroys managed project. */
	~MSIMProjectHandler();

	/*! Returns whether or not a project had been created yet. */
	bool isValid() const { return m_project != NULL; }

	/*! Returns 'true' if some part of the project was modified.*/
	bool isModified() const { return m_modified; }

	/*! Return the project file of the currently managed project.
		\return Returns a file path when the current project has been
				saved already. Returns an empty string when the project
				is newly created and hasn't been saved yet.
	*/
	QString projectFile() const { return m_projectFile; }

	/*! Creates a new project instance (must not have one already) and resets project file name.
		Emits updateActions() to signal that view state has changed.
	*/
	void newProject(QWidget * parent);

	/*! Checks for modifications and asks user to confirm saving, then closes project.
		Also destroys project object.
		Emits updateActions() signal.
		\note The function can be safely called regardless of the valid state of the project handler.
			  Function does nothing of no project exists.
	*/
	bool closeProject(QWidget * parent);

	/*! Creates a new project and loads a project file with given filename.
		This function calles internally setModified(AllModified) when reading
		was successful.
		If project cannot be loaded, the project is destroyed again and the
		function returns with invalid state of project manager.
		Emits updateActions() signal on success.
		\param parent Parent widget, needed for QMessageBox
		\param filename Filename of project file to read.
		\param silent If true, error messages won't pop-up as dialog box but rather be
					  sent to IBK::IBK_Message().
	*/
	void loadProject(	QWidget * parent,
						const QString & filename,
						bool silent);

	/*! Closes project (discarding modifications) and reopens the project.
		Project must have a valid filename already.
	*/
	void reloadProject(QWidget *parent);

	/*! Saves project with new filename (interactive function, asks user to input filename).
		Calls saveProject() internally.
	*/
	SaveResult saveWithNewFilename(QWidget * parent);

	/*! Saves the project with the given filename (non-interactive).
		Emits updateActions() signal on success.
	*/
	SaveResult saveProject(QWidget * parent, const QString & fileName);

	/*! Interface function for the user interface that allows
		different levels of modifications to be signalled to the project.

		\param modificationType An integer value defining the type of
			modification. The implementation should handle the default
			modification types (\sa DefaultModificationTypes).

		\param data The optional data argument can contain additional
			information about the specifics of the modification event.
			The implementation should cast the data pointer into
			corresponding types according to modification type.

		This function should be re-implemented in the derived class to
		handle all modifications to the project data.

		This function emits the modified() signal.

		If NotModified is passed as argument, the modification flag is reset.
		In this case the modified() signal is not emitted.

		The default implementation emits the modified() signal passing
		the function arguments and signal arguments.

		Emits updateActions() signal.
	*/
	void setModified(int modificationType, void * data = NULL);

	/*! Returns a const reference to the internal project.
		\warning This function throws an exception if there isn't a valid project loaded.
	*/
	const MASTER_SIM::Project & project() const;

	/*! Returns the time stamp of the last modification of the current project. */
	const QDateTime lastReadTime() const { return m_lastReadTime; }

	/*! Updates time stamp of the last modification of the current project, but only if project is present. */
	void updateLastReadTime();

signals:
	/*! Emitted when the project has been modified.

		The signal should be emitted from the setModified() function and passes the
		modification event info.
		\param modificationType Modification type (you need to cast the int into ModificationType).
		\param data The optional data argument (NULL if unused)
		\sa DefaultModificationTypes
	*/
	void modified( int modificationType, void * data );

	/*! Emitted from closeProject(), loadProject(), saveProject() and saveWithNewFilename(), whenever
		file name or modification state changes.
		This signal is connected to the corresponding function in the MainWindow to update the ui-state.
	*/
	void updateActions();

	/*! A signal emitted from addToRecentFiles()

		Updates the menu entries in the 'Recent Projects' submenu.
		This is a slot because we need to update the menu with the actions
		a bit delayed. When the user clicks on a recent project menu entry, the
		loadProject() function is indirectly called which in turn calls
		updateRecentProjects(). Since the menu actions in the recent projects
		menu are deleted, this would mean that the action currently processed is
		being deleted - causing a crash. Therefore we don't call updateRecentProjects()
		directly, but via a QTimer::singleShot() and thus ensure that the
		action handler function is completed before the action is touched.
	*/
	void updateRecentProjects();

private:

	/*! Creates a new project instance (must not have one already) and resets project file name.
		This function is used internally.
		\note Does not emit any signals and does not call setModified()
	*/
	void createProject();

	/*! Deletes the project being wrapped and sets internal m_project to NULL.
		This function is used internally and called from closeProject().
		Afterwards the project handler has invalid state (no project).
		\note Does not emit any signals and does not call setModified()
	*/
	void destroyProject();

	/*! Attempts to read the project with the given filename.
		If the reading was successful, the member variable projectFile is updated to 'fname'.
		\param fname The filename of the project file to be read.
		\return Returns true on success, false on error (error messages are written to IBK::IBK_Message).
		\note Does not emit any signals and does not call setModified()
	*/
	bool read(const QString & fname);

	/*! Writes the project with the given filename.
		If the writing was successful, the member variable m_projectFile is updated to 'fname'.
		\param fname The filename of the project file to be read.
		\return Returns true on success, false on error (error messages are written to IBK::IBK_Message).
		\note Does not emit any signals and does not call setModified()
	*/
	bool write(const QString & fname) const;

	/*! Adds the file fname to the list of most recently used files.
		The default implementation also emits the signal updateRecentProjects() which
		triggers a menu update.
		\sa updateRecentProjects()
	*/
	void addToRecentFiles(const QString& fname);


	// *** PRIVATE DATA MEMBERS ***

	/*! Pointer to self instance.
		Returned from instance().
	*/
	static MSIMProjectHandler	*m_self;

	/*! Pointer to the general project data storage class.
		This pointer is set and unset via the member functions create() and destroy().
		Access to the project instance is strictly forbidden and should only be granted
		to undo actions via a suitable mechanism.
	*/
	MASTER_SIM::Project		*m_project;

	/*! Holds the graphical representation of the FMU simulation scenario. */
	BLOCKMOD::Network		*m_network;

	/*! Holds the time stamp of the last time the project was read.
		This time stamp is updated in read() and used to check for external project modifications.
	*/
	QDateTime				m_lastReadTime;

	/*! Contains the state whether the project was modified or not. */
	bool					m_modified;


	/*! Contains the file name of the current project.
		Newly created projects have an empty file name (this can be used to
		determine whether a SaveAs dialog should be shown on first save).
		The variable is only set in the read() and write() functions.
		*/
	mutable QString			m_projectFile;
};

/*! Convenience function for accessing the MASTER_SIM::Project data directly with a shorter synopsis. */
inline const MASTER_SIM::Project & project() { return MSIMProjectHandler::instance().project(); }

#endif // MSIMProjectHandlerH
