#ifndef MSIMSLAVEBLOCK_H
#define MSIMSLAVEBLOCK_H

#include <BM_BlockItem.h>

/*! Re-implemented BlockItem to provide different painting based on sync-state of block definition with underlying FMU. */
class MSIMSlaveBlock : public BLOCKMOD::BlockItem {
public:
	/*! Different sync states. */
	enum BlockState {
		/*! Correct and synced state (FMU is known and all sockets match) : draw regular block. */
		StateCorrect     = 1,
		/*! Unsynced state (FMU is known, but sockets mismatch; happens after initial import of block) : draw block with red-dashed outline. */
		StateUnsynced    = 2,
		/*! No-FMU state (FMU is unknown - no way to know whether block is correct or not) : draw block in gray-dashed outline and light-gray background. */
		StateNoFMU       = 3
	};

	/*! Initializing constructor. */
	explicit MSIMSlaveBlock(BLOCKMOD::Block * b) :
		BLOCKMOD::BlockItem(b)
	{}

protected:
	/*! Re-implemented to draw the styled rectangle of the block.
		Depending on the state of the block, different appearances are drawn, see BlockState.
		\sa BlockState
	*/
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

};

#endif // MSIMSLAVEBLOCK_H
