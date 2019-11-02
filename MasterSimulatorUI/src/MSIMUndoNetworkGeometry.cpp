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
	BLOCKMOD::Network n = MSIMProjectHandler::instance().network(); // get current network
	m_network.swap(n); // n now contains the old network, the result of the undo

	setNetwork(n); // update both project and scene manager's network objectcs

	// tell project handler that geometry has changed
	MSIMProjectHandler::instance().setModified( MSIMProjectHandler::NetworkGeometryModified);
}


void MSIMUndoNetworkGeometry::redo() {
	// protect against changing of graphics item while being in the processing of an event
	// of an item
	BLOCKMOD::Network n = MSIMProjectHandler::instance().network(); // get old network without geometry change
	m_network.swap(n);

	if (m_firstRedo) {
		// on first redo, only update the project's network data structure
		m_firstRedo = false;
		setNetwork(n, true); // update only project's network object
	}
	else {
		setNetwork(n); // update both project and scene manager's network objects
	}
	// tell project handler that geometry has changed
	// Note: this will not cause the Slaves view to re-set the network
	MSIMProjectHandler::instance().setModified(MSIMProjectHandler::NetworkGeometryModified);
}
