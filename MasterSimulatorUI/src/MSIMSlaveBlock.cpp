#include "MSIMSlaveBlock.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QStyle>

#include <BM_Block.h>

/*! Returns color blended with white, whiteFraction = 0 returns the original color, 1 returns white. */
static QColor blendWithWhite(const QColor & c, double whiteFraction) {
	return QColor::fromRgbF(
		float(c.redF()   + (1 - c.redF())   * whiteFraction),
		float(c.greenF() + (1 - c.greenF()) * whiteFraction),
		float(c.blueF()  + (1 - c.blueF())  * whiteFraction));
}

void MSIMSlaveBlock::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget) {
	// special handling for invisible blocks
	if (isInvisible())
		return; // nothing to be drawn

	BlockState state = StateNoFMU; // default unknown state
	if (block()->m_properties.contains("state"))
		state = (BlockState)block()->m_properties["state"].toInt();

	switch (state) {
		case StateCorrect : {
			// if a pixmap is shown or no slave color is known, draw original stuff
			bool showPixmap = block()->m_properties.value("ShowPixmap").toBool() &&
					block()->m_properties.contains("Pixmap");
			if (showPixmap || !block()->m_properties.contains("Color")) {
				BlockItem::paint(painter, option, widget);
				return;
			}
			// draw block with slave color as background color; use light tints of the color
			// so that the black block and socket labels remain readable
			QColor c = block()->m_properties.value("Color").value<QColor>();
			painter->save();
			painter->setRenderHint(QPainter::Antialiasing, true);
			QLinearGradient grad(QPointF(0,0), QPointF(rect().width(),0));
			grad.setColorAt(0, blendWithWhite(c, 0.4));
			grad.setColorAt(1, blendWithWhite(c, 0.6));
			painter->fillRect(rect(), grad);
			if (option->state.testFlag(QStyle::State_Selected))
				painter->setPen(QPen(QBrush(QColor(0,128,0)), 1.5));
			else
				painter->setPen(Qt::black);
			painter->setBrush(Qt::NoBrush);
			painter->drawRect(rect());
			// now draw the label of the block
			QRectF r = rect();
			r.moveTop(4);
			painter->setPen(Qt::black);
			painter->drawText(r, Qt::AlignTop | Qt::AlignHCenter, block()->m_name);
			painter->restore();
			return;
		}

		case StateUnsynced : {
			painter->save();
			QRectF r = rect();
			QBrush b(QColor(255,96,96,100));
			QPen p(QColor(192,0,0));
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
		} break;

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
		} break;

	}

	// paint children
	for (QGraphicsItem * child : childItems()) {
		child->paint(painter, option, widget);
	}
}
