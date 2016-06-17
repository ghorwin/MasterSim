#include "MSIMUndoConnections.h"
#include "MSIMProjectHandler.h"

MSIMUndoConnections::MSIMUndoConnections(const QString & label,
								 const MASTER_SIM::Project & newProject) :
	m_project(newProject)
{
	setText( label );
}


void MSIMUndoConnections::undo() {

	// exchange Project
	std::swap( theProject(), m_project );

	// tell project handler that everything has changed
	MSIMProjectHandler::instance().setModified( MSIMProjectHandler::ConnectionsModified);
}


void MSIMUndoConnections::redo() {
	undo();
}
