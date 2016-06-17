#ifndef MSIMConversionH
#define MSIMConversionH

#include <string>

#include <QString>
#include <QColor>

#include <IBK_Path.h>
#include <IBK_Unit.h>
#include <IBK_Flag.h>
#include <IBK_Parameter.h>
#include <IBK_Color.h>

class QCheckBox;
class QWidget;
class QLineEdit;
class QComboBox;

namespace QtExt {
	class ValidatingLineEdit;
}


/*! Utility function for conversion of std::string holding an Utf8 encoded string to QString. */
inline QString utf82QString( const std::string & str ) {
	return QString::fromUtf8( str.c_str() );
}

/*! Utility function for conversion of an IBK::Path holding an Utf8 encoded file path to QString. */
inline QString utf82QString( const IBK::Path & path ) {
	return QString::fromUtf8( path.str().c_str() );
}

/*! Utility function for conversion of a QString to a trimmed std::string in utf8 encoding. */
inline std::string QString2trimmedUtf8(const QString & str) {
	return str.trimmed().toUtf8().data();
}

/*! Utility function for conversion of a QString to an IBK::Path. */
inline IBK::Path QString2Path(const QString & str) {
	return IBK::Path(str.toUtf8().data());
}

/*! Converts string to IBK::Unit. */
IBK::Unit s2Unit(const QString & str);

/*! Converts an IBK-color to QColor. */
inline QColor color2QColor(const IBK::Color & color) {
	return QColor( (QRgb)color.toQRgb() );
}

/*! Utility function that recursively calls blockSignal() on all widgets and their child widgets. */
void blockMySignals(QWidget * p, bool block);

/*! Parses value from line edit, if combo box is given (not NULL), sets the value with the IO unit and
	specifies parameter.
	In case of error shows an error message.
*/
bool lineEditToParameter(QWidget * parent, const std::string & name, IBK::Parameter & p, QLineEdit * lineEdit, QComboBox * unitCombo);

#endif // MSIMConversionH
