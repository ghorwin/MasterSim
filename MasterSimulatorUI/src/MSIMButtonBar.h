#ifndef MSIMButtonBarH
#define MSIMButtonBarH

#include <QWidget>

class QToolButton;
class QMenu;

/*!	\brief Declaration for class ButtonBar

	The button bar is the the container of the master control buttons to
	the right of the main window.
*/
class MSIMButtonBar : public QWidget {
	Q_OBJECT
public:
	/*! Default constructor. */
	MSIMButtonBar(QWidget * parent);
	/*! Default destructor. */
	~MSIMButtonBar();

	/*! Enables/disables certain buttons during simulation. */
	void setEnabled(bool enabled);

	// *** PUBLIC MEMBER VARIABLES ***
	QToolButton  * toolButtonAbout;
	QToolButton  * toolButtonNew;
	QToolButton  * toolButtonSave;
	QToolButton  * toolButtonLoad;
	QToolButton  * toolButtonAnalyze;
	QToolButton  * toolButtonLaunchPostProc;

	QToolButton  * toolButtonEditFMUs;
	QToolButton  * toolButtonEditConnections;
	QToolButton  * toolButtonSimulate;

	QToolButton  * toolButtonUndo;
	QToolButton  * toolButtonRedo;

	QToolButton  * toolButtonSwitchLanguage;
	QToolButton  * toolButtonQuit;

	/*! The language menu pointer will be set to the main programs language menu once this is set up. */
	QMenu * m_languageMenu = nullptr;

private slots:
	void onToolButtonSwitchLanguageClicked();

};

#endif // ButtonBarH
