/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qtpropertybrowserutils.h"

#if QT_VERSION >= 0x050000
  #include <QtWidgets/QApplication>
  #include <QtWidgets/QHBoxLayout>
  #include <QtWidgets/QCheckBox>
  #include <QtWidgets/QLineEdit>
  #include <QtWidgets/QMenu>
#else
  #include <QApplication>
  #include <QHBoxLayout>
  #include <QCheckBox>
  #include <QLineEdit>
  #include <QMenu>
  #include <QStyleOption>
#endif

#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
#include <QtCore/QLocale>

#include <QDebug>

QT_BEGIN_NAMESPACE

QtCursorDatabase::QtCursorDatabase()
{
	appendCursor(Qt::ArrowCursor, QCoreApplication::translate("QtCursorDatabase", "Arrow"),
				 QIcon(QLatin1String(":/qt-project.org/qtpropertybrowser/images/cursor-arrow.png")));
	appendCursor(Qt::UpArrowCursor, QCoreApplication::translate("QtCursorDatabase", "Up Arrow"),
				 QIcon(QLatin1String(":/qt-project.org/qtpropertybrowser/images/cursor-uparrow.png")));
	appendCursor(Qt::CrossCursor, QCoreApplication::translate("QtCursorDatabase", "Cross"),
				 QIcon(QLatin1String(":/qt-project.org/qtpropertybrowser/images/cursor-cross.png")));
	appendCursor(Qt::WaitCursor, QCoreApplication::translate("QtCursorDatabase", "Wait"),
				 QIcon(QLatin1String(":/qt-project.org/qtpropertybrowser/images/cursor-wait.png")));
	appendCursor(Qt::IBeamCursor, QCoreApplication::translate("QtCursorDatabase", "IBeam"),
				 QIcon(QLatin1String(":/qt-project.org/qtpropertybrowser/images/cursor-ibeam.png")));
	appendCursor(Qt::SizeVerCursor, QCoreApplication::translate("QtCursorDatabase", "Size Vertical"),
				 QIcon(QLatin1String(":/qt-project.org/qtpropertybrowser/images/cursor-sizev.png")));
	appendCursor(Qt::SizeHorCursor, QCoreApplication::translate("QtCursorDatabase", "Size Horizontal"),
				 QIcon(QLatin1String(":/qt-project.org/qtpropertybrowser/images/cursor-sizeh.png")));
	appendCursor(Qt::SizeFDiagCursor, QCoreApplication::translate("QtCursorDatabase", "Size Backslash"),
				 QIcon(QLatin1String(":/qt-project.org/qtpropertybrowser/images/cursor-sizef.png")));
	appendCursor(Qt::SizeBDiagCursor, QCoreApplication::translate("QtCursorDatabase", "Size Slash"),
				 QIcon(QLatin1String(":/qt-project.org/qtpropertybrowser/images/cursor-sizeb.png")));
	appendCursor(Qt::SizeAllCursor, QCoreApplication::translate("QtCursorDatabase", "Size All"),
				 QIcon(QLatin1String(":/qt-project.org/qtpropertybrowser/images/cursor-sizeall.png")));
	appendCursor(Qt::BlankCursor, QCoreApplication::translate("QtCursorDatabase", "Blank"),
				 QIcon());
	appendCursor(Qt::SplitVCursor, QCoreApplication::translate("QtCursorDatabase", "Split Vertical"),
				 QIcon(QLatin1String(":/qt-project.org/qtpropertybrowser/images/cursor-vsplit.png")));
	appendCursor(Qt::SplitHCursor, QCoreApplication::translate("QtCursorDatabase", "Split Horizontal"),
				 QIcon(QLatin1String(":/qt-project.org/qtpropertybrowser/images/cursor-hsplit.png")));
	appendCursor(Qt::PointingHandCursor, QCoreApplication::translate("QtCursorDatabase", "Pointing Hand"),
				 QIcon(QLatin1String(":/qt-project.org/qtpropertybrowser/images/cursor-hand.png")));
	appendCursor(Qt::ForbiddenCursor, QCoreApplication::translate("QtCursorDatabase", "Forbidden"),
				 QIcon(QLatin1String(":/qt-project.org/qtpropertybrowser/images/cursor-forbidden.png")));
	appendCursor(Qt::OpenHandCursor, QCoreApplication::translate("QtCursorDatabase", "Open Hand"),
				 QIcon(QLatin1String(":/qt-project.org/qtpropertybrowser/images/cursor-openhand.png")));
	appendCursor(Qt::ClosedHandCursor, QCoreApplication::translate("QtCursorDatabase", "Closed Hand"),
				 QIcon(QLatin1String(":/qt-project.org/qtpropertybrowser/images/cursor-closedhand.png")));
	appendCursor(Qt::WhatsThisCursor, QCoreApplication::translate("QtCursorDatabase", "What's This"),
				 QIcon(QLatin1String(":/qt-project.org/qtpropertybrowser/images/cursor-whatsthis.png")));
	appendCursor(Qt::BusyCursor, QCoreApplication::translate("QtCursorDatabase", "Busy"),
				 QIcon(QLatin1String(":/qt-project.org/qtpropertybrowser/images/cursor-busy.png")));
}

void QtCursorDatabase::clear()
{
	m_cursorNames.clear();
	m_cursorIcons.clear();
	m_valueToCursorShape.clear();
	m_cursorShapeToValue.clear();
}

void QtCursorDatabase::appendCursor(Qt::CursorShape shape, const QString &name, const QIcon &icon)
{
	if (m_cursorShapeToValue.contains(shape))
		return;
	const int value = m_cursorNames.count();
	m_cursorNames.append(name);
	m_cursorIcons.insert(value, icon);
	m_valueToCursorShape.insert(value, shape);
	m_cursorShapeToValue.insert(shape, value);
}

QStringList QtCursorDatabase::cursorShapeNames() const
{
	return m_cursorNames;
}

QMap<int, QIcon> QtCursorDatabase::cursorShapeIcons() const
{
	return m_cursorIcons;
}

QString QtCursorDatabase::cursorToShapeName(const QCursor &cursor) const
{
	int val = cursorToValue(cursor);
	if (val >= 0)
		return m_cursorNames.at(val);
	return QString();
}

QIcon QtCursorDatabase::cursorToShapeIcon(const QCursor &cursor) const
{
	int val = cursorToValue(cursor);
	return m_cursorIcons.value(val);
}

int QtCursorDatabase::cursorToValue(const QCursor &cursor) const
{
#ifndef QT_NO_CURSOR
	Qt::CursorShape shape = cursor.shape();
	if (m_cursorShapeToValue.contains(shape))
		return m_cursorShapeToValue[shape];
#endif
	return -1;
}

#ifndef QT_NO_CURSOR
QCursor QtCursorDatabase::valueToCursor(int value) const
{
	if (m_valueToCursorShape.contains(value))
		return QCursor(m_valueToCursorShape[value]);
	return QCursor();
}
#endif

QPixmap QtPropertyBrowserUtils::brushValuePixmap(const QBrush &b)
{
	QImage img(16, 16, QImage::Format_ARGB32_Premultiplied);
	img.fill(0);

	QPainter painter(&img);
	painter.setCompositionMode(QPainter::CompositionMode_Source);
	painter.fillRect(0, 0, img.width(), img.height(), b);
	QColor color = b.color();
	if (color.alpha() != 255) { // indicate alpha by an inset
		QBrush  opaqueBrush = b;
		color.setAlpha(255);
		opaqueBrush.setColor(color);
		painter.fillRect(img.width() / 4, img.height() / 4,
						 img.width() / 2, img.height() / 2, opaqueBrush);
	}
	painter.end();
	return QPixmap::fromImage(img);
}

QIcon QtPropertyBrowserUtils::brushValueIcon(const QBrush &b)
{
	return QIcon(brushValuePixmap(b));
}

QPixmap QtPropertyBrowserUtils::penValuePixmap(const QPen &p)
{
	QImage img(16, 16, QImage::Format_Mono);
	img.fill(0);

	QPainter painter(&img);
	painter.setCompositionMode(QPainter::CompositionMode_Source);
	QBrush brush(Qt::white);
	painter.fillRect(0, 0, img.width(), img.height(), brush);
	painter.setPen( p );
	painter.drawLine( 0, img.height()/2, img.width(), img.height()/2);
	painter.end();
	return QPixmap::fromImage(img);
}

QIcon QtPropertyBrowserUtils::penValueIcon(const QPen &p) {
	return QIcon(penValuePixmap(p));
}

QPixmap QtPropertyBrowserUtils::markerValuePixmap(const int &m) {

	QSize iconSize(16,16); // should match the size used for the icons in the tree view, see qttreepropertybrowser.cpp line 459
	QImage img(iconSize.width(), iconSize.height(), QImage::Format_Mono);

	QPainter painter(&img);
	QBrush brush(Qt::white);
	QRect r = QRect(1, 1, iconSize.width()-1, iconSize.height()-1);
	painter.fillRect(0, 0, img.width(), img.height(), brush);
	painter.setPen( QPen(Qt::black) );
	painter.setBrush( Qt::black );
	painter.setRenderHint(QPainter::Antialiasing, true);

//	qDebug() << img.width() << "x" << img.height();
////	painter.drawRect(r);
//	painter.end();
//	QPixmap p = QPixmap::fromImage(img);
//	qDebug() << "pixmap: " << p.width() << "x" << p.height();
////	QIcon ic(p);
////	qDebug() << "icon: " << ic.width() << "x" << ic.height();
//	return p;

	switch(m)
	{
		case 0: // no Symbol
			break;
		case 1: // Ellipse
			painter.drawEllipse(r);
			break;
		case 2: //Rect:
			painter.drawRect(r);
			break;
		case 3: //Diamond:
		{
			const int w2 = r.width() / 2;
			const int h2 = r.height() / 2;

			QPolygon pa(4);
			pa.setPoint(0, r.x() + w2, r.y());
			pa.setPoint(1, r.right(), r.y() + h2);
			pa.setPoint(2, r.x() + w2, r.bottom());
			pa.setPoint(3, r.x(), r.y() + h2);
			painter.drawPolygon(pa);
			break;
		}
		case 4: //:Triangle:
		case 5: //:UTriangle:
		{
			const int w2 = r.width() / 2;

			QPolygon pa(3);
			pa.setPoint(0, r.x() + w2, r.y());
			pa.setPoint(1, r.right(), r.bottom());
			pa.setPoint(2, r.x(), r.bottom());
			painter.drawPolygon(pa);
			break;
		}
		case 6: //:DTriangle:
		{
			const int w2 = r.width() / 2;

			QPolygon pa(3);
			pa.setPoint(0, r.x(), r.y());
			pa.setPoint(1, r.right(), r.y());
			pa.setPoint(2, r.x() + w2, r.bottom());
			painter.drawPolygon(pa);
			break;
		}
		case 7: //:RTriangle:
		{
			const int h2 = r.height() / 2;

			QPolygon pa(3);
			pa.setPoint(0, r.x(), r.y());
			pa.setPoint(1, r.right(), r.y() + h2);
			pa.setPoint(2, r.x(), r.bottom());
			painter.drawPolygon(pa);
			break;
		}
		case 8: //:LTriangle:
		{
			const int h2 = r.height() / 2;

			QPolygon pa(3);
			pa.setPoint(0, r.right(), r.y());
			pa.setPoint(1, r.x(), r.y() + h2);
			pa.setPoint(2, r.right(), r.bottom());
			painter.drawPolygon(pa);
			break;
		}
		case 9: //Cross:
		{
			const int w2 = r.width() / 2;
			const int h2 = r.height() / 2;

			painter.drawLine(r.x() + w2, r.y(),
				r.x() + w2, r.bottom());
			painter.drawLine(r.x(), r.y() + h2,
				r.right(), r.y() + h2);
			break;
		}
		case 10: //XCross:
		{
			painter.drawLine(r.left(), r.top(),
				r.right(), r.bottom());
			painter.drawLine(r.left(), r.bottom(),
				r.right(), r.top());
			break;
		}
		case 11: //:HLine:
		{
			const int h2 = r.height() / 2;
			painter.drawLine(r.left(), r.top() + h2,
					r.right(), r.top() + h2);
			break;
		}
		case 12: //:VLine:
		{
			const int w2 = r.width() / 2;
			painter.drawLine(r.left() + w2, r.top(),
					r.left() + w2, r.bottom());
			break;
		}
		case 13: //:Star1:
		{
			const double sqrt1_2 = 0.70710678118654752440; /* 1/sqrt(2) */

			const int w2 = r.width() / 2;
			const int h2 = r.height() / 2;
			const int d1  = (int)( (double)w2 * (1.0 - sqrt1_2) );

			painter.drawLine(r.left() + d1, r.top() + d1,
					r.right() - d1, r.bottom() - d1);
			painter.drawLine(r.left() + d1, r.bottom() - d1,
					r.right() - d1, r.top() + d1);
			painter.drawLine(r.left() + w2, r.top(),
					r.left() + w2, r.bottom());
			painter.drawLine(r.left(), r.top() + h2,
					r.right(), r.top() + h2);
			break;
		}
		case 14: //:Star2:
		{
			const int w = r.width();
			const int side = (int)(((double)r.width() * (1.0 - 0.866025)) /
				2.0);  // 0.866025 = cos(30°)
			const int h4 = r.height() / 4;
			const int h2 = r.height() / 2;
			const int h34 = (r.height() * 3) / 4;

			QPolygon pa(12);
			pa.setPoint(0, r.left() + (w / 2), r.top());
			pa.setPoint(1, r.right() - (side + (w - 2 * side) / 3),
				r.top() + h4 );
			pa.setPoint(2, r.right() - side, r.top() + h4);
			pa.setPoint(3, r.right() - (side + (w / 2 - side) / 3),
				r.top() + h2 );
			pa.setPoint(4, r.right() - side, r.top() + h34);
			pa.setPoint(5, r.right() - (side + (w - 2 * side) / 3),
				r.top() + h34 );
			pa.setPoint(6, r.left() + (w / 2), r.bottom());
			pa.setPoint(7, r.left() + (side + (w - 2 * side) / 3),
				r.top() + h34 );
			pa.setPoint(8, r.left() + side, r.top() + h34);
			pa.setPoint(9, r.left() + (side + (w / 2 - side) / 3),
				r.top() + h2 );
			pa.setPoint(10, r.left() + side, r.top() + h4);
			pa.setPoint(11, r.left() + (side + (w - 2 * side) / 3),
				r.top() + h4 );
			painter.drawPolygon(pa);
			break;
		}
		case 15: //:Hexagon:
		{
			const int w2 = r.width() / 2;
			const int side = (int)(((double)r.width() * (1.0 - 0.866025)) /
				2.0);  // 0.866025 = cos(30°)
			const int h4 = r.height() / 4;
			const int h34 = (r.height() * 3) / 4;

			QPolygon pa(6);
			pa.setPoint(0, r.left() + w2, r.top());
			pa.setPoint(1, r.right() - side, r.top() + h4);
			pa.setPoint(2, r.right() - side, r.top() + h34);
			pa.setPoint(3, r.left() + w2, r.bottom());
			pa.setPoint(4, r.left() + side, r.top() + h34);
			pa.setPoint(5, r.left() + side, r.top() + h4);
			painter.drawPolygon(pa);
			break;
		}
		default:;
	}

	painter.end();
	return QPixmap::fromImage(img);
}

QIcon QtPropertyBrowserUtils::markerValueIcon(const int &m) {
	return QIcon(markerValuePixmap(m));
}

QString QtPropertyBrowserUtils::colorValueText(const QColor &c)
{
	return QCoreApplication::translate("QtPropertyBrowserUtils", "[%1, %2, %3] (%4)")
		   .arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alpha());
}

QPixmap QtPropertyBrowserUtils::fontValuePixmap(const QFont &font)
{
	QFont f = font;
	QImage img(16, 16, QImage::Format_ARGB32_Premultiplied);
	img.fill(0);
	QPainter p(&img);
	p.setRenderHint(QPainter::TextAntialiasing, true);
	p.setRenderHint(QPainter::Antialiasing, true);
	f.setPointSize(13);
	p.setFont(f);
	QTextOption t;
	t.setAlignment(Qt::AlignCenter);
	p.drawText(QRect(0, 0, 16, 16), QString(QLatin1Char('A')), t);
	return QPixmap::fromImage(img);
}

QIcon QtPropertyBrowserUtils::fontValueIcon(const QFont &f)
{
	return QIcon(fontValuePixmap(f));
}

QString QtPropertyBrowserUtils::fontValueText(const QFont &f)
{
	return QCoreApplication::translate("QtPropertyBrowserUtils", "[%1, %2]")
		   .arg(f.family()).arg(f.pointSize());
}

QString QtPropertyBrowserUtils::dateFormat()
{
	QLocale loc;
	return loc.dateFormat(QLocale::ShortFormat);
}

QString QtPropertyBrowserUtils::timeFormat()
{
	QLocale loc;
	// ShortFormat is missing seconds on UNIX.
	return loc.timeFormat(QLocale::LongFormat);
}

QString QtPropertyBrowserUtils::dateTimeFormat()
{
	QString format = dateFormat();
	format += QLatin1Char(' ');
	format += timeFormat();
	return format;
}

QtBoolEdit::QtBoolEdit(QWidget *parent) :
	QWidget(parent),
	m_checkBox(new QCheckBox(this)),
	m_textVisible(false)
{
	QHBoxLayout *lt = new QHBoxLayout;
	if (QApplication::layoutDirection() == Qt::LeftToRight)
		lt->setContentsMargins(4, 0, 0, 0);
	else
		lt->setContentsMargins(0, 0, 4, 0);
	lt->addWidget(m_checkBox);
	setLayout(lt);
	connect(m_checkBox, SIGNAL(toggled(bool)), this, SIGNAL(toggled(bool)));
	setFocusProxy(m_checkBox);
	m_checkBox->setText(QString()); // Hack by Andreas, we don't want "True" or "False" for a checkbox
}

void QtBoolEdit::setTextVisible(bool textVisible)
{
	if (m_textVisible == textVisible)
		return;

	m_textVisible = textVisible;
	if (m_textVisible)
		m_checkBox->setText(isChecked() ? tr("True") : tr("False"));
	else
		m_checkBox->setText(QString());
}

Qt::CheckState QtBoolEdit::checkState() const
{
	return m_checkBox->checkState();
}

void QtBoolEdit::setCheckState(Qt::CheckState state)
{
	m_checkBox->setCheckState(state);
}

bool QtBoolEdit::isChecked() const
{
	return m_checkBox->isChecked();
}

void QtBoolEdit::setChecked(bool c)
{
	m_checkBox->setChecked(c);
	if (!m_textVisible)
		return;
	m_checkBox->setText(isChecked() ? tr("True") : tr("False"));
}

bool QtBoolEdit::blockCheckBoxSignals(bool block)
{
	return m_checkBox->blockSignals(block);
}

void QtBoolEdit::mousePressEvent(QMouseEvent *event)
{
	if (event->buttons() == Qt::LeftButton) {
		m_checkBox->click();
		event->accept();
	} else {
		QWidget::mousePressEvent(event);
	}
}

#if QT_VERSION < 0x050000

QtKeySequenceEdit::QtKeySequenceEdit(QWidget *parent)
	: QWidget(parent), m_num(0), m_lineEdit(new QLineEdit(this))
{
	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->addWidget(m_lineEdit);
	layout->setMargin(0);
	m_lineEdit->installEventFilter(this);
	m_lineEdit->setReadOnly(true);
	m_lineEdit->setFocusProxy(this);
	setFocusPolicy(m_lineEdit->focusPolicy());
	setAttribute(Qt::WA_InputMethodEnabled);
}

bool QtKeySequenceEdit::eventFilter(QObject *o, QEvent *e)
{
	if (o == m_lineEdit && e->type() == QEvent::ContextMenu) {
		QContextMenuEvent *c = static_cast<QContextMenuEvent *>(e);
		QMenu *menu = m_lineEdit->createStandardContextMenu();
		const QList<QAction *> actions = menu->actions();
		QListIterator<QAction *> itAction(actions);
		while (itAction.hasNext()) {
			QAction *action = itAction.next();
			action->setShortcut(QKeySequence());
			QString actionString = action->text();
			const int pos = actionString.lastIndexOf(QLatin1Char('\t'));
			if (pos > 0)
				actionString.remove(pos, actionString.length() - pos);
			action->setText(actionString);
		}
		QAction *actionBefore = 0;
		if (actions.count() > 0)
			actionBefore = actions[0];
		QAction *clearAction = new QAction(tr("Clear Shortcut"), menu);
		menu->insertAction(actionBefore, clearAction);
		menu->insertSeparator(actionBefore);
		clearAction->setEnabled(!m_keySequence.isEmpty());
		connect(clearAction, SIGNAL(triggered()), this, SLOT(slotClearShortcut()));
		menu->exec(c->globalPos());
		delete menu;
		e->accept();
		return true;
	}

	return QWidget::eventFilter(o, e);
}

void QtKeySequenceEdit::slotClearShortcut()
{
	if (m_keySequence.isEmpty())
		return;
	setKeySequence(QKeySequence());
	emit keySequenceChanged(m_keySequence);
}

void QtKeySequenceEdit::handleKeyEvent(QKeyEvent *e)
{
	int nextKey = e->key();
	if (nextKey == Qt::Key_Control || nextKey == Qt::Key_Shift ||
			nextKey == Qt::Key_Meta || nextKey == Qt::Key_Alt ||
			nextKey == Qt::Key_Super_L || nextKey == Qt::Key_AltGr)
		return;

	nextKey |= translateModifiers(e->modifiers(), e->text());
	int k0 = m_keySequence[0];
	int k1 = m_keySequence[1];
	int k2 = m_keySequence[2];
	int k3 = m_keySequence[3];
	switch (m_num) {
		case 0: k0 = nextKey; k1 = 0; k2 = 0; k3 = 0; break;
		case 1: k1 = nextKey; k2 = 0; k3 = 0; break;
		case 2: k2 = nextKey; k3 = 0; break;
		case 3: k3 = nextKey; break;
		default: break;
	}
	++m_num;
	if (m_num > 3)
		m_num = 0;
	m_keySequence = QKeySequence(k0, k1, k2, k3);
	m_lineEdit->setText(m_keySequence.toString(QKeySequence::NativeText));
	e->accept();
	emit keySequenceChanged(m_keySequence);
}

void QtKeySequenceEdit::setKeySequence(const QKeySequence &sequence)
{
	if (sequence == m_keySequence)
		return;
	m_num = 0;
	m_keySequence = sequence;
	m_lineEdit->setText(m_keySequence.toString(QKeySequence::NativeText));
}

QKeySequence QtKeySequenceEdit::keySequence() const
{
	return m_keySequence;
}

int QtKeySequenceEdit::translateModifiers(Qt::KeyboardModifiers state, const QString &text) const
{
	int result = 0;
	if ((state & Qt::ShiftModifier) && (text.size() == 0 || !text.at(0).isPrint() || text.at(0).isLetter() || text.at(0).isSpace()))
		result |= Qt::SHIFT;
	if (state & Qt::ControlModifier)
		result |= Qt::CTRL;
	if (state & Qt::MetaModifier)
		result |= Qt::META;
	if (state & Qt::AltModifier)
		result |= Qt::ALT;
	return result;
}

void QtKeySequenceEdit::focusInEvent(QFocusEvent *e)
{
	m_lineEdit->event(e);
	m_lineEdit->selectAll();
	QWidget::focusInEvent(e);
}

void QtKeySequenceEdit::focusOutEvent(QFocusEvent *e)
{
	m_num = 0;
	m_lineEdit->event(e);
	QWidget::focusOutEvent(e);
}

void QtKeySequenceEdit::keyPressEvent(QKeyEvent *e)
{
	handleKeyEvent(e);
	e->accept();
}

void QtKeySequenceEdit::keyReleaseEvent(QKeyEvent *e)
{
	m_lineEdit->event(e);
}

void QtKeySequenceEdit::paintEvent(QPaintEvent *)
{
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

bool QtKeySequenceEdit::event(QEvent *e)
{
	if (e->type() == QEvent::Shortcut ||
			e->type() == QEvent::ShortcutOverride  ||
			e->type() == QEvent::KeyRelease) {
		e->accept();
		return true;
	}
	return QWidget::event(e);
}


#endif

QT_END_NAMESPACE

#include "moc_qtpropertybrowserutils.cpp"
