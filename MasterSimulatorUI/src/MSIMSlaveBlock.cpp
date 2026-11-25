#include "MSIMSlaveBlock.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QStyle>

#include <BM_Block.h>
#include <BM_Globals.h>

/*! Returns color c blended with color base, fraction = 0 returns c, 1 returns base. */
static QColor blendWith(const QColor & c, const QColor & base, double fraction) {
	return QColor::fromRgbF(
		float(c.redF()   + (base.redF()   - c.redF())   * fraction),
		float(c.greenF() + (base.greenF() - c.greenF()) * fraction),
		float(c.blueF()  + (base.blueF()  - c.blueF())  * fraction));
}

void MSIMSlaveBlock::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget) {
	// special handling for invisible blocks
	if (isInvisible())
		return; // nothing to be drawn

	const BLOCKMOD::Theme & theme = BLOCKMOD::Globals::CurrentTheme;

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
			// draw block with slave color as background color; use tints of the color (light tints in
			// light theme, dark tints in dark theme) so that the block and socket labels remain readable.
			// Unselected blocks are drawn with faded tints, selected blocks with much stronger tints
			// (i.e. darker in light theme, brighter in dark theme) and a thick black border.
			bool selected = option->state.testFlag(QStyle::State_Selected);
			QColor c = block()->m_properties.value("Color").value<QColor>();
			painter->save();
			painter->setRenderHint(QPainter::Antialiasing, true);
			QLinearGradient grad(QPointF(0,0), QPointF(rect().width(),0));
			if (selected) {
				grad.setColorAt(0, blendWith(c, theme.blockTintBase, 0.1));
				grad.setColorAt(1, blendWith(c, theme.blockTintBase, 0.25));
			}
			else {
				grad.setColorAt(0, blendWith(c, theme.blockTintBase, 0.55));
				grad.setColorAt(1, blendWith(c, theme.blockTintBase, 0.75));
			}
			painter->fillRect(rect(), grad);
			if (selected)
				painter->setPen(QPen(QBrush(theme.blockBorder), 2));
			else
				painter->setPen(theme.blockBorder);
			painter->setBrush(Qt::NoBrush);
			// thick border is drawn inside the block rect, so that it is not clipped at the bounding rect
			if (selected)
				painter->drawRect(rect().adjusted(1, 1, -1, -1));
			else
				painter->drawRect(rect());
			// now draw the label of the block
			QRectF r = rect();
			r.moveTop(4);
			painter->setPen(theme.blockText);
			painter->drawText(r, Qt::AlignTop | Qt::AlignHCenter, block()->m_name);
			painter->restore();
			return;
		}

		case StateUnsynced : {
			painter->save();
			QRectF r = rect();
			QBrush b(QColor(255,96,96,100));
			QPen p(theme.connectorSelected);
			if (option->state.testFlag(QStyle::State_Selected))
				p.setStyle(Qt::DashLine);
			else
				p.setStyle(Qt::DotLine);
			painter->setBrush(b);
			painter->setPen( p );
			painter->drawRect(r);
			// now draw the label of the block
			r.moveTop(4);
			painter->setPen(theme.blockText);
			painter->drawText(r, Qt::AlignTop | Qt::AlignHCenter, block()->m_name);
			painter->restore();
		} break;

		case StateNoFMU : {
			painter->save();
			QRectF r = rect();
			QBrush b(QColor(96,96,96,64));
			QPen p(theme.blockBorder);
			if (option->state.testFlag(QStyle::State_Selected))
				p.setStyle(Qt::DashLine);
			else
				p.setStyle(Qt::DotLine);
			painter->setBrush(b);
			painter->setPen( p );
			painter->drawRect(r);
			// now draw the label of the block
			r.moveTop(4);
			painter->setPen(theme.blockText);
			painter->drawText(r, Qt::AlignTop | Qt::AlignHCenter, block()->m_name);
			painter->restore();
		} break;

	}

	// paint children
	for (QGraphicsItem * child : childItems()) {
		child->paint(painter, option, widget);
	}
}
