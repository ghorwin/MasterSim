#ifndef MSIM_SCENEMANAGER_H
#define MSIM_SCENEMANAGER_H

#include <BM_SceneManager.h>

namespace BLOCKMOD {
	class BlockItem;
	class Block;
}

class MSIMSceneManager : public BLOCKMOD::SceneManager {
public:
	explicit MSIMSceneManager(QObject *parent = nullptr);

protected:

	/*! Create a graphics item based on the data in the given block.
		You can override this method and create your own graphics items, derived from
		base class BlockItem (which contains all the move/selection logic).
	*/
	BLOCKMOD::BlockItem * createBlockItem(BLOCKMOD::Block & b) override;

};

#endif // MSIM_SCENEMANAGER_H
