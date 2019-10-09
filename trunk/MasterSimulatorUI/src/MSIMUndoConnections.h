#ifndef MSIMUndoConnectionsH
#define MSIMUndoConnectionsH

#include "MSIMUndoCommandBase.h"

#include <BM_Network.h>

/*! Command for changing connectors. */
class MSIMUndoConnections : public MSIMUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(MSIMUndoConnections)
public:
	MSIMUndoConnections(const QString & label,
						const MASTER_SIM::Project & newProject,
						const BLOCKMOD::Network & newNetwork
	);

	virtual void undo();
	virtual void redo();

private:

	/*! Cache for solver settings. */
	MASTER_SIM::Project	m_project;

	/*! Cache for network before/after the connection was inserted/removed. */
	BLOCKMOD::Network	m_network;
};

#endif // MSIMUndoConnectionsH
