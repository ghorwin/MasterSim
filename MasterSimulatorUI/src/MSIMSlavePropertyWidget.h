#ifndef MSIMSlavePropertyWidget_H
#define MSIMSlavePropertyWidget_H

#include <QPW_PropertyWidgetBase.h>

/*! Implementation of the property edit widget. */
class MSIMSlavePropertyWidget : public QPW::PropertyWidgetBase {
	Q_OBJECT
public:
	MSIMSlavePropertyWidget(QWidget *parent = nullptr);

	void updateProperties(int selectedSlave);

private slots:
	/*! Triggered when a value has changed. */
	void valueChanged( QtProperty * property, const QVariant & value );

private:

	/*! Used to disable the property tree update when an undo action is triggered from valueChanged(). */
	bool	m_blockUpdateProperties;
	/*! Cached index of the currently selected slave. */
	int		m_currentSlaveIndex;
};

#endif // MSIMSlavePropertyWidget_H
