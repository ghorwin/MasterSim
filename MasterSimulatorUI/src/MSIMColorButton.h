#ifndef MSIMColorButtonH
#define MSIMColorButtonH

#include <QPushButton>
#include <QColor>


/*! A tool button with special color background that can be used to select this color
*/
class MSIMColorButton : public QPushButton {
	Q_OBJECT
public:
	explicit MSIMColorButton(QWidget *parent = nullptr);

	/*! Sets the color and triggers a repaint. */
	void setColor(const QColor & c);

	/*! Returns the currently selected color. */
	const QColor color() const { return m_color; }

	/*! Sets a special "disabled" state - button will be disabled
		but being painted as regular.
	*/
	void setReadOnly(bool readOnly);

	/*! Overloaded to enable change of appearance (gray color). */
	void setEnabled(bool enabled);

	/*! Use this function to enable/disable use of native color dialog. */
	void setDontUseNativeDialog(bool dontUseNativeDialog);

signals:
	/*! Emitted, when color has been changed by user. */
	void colorChanged();

protected:
	virtual void paintEvent( QPaintEvent* ) override;

private slots:
	void onClicked();

private:
	/*! Holds the color to be drawn on the button. */
	QColor	m_color;

	bool	m_readOnly;
	bool	m_dontUseNativeDialog;
};



#endif // MSIMColorButtonH
