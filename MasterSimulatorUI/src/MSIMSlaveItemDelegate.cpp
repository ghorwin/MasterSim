#include "MSIMSlaveItemDelegate.h"

#include <QPainter>
#include <QColor>

MSIMSlaveItemDelegate::MSIMSlaveItemDelegate(QObject *parent) :
	QItemDelegate(parent)
{
}

void MSIMSlaveItemDelegate::paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const {
	if (index.column() != 0) {
		QItemDelegate::paint(painter, option, index);
		return;
	}

	QVariant data = index.data(Qt::UserRole);
	QColor col = data.value<QColor>();
	QRect r(option.rect);
	painter->setBrush(col);
	painter->drawRect(r);
}

