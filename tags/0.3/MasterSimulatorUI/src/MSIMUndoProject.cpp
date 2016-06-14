#include "MSIMUndoProject.h"
#include "MSIMProjectHandler.h"

MSIMUndoProject::MSIMUndoProject(const QString & label,
								 const MASTER_SIM::Project & newProject) :
	m_project(newProject)
{
	setText( label );
}


void MSIMUndoProject::undo() {

	// exchange Project
	std::swap( theProject(), m_project );

	// tell project handler that everything has changed
	MSIMProjectHandler::instance().setModified( MSIMProjectHandler::AllModified);
}


void MSIMUndoProject::redo() {
	undo();
}
