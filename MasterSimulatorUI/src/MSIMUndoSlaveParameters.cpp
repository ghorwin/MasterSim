#include "MSIMUndoSlaveParameters.h"
#include "MSIMProjectHandler.h"

MSIMUndoSlaveParameters::MSIMUndoSlaveParameters(const QString & label,
												 unsigned int slaveIndex,
												 const std::string & parameterName,
												 const std::string & value) :
	m_slaveIndex(slaveIndex),
	m_parameterName(parameterName),
	m_value(value)
{
	setText( label );
}


void MSIMUndoSlaveParameters::undo() {

	// retrieve previous map value
	std::string currentParameterValue = theProject().m_simulators[m_slaveIndex].m_parameters[m_parameterName];
	if (m_value.empty())
		theProject().m_simulators[m_slaveIndex].m_parameters.erase(m_parameterName);
	else
		theProject().m_simulators[m_slaveIndex].m_parameters[m_parameterName] = m_value;
	m_value.swap(currentParameterValue);

	// tell project handler that everything has changed
	MSIMProjectHandler::instance().setModified(MSIMProjectHandler::SlaveParameterModified);
}


void MSIMUndoSlaveParameters::redo() {
	undo();
}


bool MSIMUndoSlaveParameters::mergeWith(const QUndoCommand *other) {
	const MSIMUndoSlaveParameters * otherCmd = dynamic_cast<const MSIMUndoSlaveParameters*>(other);
	if (otherCmd->m_slaveIndex != m_slaveIndex)
		return false;
	if (otherCmd->m_parameterName != m_parameterName)
		return false;

	// this->m_value holds original value (the value to go back on undo)
	// otherCmd->m_value holds the new undo value (the one that was just retrieved during redo)
	// project holds new value
	//
	// to merge, we do not have to do anything

	return true;
}
