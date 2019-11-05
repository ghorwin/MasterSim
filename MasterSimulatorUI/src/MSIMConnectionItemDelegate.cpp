#include "MSIMConnectionItemDelegate.h"

#include <QPainter>
#include <QColor>
#include <QTextDocument>

#include "MSIMUIConstants.h"

MSIMConnectionItemDelegate::MSIMConnectionItemDelegate(QObject *parent) :
	QItemDelegate(parent)
{
}

void MSIMConnectionItemDelegate::paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const {
	// special handling for selections
	if (option.state & QStyle::State_Selected) {

		QRect checkRect, decorationRect, displayRect;
		doLayout(option, &checkRect, &decorationRect, &displayRect, false);

		// unfortunately, the bar with the selection color has already been drawn, so we first
		// need to undo that and overpaint with window color

		painter->save();
		painter->setBrush(option.palette.brush(QPalette::Normal, QPalette::Base));
		painter->setPen(Qt::NoPen);
		painter->drawRect(displayRect);
		painter->restore();

		// draw the item
		QStyleOptionViewItem option2(option);
		option2.state = QStyle::State_Enabled;
		QString text = index.data().toString();
		QColor textColor = index.data(Qt::TextColorRole).value<QColor>();
		painter->setPen(textColor);
		painter->drawText(displayRect.adjusted(3,0,0,0), Qt::AlignVCenter | Qt::AlignLeft, text);
//		QColor foregroundColor = index.data(Qt::ForegroundRole).value<QColor>();
//		option2.
//		option2.palette.setColor(QPalette::Normal, QPalette::Text, textColor);
//		drawDisplay(painter, option2, displayRect, text);
//		painter->drawRect(displayRect.adjusted(0, 0, -1, -1));

		QPalette p;
//		option2.backgroundBrush = option.palette.brush(QPalette::Normal, QPalette::Window);
//		drawBackground(painter, option2, index);
//		QItemDelegate::paint(painter, option2, index);
		// now overpaint a transparent selection
		QColor selectionColor(128,128,128,64);
		painter->setBrush(selectionColor);
		painter->setPen(Qt::NoPen);
		painter->drawRect(option.rect);
	}
	else {
		QItemDelegate::paint(painter, option, index);
	}
#if 0
	QVariant haveError = index.data(Qt::UserRole+1);
	if (haveError.toBool()) {
		QItemDelegate::paint(painter, option, index);
		return;
	}

	QString text = index.data().toString();
	QVariant data = index.data(Qt::UserRole);

	QColor col = data.value<QColor>();
	QRect r(option.rect);

	if (text.isEmpty()) {
		// color background when there is no text
		painter->setBrush(col);
		painter->drawRect(r);
	}
	else {
		// write colored text otherwise
		QStyleOptionViewItem options = option;

		painter->save();

		QTextDocument doc;
		QFont f;
		f.setPointSize(TABLE_FONT_SIZE);
		doc.setDefaultFont(f);
		doc.setDocumentMargin(0);
		QString htmlText = QString("<div style=\"margin:0; color:%1\">%2</div>").arg(col.name()).arg(text);
		doc.setHtml(htmlText);

		options.text = "";
		options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter);

		painter->translate(r.left()+2, r.top());
		QRect clip(0, 0, r.width(), r.height());
		doc.drawContents(painter, clip);

		painter->restore();
	}
#endif
}

