TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
	main.cpp


	target.path = $${QWT_INSTALL_PLUGINS}
	INSTALLS += target

	macx {
		contains(QWT_CONFIG, QwtFramework) {
			QWT_LIB = qwt.framework/Versions/$${QWT_VER_MAJ}/qwt
		}
		else {
			QWT_LIB = libqwt.$${QWT_VER_MAJ}.dylib
		}
		QMAKE_POST_LINK = install_name_tool -change $${QWT_LIB} $${QWT_INSTALL_LIBS}/$${QWT_LIB} $(DESTDIR)$(TARGET)
	}
}
else {
	TEMPLATE        = subdirs # do nothing
