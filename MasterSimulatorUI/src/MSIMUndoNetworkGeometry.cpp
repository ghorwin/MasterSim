#include "MSIMUndoNetworkGeometry.h"
#include "MSIMProjectHandler.h"

#include <BM_SceneManager.h>

MSIMUndoNetworkGeometry::MSIMUndoNetworkGeometry(const QString & label,
							   const BLOCKMOD::Network & newNetwork) :
	m_network(newNetwork),
	m_firstRedo(true)
{
	setText( label );
}


void MSIMUndoNetworkGeometry::undo() {
	// exchange Network
	BLOCKMOD::Network n = MSIMProjectHandler::instance().sceneManager()->network();
	MSIMProjectHandler::instance().sceneManager()->setNetwork(m_network);
	m_network.swap(n);

	// tell project handler that geometry has changed
	MSIMProjectHandler::instance().setModified( MSIMProjectHandler::NetworkGeometryModified);
}


void MSIMUndoNetworkGeometry::redo() {
	// protect against changing of graphics item while being in the processing of an event
	// of an item
	BLOCKMOD::Network n = MSIMProjectHandler::instance().sceneManager()->network();
	m_network.swap(n);
	if (m_firstRedo) {
		m_firstRedo = false;
	}
	else
		MSIMProjectHandler::instance().sceneManager()->setNetwork(m_network);
	// tell project handler that geometry has changed
	MSIMProjectHandler::instance().setModified( MSIMProjectHandler::NetworkGeometryModified);
}
