#include "MSIMUndoSlaves.h"
#include "MSIMProjectHandler.h"

MSIMUndoSlaves::MSIMUndoSlaves(const QString & label,
								 const MASTER_SIM::Project & newProject) :
	m_project(newProject)
{
	setText( label );
}


void MSIMUndoSlaves::undo() {

	// exchange Project
	std::swap( theProject(), m_project );

	// tell project handler that everything has changed
	MSIMProjectHandler::instance().setModified( MSIMProjectHandler::SlavesModified);
}


void MSIMUndoSlaves::redo() {
	undo();
}
