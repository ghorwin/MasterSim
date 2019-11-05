#ifndef MSIMConnectionItemDelegateH
#define MSIMConnectionItemDelegateH

#include <QItemDelegate>

/*! This class implementation is only used to draw the background colors for slaves. */
class MSIMConnectionItemDelegate : public QItemDelegate {
	Q_OBJECT
public:
	explicit MSIMConnectionItemDelegate(QObject *parent = nullptr);

protected:
	virtual void paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
};

#endif // MSIMConnectionItemDelegateH
