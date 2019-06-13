#include "QPW_PropertyWidgetBase.h"

#include "qttreepropertybrowser.h"

#include <QVBoxLayout>
#include <QList>
#include <QEvent>

namespace QPW {

PropertyWidgetBase::PropertyWidgetBase(QWidget *parent) :
	QWidget(parent),
	m_variantManager( new VariantPropertyManager(this) ),
	m_propertyEditor( new QtTreePropertyBrowser(this) ),
	m_layout( new QVBoxLayout(this) )
{

	// create property manager, editor and factory
	QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory(this);
	QtVariantPropertyManager *varMan = m_variantManager;
	m_propertyEditor->setFactoryForManager( varMan, variantFactory );
//	m_propertyEditor->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );
//	m_propertyEditor->setResizeMode( QtTreePropertyBrowser::ResizeToContents );

	connect(m_variantManager, SIGNAL(propertyDestroyed(QtProperty*)),
			this, SLOT(onPropertyDestroyed(QtProperty*)));


	m_layout->addWidget( m_propertyEditor );
	m_layout->addStretch(1);
	m_layout->setContentsMargins(0,0,0,0);

	setLayout(m_layout);
}


void PropertyWidgetBase::onPropertyDestroyed( QtProperty* property ) {

	QMap< QtProperty*, unsigned int >::iterator sit = m_propertyToId.find( property );
	if ( sit != m_propertyToId.end() ) {
		m_idToProperty[sit.value()] = 0;
		m_propertyToId.erase( sit );
	}

	QMap< QtProperty*, bool >::iterator bit = m_propertyToExpanded.find( property );
	if ( bit != m_propertyToExpanded.end() ) {
		m_propertyToExpanded.erase( bit );
	}
}


void PropertyWidgetBase::updateExpandState() {

	// get item list and iterator
	QList< QtBrowserItem * > list = m_propertyEditor->topLevelItems();
	QListIterator< QtBrowserItem * > iterator( list );

	// go trough list and act
	while( iterator.hasNext() ){

		QtBrowserItem	*item = iterator.next();
		QtProperty		*itemProperty = item->property();
		m_propertyToExpanded[ itemProperty ] = m_propertyEditor->isExpanded( item );
	}
}


void PropertyWidgetBase::addProperty( QtVariantProperty * property, const int &id ){

	m_propertyToId[ property ] = id;
	m_idToProperty[ id ] = property;

	QtBrowserItem *item = m_propertyEditor->addProperty( property );
	if ( m_propertyToExpanded.contains(property) ){
		m_propertyEditor->setExpanded( item, m_propertyToExpanded[property] );
	}
}


void PropertyWidgetBase::registerSubProperty(QtVariantProperty * property, const int &id ) {
	m_propertyToId[ property ] = id;
	m_idToProperty[ id ] = property;
}


void PropertyWidgetBase::removeProperty( QtVariantProperty * property ) {

	QMap< QtProperty*, bool >::iterator bit = m_propertyToExpanded.find( property );
	if ( bit != m_propertyToExpanded.end() ) {
		m_propertyToExpanded.erase( bit );
	}

	QMap< QtProperty*, unsigned int >::iterator sit = m_propertyToId.find( property );
	if ( sit != m_propertyToId.end() ) {
		m_idToProperty[sit.value()] = 0;
		m_propertyToId.erase( sit );
		delete property;
	}
}


void PropertyWidgetBase::clearProperties() {

	// also clear maps
	m_idToProperty.clear();
	m_propertyToId.clear();
}


} // namespace QPW


#include "moc_QPW_PropertyWidgetBase.cpp"
