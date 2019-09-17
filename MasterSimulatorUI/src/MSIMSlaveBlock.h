#ifndef MSIMSLAVEBLOCK_H
#define MSIMSLAVEBLOCK_H

#include <BM_BlockItem.h>

class SlaveBlock : public BLOCKMOD::BlockItem {
public:
	explicit SlaveBlock(BLOCKMOD::Block * b) :
		BLOCKMOD::BlockItem(b)
	{}

protected:
	/*! Re-implemented to draw the styled rectangle of the block. */
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

};

#endif // MSIMSLAVEBLOCK_H
