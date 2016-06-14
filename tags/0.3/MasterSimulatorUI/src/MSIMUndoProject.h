#ifndef MSIMUndoProjectH
#define MSIMUndoProjectH

#include "MSIMUndoCommandBase.h"

/*! Command for changing project data entirely. */
class MSIMUndoProject : public MSIMUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(MSIMUndoProject)
public:
	MSIMUndoProject(const QString & label,
				   const MASTER_SIM::Project & newProject
	);

	virtual void undo();
	virtual void redo();

private:

	/*! Cache for solver settings. */
	MASTER_SIM::Project	m_project;

};

#endif // MSIMUndoProjectH
