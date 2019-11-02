#ifndef MSIMUndoNetworkGeometryH
#define MSIMUndoNetworkGeometryH

#include "MSIMUndoCommandBase.h"

#include <BM_Network.h>

/*! Command for changing slave blocks. */
class MSIMUndoNetworkGeometry : public MSIMUndoCommandBase {
	Q_DECLARE_TR_FUNCTIONS(MSIMUndoNetworkGeometry)
public:
	MSIMUndoNetworkGeometry(const QString & label,
				   const BLOCKMOD::Network & newNetwork
	);

	virtual void undo();
	virtual void redo();

private:
	/*! Cache for network before/after the block was inserted. */
	BLOCKMOD::Network	m_network;

	/*! We need to distinguish between the first redo (the initial modification of the project)
		and subsequent redos triggered by the undo action.
		In the first redo we must not update the network in the scene manager, because
		this would delete the block item that is actually sending the change. Do avoid that,
		we only update the project's copy of the network in the first redo, and only afterwards
		update both.
	*/
	bool				m_firstRedo;
};

#endif // MSIMUndoNetworkGeometryH
