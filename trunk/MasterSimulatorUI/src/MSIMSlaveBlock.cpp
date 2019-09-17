#include "MSIMSlaveBlock.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include <BM_Block.h>

void MSIMSlaveBlock::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget) {
	// special handling for invisible blocks
	if (isInvisible())
		return; // nothing to be drawn

	// special handlin for blocks without FMU
	if (!block()->m_properties.contains("haveFMU")) {
		painter->save();
		QRectF r = rect();
		QPen p(Qt::black);
		QBrush b(QColor(96,96,96,64));
		p.setStyle(Qt::DashLine);
		painter->setBrush(b);
		painter->setPen( p );
		painter->drawRect(r);
		// now draw the label of the block
		r.moveTop(4);
		painter->setPen(Qt::black);
		painter->drawText(r, Qt::AlignTop | Qt::AlignHCenter, block()->m_name);
		painter->restore();
		return;
	}
	BlockItem::paint(painter, option, widget);
}
