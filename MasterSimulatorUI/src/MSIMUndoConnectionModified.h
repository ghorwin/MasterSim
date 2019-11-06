#ifndef MSIMUndoConnectionModifiedH
#define MSIMUndoConnectionModifiedH

#include "MSIMUndoCommandBase.h"

#include <BM_Network.h>

/*! Command for changing connector properties (but not inlet/outlet slaves). */
class MSIMUndoConnectionModified : public MSIMUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(MSIMUndoConnectionModified)
public:
	MSIMUndoConnectionModified(const QString & label,
						const MASTER_SIM::Project & newProject
	);

	virtual void undo();
	virtual void redo();

private:

	/*! Cache for solver settings. */
	MASTER_SIM::Project	m_project;
};

#endif // MSIMUndoConnectionModifiedH
