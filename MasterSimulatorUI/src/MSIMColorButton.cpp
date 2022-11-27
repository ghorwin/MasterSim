#include "MSIMColorButton.h"

#include <QPainter>
#include <QStyleOptionButton>
#include <QColorDialog>

#if QT_VERSION >= 0x050000
#include <qdrawutil.h>
#endif

MSIMColorButton::MSIMColorButton(QWidget *parent) :
	QPushButton(parent),
	m_color(Qt::red),
	m_readOnly(false),
	m_dontUseNativeDialog(true)
{
	connect(this, SIGNAL(clicked()), this, SLOT(onClicked()));
}


void MSIMColorButton::setColor(const QColor & c) {
	m_color = c;
	update();
}


void MSIMColorButton::setReadOnly(bool readOnly) {
	setEnabled(!readOnly);
	m_readOnly = readOnly;
}


void MSIMColorButton::setEnabled(bool enabled) {
	m_readOnly = false;
	QPushButton::setEnabled(enabled);
}


void MSIMColorButton::setDontUseNativeDialog(bool dontUseNativeDialog) {
	m_dontUseNativeDialog = dontUseNativeDialog;
}


void MSIMColorButton::paintEvent( QPaintEvent* e) {
	// draw default button
	QPushButton::paintEvent(e);

	// now draw the color
	QPainter painter(this);
	QStyle *style = QWidget::style();
	Q_ASSERT(style != nullptr);
	QStyleOptionButton opt;
	opt.initFrom(this);
	opt.state |= isDown() ? QStyle::State_Sunken : QStyle::State_Raised;
	opt.features = QStyleOptionButton::None;
	if (isDefault())
		opt.features |= QStyleOptionButton::DefaultButton;
	opt.text.clear();
	opt.icon = QIcon();


	QRect labelRect = style->subElementRect( QStyle::SE_PushButtonContents, &opt, this );
	int shift = style->pixelMetric( QStyle::PM_ButtonMargin, &opt, this ) / 2;
	labelRect.adjust(shift, shift, -shift, -shift);
	int x, y, w, h;
	labelRect.getRect(&x, &y, &w, &h);

	if (isChecked() || isDown()) {
		x += style->pixelMetric( QStyle::PM_ButtonShiftHorizontal, &opt, this );
		y += style->pixelMetric( QStyle::PM_ButtonShiftVertical, &opt, this );
	}

	// special handling
	QColor fillCol = m_color;
	if (!m_readOnly && !isEnabled())
		fillCol = palette().color(backgroundRole());
	qDrawShadePanel( &painter, x, y, w, h, palette(), true, 1, nullptr);
	if ( fillCol.isValid() ) {
		painter.fillRect(QRect( x+1, y+1, w-2, h-2), QBrush(fillCol));
	}
}


void MSIMColorButton::onClicked() {
	QColorDialog dlg(this);

	if (m_dontUseNativeDialog)
		dlg.setOption(QColorDialog::DontUseNativeDialog, true);

	dlg.setCurrentColor(m_color);
	if (dlg.exec() == QDialog::Accepted) {
		QColor newColor = dlg.currentColor();
		if (newColor != m_color) {
			setColor(dlg.currentColor());
			emit colorChanged();
		}
	}
}
