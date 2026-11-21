#include "MSIMConnectionItemDelegate.h"

#include <QPainter>
#include <QColor>

#include "MSIMSettings.h"

MSIMConnectionItemDelegate::MSIMConnectionItemDelegate(QObject *parent) :
	QItemDelegate(parent)
{
}


void MSIMConnectionItemDelegate::paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const {
	// special handling for selections: use the same selection color as in the slave table and
	// keep the item's text color (e.g. slave color), instead of the style's highlighted text color
	if (option.state & QStyle::State_Selected) {
		const QColor selColor = MSIMSettings::selectionColor();

		QStyleOptionViewItem option2(option);
		QColor textColor = index.data(Qt::ForegroundRole).value<QColor>();
		if (!textColor.isValid())
			textColor = option.palette.color(QPalette::Text);
		for (QPalette::ColorGroup group : {QPalette::Active, QPalette::Inactive}) {
			option2.palette.setColor(group, QPalette::Highlight, selColor);
			option2.palette.setColor(group, QPalette::HighlightedText, textColor);
		}
		QItemDelegate::paint(painter, option2, index);
	}
	else {
		QItemDelegate::paint(painter, option, index);
	}
}

