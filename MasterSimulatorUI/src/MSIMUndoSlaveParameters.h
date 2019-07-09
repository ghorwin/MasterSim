#ifndef MSIMUndoSlaveParametersH
#define MSIMUndoSlaveParametersH

#include "MSIMUndoCommandBase.h"

/*! Command for changing simulation settings data. */
class MSIMUndoSlaveParameters : public MSIMUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(MSIMUndoSlaveParameters)
public:
	MSIMUndoSlaveParameters(const QString & label,
							unsigned int slaveIndex,
							const std::string & parameterName,
							const std::string & value
	);

	/*! Unique ID for this undo command - needed for command compression. */
	int id() const { return 0x11; }

	virtual void undo();
	virtual void redo();

	/*! Called when a subsequent command of same type is added to the stack.
		In this case, both commands are merged.
	*/
	bool mergeWith(const QUndoCommand *other);

private:

	unsigned int m_slaveIndex;
	std::string	m_parameterName;
	std::string m_value;

};

#endif // MSIMUndoSlaveParametersH
