# ---------------------------------------------------
# Project for QtPropertyBrowser library
# ---------------------------------------------------
TARGET = QtPropertyBrowser
QT += svg

greaterThan(QT_MAJOR_VERSION, 4): {
	QT += widgets
	CONFIG += c++11
}

TEMPLATE = lib

# check if 32 or 64 bit version and set prefix variable for using in output paths
greaterThan(QT_MAJOR_VERSION, 4) {
	contains(QT_ARCH, i386): {
		DIR_PREFIX =
	} else {
		DIR_PREFIX = _x64
	}
} else {
	DIR_PREFIX =
}

# using of shared libs only for non MC compiler
# MS compiler needs explicite export statements in case of shared libs
win32-msvc* {
	CONFIG += static
} else {
	CONFIG += shared
}

INCLUDEPATH += ../../src/

unix|mac {
	VER_MAJ = 1
	VER_MIN = 0
	VER_PAT = 0
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
}

DESTDIR = ../../../lib$${DIR_PREFIX}
CONFIG(debug, debug|release) {
	OBJECTS_DIR = debug$${DIR_PREFIX}
	windows {
		DLLDESTDIR = ../../../../bin/debug$${DIR_PREFIX}
	}
}
else {
	OBJECTS_DIR = release$${DIR_PREFIX}
	windows {
		DLLDESTDIR = ../../../../bin/release$${DIR_PREFIX}
	}
}

MOC_DIR = moc

HEADERS +=  ../../src/qtbuttonpropertybrowser.h  \
	../../src/qtgroupboxpropertybrowser.h  \
	../../src/qttreepropertybrowser.h \
	../../src/qteditorfactory.h  \
	../../src/qtpropertybrowser.h  \
	../../src/qtpropertymanager.h  \
	../../src/qtvariantproperty.h \
	../../src/QPW_PropertyWidgetBase.h \
	../../src/QPW_VariantPropertyManager.h \
	../../src/QPW_Style.h \
	../../src/QPW_ValidatingLineEdit.h \
	../../src/qtpropertybrowserutils.h \
	../../src/QPW_CoordinateIndexEdit.h \
	../../src/QPW_ValidatingInputBase.h

SOURCES += ../../src/qtbuttonpropertybrowser.cpp \
	 ../../src/qtgroupboxpropertybrowser.cpp \
	 ../../src/qtpropertybrowserutils.cpp \
	 ../../src/qttreepropertybrowser.cpp \
	 ../../src/qteditorfactory.cpp \
	 ../../src/qtpropertybrowser.cpp  \
	 ../../src/qtpropertymanager.cpp   \
	 ../../src/qtvariantproperty.cpp \
	../../src/QPW_PropertyWidgetBase.cpp \
	../../src/QPW_VariantPropertyManager.cpp \
	../../src/QPW_Style.cpp \
	../../src/QPW_ValidatingLineEdit.cpp \
	../../src/QPW_CoordinateIndexEdit.cpp \
	../../src/QPW_ValidatingInputBase.cpp

FORMS += \
	../../src/QPW_CoordinateIndexEdit.ui
RESOURCES += ../../resources/gfx/QtPropertyBrowser.qrc
TRANSLATIONS += ../../resources/translations/QtPropertyBrowser_de.ts
