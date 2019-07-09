#include "MSIMSlavePropertyWidget.h"

#include <qttreepropertybrowser.h>
#include <qtpropertybrowserutils.h>

#include <QTreeWidget>

#include <MSIM_Project.h>

#include "MSIMMainWindow.h"
#include "MSIMUndoSlaveParameters.h"

MSIMSlavePropertyWidget::MSIMSlavePropertyWidget(QWidget * parent) :
	QPW::PropertyWidgetBase(parent),
	m_blockUpdateProperties(false)
{
	disconnect( m_variantManager, SIGNAL(valueChanged(QtProperty*,QVariant)),
				m_variantManager, SLOT(slotValueChanged(QtProperty*,QVariant)));

	setMinimumWidth( 300 );

	// first column in property editor (labels) takes only as much space as needed
	m_propertyEditor->setResizeMode(QtTreePropertyBrowser::ResizeToContents);
	m_propertyEditor->setFixHeightToContents(true);
	QTreeWidgetItem * item = m_propertyEditor->treeWidget()->headerItem();
	item->setText(0, tr("Parameter/Variable"));
}


void MSIMSlavePropertyWidget::updateProperties(int selectedSlave) {
	// prevent cyclic updates
	if (m_blockUpdateProperties)
		return;

	m_currentSlaveIndex = selectedSlave;

	disconnect( m_variantManager, SIGNAL(valueChanged(QtProperty*,QVariant)), this, SLOT(valueChanged(QtProperty*,QVariant)));

	m_propertyEditor->clear();
	m_propertyEditor->setEnabled(false);
	if (selectedSlave == -1)
		return;

	m_propertyEditor->setEnabled(true);

	// fill in all parameters defined in FMU slave
	// get slave instance
	const MASTER_SIM::Project & p = project();
	if (selectedSlave >= (int)p.m_simulators.size())
		return;

	std::string slaveName = project().m_simulators[m_currentSlaveIndex].m_name;

	// attempt to find FMU definition
	try {
		const MASTER_SIM::ModelDescription & modelDesc = MSIMMainWindow::instance().modelDescription(slaveName);
		// process model description and create properties for all parameters
		for (unsigned int i=0; i<modelDesc.m_variables.size(); ++i) {
			const MASTER_SIM::FMIVariable & var = modelDesc.m_variables[i];
			if (var.m_variability == "parameter" || var.m_variability == "tunable") {
				QString varNameType;
				if (var.m_type == MASTER_SIM::FMIVariable::VT_DOUBLE) {
					std::string unit = var.m_unit;
					if (unit.empty())
						unit = "---";
					varNameType = QString::fromStdString(var.m_name + " [" + unit + "]");
				}
				else
					varNameType = QString::fromStdString(var.m_name);
				QtVariantProperty * varProp = m_variantManager->addProperty( QVariant::String, varNameType );
				addProperty( varProp, var.m_varIdx );
				if (var.m_causality == MASTER_SIM::FMIVariable::C_INTERNAL)
					varProp->setInternal(true);
				// todo - set "default/auto-text" property propertyResultRootDir->setAttribute()
				if (var.m_name == "ResultRootDir") {
					varProp->setToolTip( tr("The directory to be used by the slave to stored output data therein.") );
					varProp->setValueToolTip( tr("If empty, MasterSim will automatically pass the directory generated for the slave, such that output data can be stored therein.") );
				}
				else {
					if (var.m_causality == MASTER_SIM::FMIVariable::C_INTERNAL)
						varProp->setDescriptionToolTip( tr("Internal parameter: '%1'").arg(QString::fromStdString(var.m_description) ) );
					else
						varProp->setDescriptionToolTip( QString::fromStdString(var.m_description) );
					varProp->setValueToolTip( tr("Default value = %1").arg(QString::fromStdString(var.m_startValue)) );
				}
				// check for a parameter value with this name
				for (auto p : project().m_simulators[selectedSlave].m_parameters) {
					if (p.first == var.m_name) {
						varProp->setValue( QString::fromStdString(p.second) );
					}
				}
			}
		}

		connect( m_variantManager, SIGNAL(valueChanged(QtProperty*,QVariant)), this, SLOT(valueChanged(QtProperty*,QVariant)));
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

	// prevent cyclic updates
	if (m_blockUpdateProperties)
		return;

	// prevent call of valueChanged() when we modify property items below
	disconnect( m_variantManager, SIGNAL(valueChanged(QtProperty*,QVariant)), this, SLOT(valueChanged(QtProperty*,QVariant)));

	m_blockUpdateProperties = true;

	std::string slaveName = project().m_simulators[m_currentSlaveIndex].m_name;
	const MASTER_SIM::ModelDescription & modelDesc = MSIMMainWindow::instance().modelDescription(slaveName);

	// get id for this property
	unsigned int varIdx = m_propertyToId[prop];

	const MASTER_SIM::Project & p = project();
	Q_ASSERT(m_currentSlaveIndex < (int)p.m_simulators.size());

	// look up variable that matches this varIdx
	for (auto var : modelDesc.m_variables) {
		if (var.m_varIdx == varIdx) {
			// create an undo-action for modifying a slave parameter
			MSIMUndoSlaveParameters * undo = new MSIMUndoSlaveParameters(tr("Parameter/variable '%1.%2' modified.")
																		 .arg(QString::fromStdString(slaveName), QString::fromStdString(var.m_name)),
																		 m_currentSlaveIndex, var.m_name, value.toString().toStdString());
			undo->push();
			break;
		}
	}

	// reconnect the change signal
	connect( m_variantManager, SIGNAL(valueChanged(QtProperty*,QVariant)), this, SLOT(valueChanged(QtProperty*,QVariant)));

	m_blockUpdateProperties = false;

}

