#ifndef MSIMSlaveItemDelegateH
#define MSIMSlaveItemDelegateH

#include <QItemDelegate>

/*! This class implementation is only used to draw the background colors for slaves. */
class MSIMSlaveItemDelegate : public QItemDelegate {
	Q_OBJECT
public:
	explicit MSIMSlaveItemDelegate(QObject *parent = 0);

protected:
	virtual void paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
};

#endif // MSIMSlaveItemDelegateH
