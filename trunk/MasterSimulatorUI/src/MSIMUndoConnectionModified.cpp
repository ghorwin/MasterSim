#include "MSIMUndoConnectionModified.h"
#include "MSIMProjectHandler.h"

#include <BM_SceneManager.h>

MSIMUndoConnectionModified::MSIMUndoConnectionModified(const QString & label,
										 const MASTER_SIM::Project & newProject) :
	m_project(newProject)
{
	setText( label );
}


void MSIMUndoConnectionModified::undo() {

	// exchange Project
	std::swap( theProject(), m_project );

	// tell project handler that everything has changed
	MSIMProjectHandler::instance().setModified( MSIMProjectHandler::SingleConnectionModified);
}


void MSIMUndoConnectionModified::redo() {
	undo();
}
