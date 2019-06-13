#ifndef MSIMSlavePropertyWidget_H
#define MSIMSlavePropertyWidget_H

#include <QPW_PropertyWidgetBase.h>

/*! Implementation of the property edit widget. */
class MSIMSlavePropertyWidget : public QPW::PropertyWidgetBase {
	Q_OBJECT
public:
	MSIMSlavePropertyWidget(QWidget *parent = nullptr);

	void updateProperties(int selectedSlave);
};

#endif // MSIMSlavePropertyWidget_H
