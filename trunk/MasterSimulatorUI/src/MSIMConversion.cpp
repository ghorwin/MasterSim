#include "MSIMConversion.h"

#include <QWidget>

IBK::Unit s2Unit(const QString & str) {
	return IBK::Unit(str.toUtf8().data());
}

void blockMySignals(QWidget * p, bool block) {
	foreach (QObject * child, p->children()) {
		QWidget * w = dynamic_cast<QWidget*>(child);
		if (w != NULL) {
			w->blockSignals(block);
			blockMySignals(w, block);
		}
	}

}

