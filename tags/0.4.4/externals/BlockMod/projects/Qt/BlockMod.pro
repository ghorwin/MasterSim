# ----------------------------------------------------
# Project for BlockMod Toolkit Library
# ----------------------------------------------------

TARGET = BlockMod
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QT += network xml

TEMPLATE = lib

CONFIG(debug, debug|release) {
	OBJECTS_DIR = debug
}
else {
	OBJECTS_DIR = release
}

MOC_DIR = moc
UI_DIR = ui
DESTDIR = ../../../lib

INCLUDEPATH = \
	. \
	../../src

RESOURCES += ../../resources/BlockMod.qrc

HEADERS += \
	../../src/BM_ZoomMeshGraphicsView.h \
	../../src/BM_Block.h \
	../../src/BM_Connector.h \
	../../src/BM_Socket.h \
	../../src/BM_Network.h \
	../../src/BM_Entity.h \
	../../src/BM_XMLHelpers.h \
	../../src/BM_SceneManager.h \
	../../src/BM_BlockItem.h
SOURCES += \
	../../src/BM_ZoomMeshGraphicsView.cpp \
	../../src/BM_Network.cpp \
	../../src/BM_Block.cpp \
	../../src/BM_Entity.cpp \
	../../src/BM_Socket.cpp \
	../../src/BM_XMLHelpers.cpp \
	../../src/BM_Connector.cpp \
	../../src/BM_SceneManager.cpp \
	../../src/BM_BlockItem.cpp
FORMS +=

OTHER_FILES +=


