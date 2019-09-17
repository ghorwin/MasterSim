#include "MSIMSlaveBlock.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include <BM_Block.h>

void SlaveBlock::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget) {
	// special handling for invisible blocks
	if (isInvisible())
		return; // nothing to be drawn

	// special handlin for blocks without FMU
	if (!block()->m_properties.contains("haveFMU")) {
		painter->save();
		QPen p(Qt::gray);
		p.setStyle(Qt::DashLine);
		painter->setPen( p );
		painter->drawRect(rect());
		// now draw the label of the block
		QRectF r = rect();
		r.moveTop(4);
		painter->drawText(r, Qt::AlignTop | Qt::AlignHCenter, block()->m_name);
		painter->restore();
		return;
	}
	BlockItem::paint(painter, option, widget);
}
