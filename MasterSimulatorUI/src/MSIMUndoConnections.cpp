#include "MSIMUndoConnections.h"
#include "MSIMProjectHandler.h"

#include <BM_SceneManager.h>

MSIMUndoConnections::MSIMUndoConnections(const QString & label,
										 const MASTER_SIM::Project & newProject,
										 const BLOCKMOD::Network & newNetwork) :
	m_project(newProject),
	m_network(newNetwork)
{
	setText( label );
}


void MSIMUndoConnections::undo() {

	// exchange Project
	std::swap( theProject(), m_project );

	// exchange Network
	BLOCKMOD::Network n = MSIMProjectHandler::instance().network(); // get current network from project
	setNetwork(m_network); // set new network in both project and scene manager
	m_network.swap(n); // exchange cached network data

	// tell project handler that everything has changed
	MSIMProjectHandler::instance().setModified( MSIMProjectHandler::ConnectionsModified);
}


void MSIMUndoConnections::redo() {
	undo();
}
