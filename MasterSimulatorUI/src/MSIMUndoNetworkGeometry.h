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

	bool				m_firstRedo;
};

#endif // MSIMUndoNetworkGeometryH
