#include "MSIMSlaveItemDelegate.h"

#include <QPainter>
#include <QColor>
#include <QTextDocument>
#include <QColorDialog>

#include "MSIMUIConstants.h"

MSIMSlaveItemDelegate::MSIMSlaveItemDelegate(QObject *parent) :
	QItemDelegate(parent)
{
}


void MSIMSlaveItemDelegate::paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const {
	if (index.column() != 0) {
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
		QStyleOptionViewItemV4 options = option;

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
}


QWidget * MSIMSlaveItemDelegate::createEditor ( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const {
	if (index.column() == 0) {
		QColorDialog * editor = new QColorDialog(parent);
		connect(editor, SIGNAL(accepted()), this, SLOT(commitAndCloseEditor()));
		connect(editor, SIGNAL(rejected()), this, SLOT(rejectCloseEditor()));
		return editor;
	}
	else {
		return QItemDelegate::createEditor(parent, option, index);
	}
}


void MSIMSlaveItemDelegate::setEditorData ( QWidget * editor, const QModelIndex & index ) const {
	if (index.column() == 0) {
		QVariant data = index.model()->data(index, Qt::UserRole);
		QColor col = data.value<QColor>();
		QColorDialog *colorDialog = qobject_cast<QColorDialog *>(editor);
		colorDialog->setCurrentColor(col);
	}
	else {
		QItemDelegate::setEditorData(editor, index);
	}
}


void MSIMSlaveItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
	if (index.column() == 0) {
		QColorDialog *colorDialog = qobject_cast<QColorDialog *>(editor);
		model->setData(index, colorDialog->currentColor(), Qt::UserRole);
	}
	else {
		QItemDelegate::setModelData(editor, model, index);
	}
}


void MSIMSlaveItemDelegate::rejectCloseEditor() {
	QColorDialog *editor = qobject_cast<QColorDialog *>(sender());
	emit closeEditor(editor);
}


void MSIMSlaveItemDelegate::commitAndCloseEditor() {
	QColorDialog *editor = qobject_cast<QColorDialog *>(sender());
	emit commitData(editor);
	emit closeEditor(editor);
}
