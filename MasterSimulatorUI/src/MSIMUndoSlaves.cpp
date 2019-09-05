#include "MSIMUndoSlaves.h"
#include "MSIMProjectHandler.h"

#include <BM_SceneManager.h>

MSIMUndoSlaves::MSIMUndoSlaves(const QString & label,
							   const MASTER_SIM::Project & newProject,
							   const BLOCKMOD::Network & newNetwork) :
	m_project(newProject),
	m_network(newNetwork)
{
	setText( label );
}


void MSIMUndoSlaves::undo() {

	// exchange Project
	std::swap( theProject(), m_project );

	// exchange Network
	BLOCKMOD::Network n = MSIMProjectHandler::instance().sceneManager()->network();
	MSIMProjectHandler::instance().sceneManager()->setNetwork(m_network);
	m_network.swap(n);

	// tell project handler that everything has changed
	MSIMProjectHandler::instance().setModified( MSIMProjectHandler::SlavesModified);
}


void MSIMUndoSlaves::redo() {
	undo();
}
