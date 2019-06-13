#include "MSIMSlavePropertyWidget.h"

#include <qttreepropertybrowser.h>
#include <qtpropertybrowserutils.h>

#include <QTreeWidget>

#include <MSIM_Project.h>

#include "MSIMMainWindow.h"

MSIMSlavePropertyWidget::MSIMSlavePropertyWidget(QWidget * parent) :
	QPW::PropertyWidgetBase(parent)
{
	disconnect( m_variantManager, SIGNAL(valueChanged(QtProperty*,QVariant)),
				m_variantManager, SLOT(slotValueChanged(QtProperty*,QVariant)));

	setMinimumWidth( 300 );

	// first column in property editor (labels) takes only as much space as needed
	m_propertyEditor->setResizeMode(QtTreePropertyBrowser::ResizeToContents);
	m_propertyEditor->setFixHeightToContents(true);
}


void MSIMSlavePropertyWidget::updateProperties(int selectedSlave) {
	m_currentSlaveIndex = selectedSlave;

	m_propertyEditor->clear();
	m_propertyEditor->setEnabled(false);
	if (selectedSlave == -1)
		return;

	// fill in all parameters defined in FMU slave
	// get slave instance
	const MASTER_SIM::Project & p = project();
	if (selectedSlave >= p.m_simulators.size())
		return;

	m_propertyEditor->setEnabled(false); // TODO : for now we leave this off
	std::string slaveName = project().m_simulators[selectedSlave].m_name;

	// create default property
	QtVariantProperty * propertyResultRootDir = m_variantManager->addProperty( QVariant::String, tr("ResultRootDir") );
	addProperty( propertyResultRootDir, 0 );
	propertyResultRootDir->setToolTip( tr("MasterSim will automatically pass the directory generated for the slave, such that output data can be stored therein.") );

	// attempt to find FMU definition
	try {
		const MASTER_SIM::ModelDescription & modelDesc = MSIMMainWindow::instance().modelDescription(slaveName);
		// process model description and create properties for all parameters
		for (unsigned int i=0; i<modelDesc.m_variables.size(); ++i) {
			const MASTER_SIM::FMIVariable & var = modelDesc.m_variables[i];
			if (var.m_causality == MASTER_SIM::FMIVariable::C_PARAMETER) {
				QtVariantProperty * varProp = m_variantManager->addProperty( QVariant::String, QString::fromStdString(var.m_name) );
				addProperty( varProp, 1 );
				// todo - set "default/auto-text" property propertyResultRootDir->setAttribute()
				varProp->setToolTip( QString::fromStdString(var.m_description) );
				varProp->setValue( QString::fromStdString(var.m_startValue) );
			}
		}

	}
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		m_propertyEditor->clear();
		m_propertyEditor->setEnabled(false);
	}
}


void MSIMSlavePropertyWidget::valueChanged( QtProperty * prop, const QVariant & value ) {

	// Mind: when a subproperty is edited (point size of a font), the valueChanged()
	//       signal is emitted twice - once for the complex property (i.e. QFont) and
	//       afterwards for the single property as well (i.e. the point size as integer)
	//       We are only interested in the complex property and only that is
	//       registered in m_propertyToId. All others are silently ignored.
	if (!m_propertyToId.contains(prop) )
		return;

	// prevent call of valueChanged() when we modify property items below
	disconnect( m_variantManager, SIGNAL(valueChanged(QtProperty*,QVariant)), this, SLOT(valueChanged(QtProperty*,QVariant)));

	m_blockUpdateProperties = true;

	// get id for this property
	int role = m_propertyToId[prop];

	const MASTER_SIM::Project & p = project();
	Q_ASSERT(m_currentSlaveIndex < p.m_simulators.size());

	switch (role) {

		case 0 :
			// populate parameter change undo action
//			p.m_simulators[m_currentSlaveIndex].m_parameters["ResultRootDir"] = value.toString().toStdString();
			break;

		default: ;
	}

	// reconnect the change signal
	connect( m_variantManager, SIGNAL(valueChanged(QtProperty*,QVariant)), this, SLOT(valueChanged(QtProperty*,QVariant)));

	m_blockUpdateProperties = false;

}

