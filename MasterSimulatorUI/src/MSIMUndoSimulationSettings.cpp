#include "MSIMUndoSimulationSettings.h"
#include "MSIMProjectHandler.h"

MSIMUndoSimulationSettings::MSIMUndoSimulationSettings(const QString & label,
								 const MASTER_SIM::Project & newProject) :
	m_project(newProject)
{
	setText( label );
}


void MSIMUndoSimulationSettings::undo() {

	// exchange Project
	std::swap( theProject(), m_project );

	// tell project handler that everything has changed
	MSIMProjectHandler::instance().setModified( MSIMProjectHandler::SimulationSettingsModified);
}


void MSIMUndoSimulationSettings::redo() {
	undo();
}
