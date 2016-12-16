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

	virtual QWidget * createEditor ( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
	virtual void setEditorData ( QWidget * editor, const QModelIndex & index ) const;
	virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
	virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex & index) const;

private slots:
	void rejectCloseEditor();
	void commitAndCloseEditor();
};

#endif // MSIMSlaveItemDelegateH
