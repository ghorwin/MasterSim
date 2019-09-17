#include "MSIMSceneManager.h"

#include <BM_Block.h>

#include "MSIMSlaveBlock.h"

MSIMSceneManager::MSIMSceneManager(QObject *parent) :
	BLOCKMOD::SceneManager(parent)
{
}


BLOCKMOD::BlockItem * MSIMSceneManager::createBlockItem(BLOCKMOD::Block & b) {
	BLOCKMOD::BlockItem * item = new MSIMSlaveBlock(&b);
	item->setRect(0,0,b.m_size.width(), b.m_size.height());
	item->setPos(b.m_pos);
	return item;
}

