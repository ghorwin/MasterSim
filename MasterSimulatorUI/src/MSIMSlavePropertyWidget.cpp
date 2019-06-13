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
	m_propertyEditor->clear();
	m_propertyEditor->setEnabled(false);
	if (selectedSlave == -1)
		return;

	// fill in all parameters defined in FMU slave
	// get slave instance
	const MASTER_SIM::Project & p = project();
	if (selectedSlave >= p.m_simulators.size())
		return;

	m_propertyEditor->setEnabled(true);
	IBK::Path fmuPath = project().m_simulators[selectedSlave].m_pathToFMU;

	// create default property
	QtVariantProperty * propertyResultRootDir = m_variantManager->addProperty( QVariant::String, tr("ResultRootDir") );
	addProperty( propertyResultRootDir, 0 );
	propertyResultRootDir->setToolTip( tr("MasterSim will automatically pass the directory generated for the slave, such that output data can be stored therein.") );

	// attempt to find FMU definition
	try {
		const MASTER_SIM::ModelDescription & modelDesc = MSIMMainWindow::instance().modelDescription(fmuPath.str());
		// process model description and create properties for all parameters

	}
	catch (IBK::Exception & ex) {
		clearProperties();
		return;
	}

}
