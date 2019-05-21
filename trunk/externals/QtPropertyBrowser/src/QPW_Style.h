#ifndef QtExt_StyleH
#define QtExt_StyleH

#include <QString>
#include <QWidget>

namespace QPW {

class FeatureWidget;

/*! This class encapsulates all style-related properties that affect the appearance of widgets
	in the QPW library.

	By using the constants in this class, a uniform look-and-feel of all QPW widgets is ensured.

	If an application wishes to use different colors, the style object's static members must be set prior
	to creating any widgets in the UI since active monitoring of style changes is not implemented in the widgets.
*/
class Style {
public:

	/*! To be used for any edit fields (line edits etc.) when they are in normal/valid state. */
	static QString EditFieldBackground;
	/*! To be used as alternative edit field backgrounds, e.g. when having tables with editable lines. */
	static QString AlternativeEditFieldBackground;
	/*! Background color when input is in invalid state. */
	static QString ErrorEditFieldBackground;
	/*! Background color when input is in invalid state but for alternative row colors. */
	static QString AlternativeErrorEditFieldBackground;
	/*! Background color when input is read-only. */
	static QString ReadOnlyEditFieldBackground;

	static QString AlternativeBackgroundBright;
	static QString AlternativeBackgroundDark;
	static QString AlternativeBackgroundText;
};


} // namespace QPW


#endif // QtExt_StyleH
