#include "MSIMConversion.h"

IBK::Unit s2Unit(const QString & str) {
	return IBK::Unit(str.toUtf8().data());
}

