#include "MSIMSlaveBlock.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QStyle>

#include <BM_Block.h>

void MSIMSlaveBlock::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget) {
	// special handling for invisible blocks
	if (isInvisible())
		return; // nothing to be drawn

	BlockState state = StateNoFMU; // default unknown state
	if (block()->m_properties.contains("state"))
		state = (BlockState)block()->m_properties["state"].toInt();

	switch (state) {
		case StateCorrect :
			// draw original stuff
			BlockItem::paint(painter, option, widget);
			return;

		case StateUnsynced : {
			painter->save();
			QRectF r = rect();
			QBrush b(QColor(192,96,96,64));
			QPen p(QColor(96,0,0));
			if (option->state.testFlag(QStyle::State_Selected))
				p.setStyle(Qt::DashLine);
			else
				p.setStyle(Qt::DotLine);
			painter->setBrush(b);
			painter->setPen( p );
			painter->drawRect(r);
			// now draw the label of the block
			r.moveTop(4);
			painter->setPen(Qt::black);
			painter->drawText(r, Qt::AlignTop | Qt::AlignHCenter, block()->m_name);
			painter->restore();
		}
		case StateNoFMU : {
			painter->save();
			QRectF r = rect();
			QBrush b(QColor(96,96,96,64));
			QPen p(Qt::black);
			if (option->state.testFlag(QStyle::State_Selected))
				p.setStyle(Qt::DashLine);
			else
				p.setStyle(Qt::DotLine);
			painter->setBrush(b);
			painter->setPen( p );
			painter->drawRect(r);
			// now draw the label of the block
			r.moveTop(4);
			painter->setPen(Qt::black);
			painter->drawText(r, Qt::AlignTop | Qt::AlignHCenter, block()->m_name);
			painter->restore();
		}

	}

	// paint children
	for (QGraphicsItem * child : childItems()) {
		child->paint(painter, option, widget);
	}
}
