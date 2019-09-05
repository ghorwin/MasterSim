#ifndef MSIMUndoSlavesH
#define MSIMUndoSlavesH

#include "MSIMUndoCommandBase.h"

#include <BM_Network.h>

/*! Command for changing simulation settings data. */
class MSIMUndoSlaves : public MSIMUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(MSIMUndoSlaves)
public:
	MSIMUndoSlaves(const QString & label,
				   const MASTER_SIM::Project & newProject,
				   const BLOCKMOD::Network & newNetwork
	);

	virtual void undo();
	virtual void redo();

private:

	/*! Cache for solver settings. */
	MASTER_SIM::Project	m_project;

	/*! Cache for network before/after the block was inserted. */
	BLOCKMOD::Network	m_network;
};

#endif // MSIMUndoSlavesH
