#ifndef MSIMUndoSimulationSettingsH
#define MSIMUndoSimulationSettingsH

#include "MSIMUndoCommandBase.h"

/*! Command for changing simulation settings data. */
class MSIMUndoSimulationSettings : public MSIMUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(MSIMUndoSimulationSettings)
public:
	MSIMUndoSimulationSettings(const QString & label,
				   const MASTER_SIM::Project & newProject
	);

	virtual void undo();
	virtual void redo();

private:

	/*! Cache for solver settings. */
	MASTER_SIM::Project	m_project;

};

#endif // MSIMUndoSimulationSettingsH
