#include "MSIMSlaveItemDelegate.h"

#include <QPainter>
#include <QColor>
#include <QTextDocument>
#include <QColorDialog>
#include <QSpinBox>

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

	painter->setBrush(col);
	painter->drawRect(r);
}


QWidget * MSIMSlaveItemDelegate::createEditor ( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const {
	switch (index.column()) {
		case 0 : {
			QColorDialog * editor = new QColorDialog(parent);
			connect(editor, SIGNAL(accepted()), this, SLOT(commitAndCloseEditor()));
			connect(editor, SIGNAL(rejected()), this, SLOT(rejectCloseEditor()));
			return editor;
		}
		case 3 : {
			QSpinBox * editor = new QSpinBox(parent);
			editor->setMinimum(0);
			editor->setMaximum(1000);
			return editor;
		}
		default :
			return QItemDelegate::createEditor(parent, option, index);
	}
}


void MSIMSlaveItemDelegate::setEditorData ( QWidget * editor, const QModelIndex & index ) const {
	switch (index.column()) {
		case 0 : {
			QVariant data = index.model()->data(index, Qt::UserRole);
			QColor col = data.value<QColor>();
			QColorDialog *colorDialog = qobject_cast<QColorDialog *>(editor);
			colorDialog->setCurrentColor(col);
		} break;

		case 3 : {
			int value = index.model()->data(index, Qt::DisplayRole).toInt();
			QSpinBox *spinBox = qobject_cast<QSpinBox *>(editor);
			spinBox->setValue(value);
		} break;

		default :	QItemDelegate::setEditorData(editor, index);
	}
}


void MSIMSlaveItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
	switch (index.column()) {
		case 0 : {
			QColorDialog *colorDialog = qobject_cast<QColorDialog *>(editor);
			model->setData(index, colorDialog->currentColor(), Qt::UserRole);
		} break;

		case 3 : {
			QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
			spinBox->interpretText();
			int value = spinBox->value();

			model->setData(index, value, Qt::EditRole);
		} break;

		default : QItemDelegate::setModelData(editor, model, index);
	}
}


void MSIMSlaveItemDelegate::updateEditorGeometry(QWidget *editor,
	const QStyleOptionViewItem &option, const QModelIndex & index) const
{
	if (index.column() == 3)
		editor->setGeometry(option.rect);
	else
		QItemDelegate::updateEditorGeometry(editor, option, index);
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
