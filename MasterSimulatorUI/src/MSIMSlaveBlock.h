#ifndef MSIMSLAVEBLOCK_H
#define MSIMSLAVEBLOCK_H

#include <BM_BlockItem.h>

class MSIMSlaveBlock : public BLOCKMOD::BlockItem {
public:
	explicit MSIMSlaveBlock(BLOCKMOD::Block * b) :
		BLOCKMOD::BlockItem(b)
	{}

protected:
	/*! Re-implemented to draw the styled rectangle of the block. */
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

};

#endif // MSIMSLAVEBLOCK_H
