#ifndef MSIMUndoSlavesH
#define MSIMUndoSlavesH

#include "MSIMUndoCommandBase.h"

/*! Command for changing simulation settings data. */
class MSIMUndoSlaves : public MSIMUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(MSIMUndoSlaves)
public:
	MSIMUndoSlaves(const QString & label,
				   const MASTER_SIM::Project & newProject
	);

	virtual void undo();
	virtual void redo();

private:

	/*! Cache for solver settings. */
	MASTER_SIM::Project	m_project;

};

#endif // MSIMUndoSlavesH
