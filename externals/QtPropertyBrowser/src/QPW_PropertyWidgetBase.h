#ifndef QPW_PropertyWidgetBaseH
#define QPW_PropertyWidgetBaseH

#include <QWidget>
#include <QMap>
#include <QString>

#include <vector>

#include "qtvariantproperty.h"

#include "QPW_VariantPropertyManager.h"

class QtTreePropertyBrowser;
class QVBoxLayout;
class QEvent;

namespace QPW {

/*! Common base class for all property widgets.
*/
class PropertyWidgetBase : public QWidget {
	Q_OBJECT
public:
	explicit PropertyWidgetBase(QWidget *parent = 0);

private slots:
	/*! Removes properties */
	void onPropertyDestroyed( QtProperty* );

protected:
	/*! Adds a property to the browser and the maps. */
	void addProperty( QtVariantProperty * property, const int &id );

	/*! Maps the property versus its id, but does not add the property (it has to
		be added manually by calling parentProp->addSubProperty(property).
	*/
	void registerSubProperty(QtVariantProperty * property, const int &id );

	/*! Removes a property from the browser and the maps. */
	void removeProperty( QtVariantProperty * property );

	/*! Removes all properties. */
	void clearProperties();


	/*! The manager for the propertys. */
	VariantPropertyManager					*m_variantManager;

	/*! The actual property editor. */
	QtTreePropertyBrowser					*m_propertyEditor;

	/*! The layout of the widget. Add other userdefined elements.*/
	QVBoxLayout								*m_layout;

	/*! Mapping of properties. */
	QMap< QtProperty*, unsigned int >		m_propertyToId;
	QMap< unsigned int, QtProperty* >		m_idToProperty;
	QMap< QtProperty*, bool >				m_propertyToExpanded;

private:
	/*! Updates the expand state in the m_idToExpanded map according to the current property table state. */
	void updateExpandState();

};

} // namespace QPW

#endif // PropertyWidgetBaseH
