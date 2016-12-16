#include "MSIMConversion.h"

#include <QWidget>
#include <QLocale>
#include <QLineEdit>
#include <QComboBox>
#include <QMessageBox>
#include <QApplication>

IBK::Unit s2Unit(const QString & str) {
	return IBK::Unit(str.toUtf8().data());
}


void blockMySignals(QWidget * p, bool block) {
#if 0
	// my original version
	foreach (QObject * child, p->children()) {
		QWidget * w = dynamic_cast<QWidget*>(child);
		if (w != NULL) {
			w->blockSignals(block);
			blockMySignals(w, block);
		}
	}
#else
	// version suggested by Qt Support
	QList<QWidget *> allChildren = p->findChildren<QWidget *>();
		foreach (QWidget *child, allChildren)
			child->blockSignals(block);
#endif
}


bool lineEditToParameter(QWidget * parent, const std::string & name, IBK::Parameter & p, QLineEdit * lineEdit,
						 QComboBox * unitCombo)
{
	QLocale loc;
	loc.setNumberOptions( QLocale::RejectGroupSeparator);
	bool ok;
	double val;
	val = loc.toDouble(lineEdit->text(), &ok);
	if (!ok) {
		// try with english locale
		loc = QLocale(QLocale::C);
		loc.setNumberOptions( QLocale::RejectGroupSeparator);
		val = loc.toDouble(lineEdit->text(), &ok);
		if (!ok) {
			QMessageBox::critical(parent, qApp->translate("conversion", "Invalid input"), qApp->translate("conversion", "This is not a valid number."));
			lineEdit->selectAll();
			return false;
		}
	}
	// unit conversion if combo box is given
	if (unitCombo != NULL) {
		IBK::Unit u(unitCombo->currentText().toStdString());
		p.set(name, val, u);
	}
	else {
		p.set(name, val, IBK::Unit("-"));
	}
	return true;
}
