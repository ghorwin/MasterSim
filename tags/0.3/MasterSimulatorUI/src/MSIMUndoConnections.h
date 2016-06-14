#ifndef MSIMUndoConnectionsH
#define MSIMUndoConnectionsH

#include "MSIMUndoCommandBase.h"

/*! Command for changing simulation settings data. */
class MSIMUndoConnections : public MSIMUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(MSIMUndoConnections)
public:
	MSIMUndoConnections(const QString & label,
				   const MASTER_SIM::Project & newProject
	);

	virtual void undo();
	virtual void redo();

private:

	/*! Cache for solver settings. */
	MASTER_SIM::Project	m_project;

};

#endif // MSIMUndoConnectionsH
