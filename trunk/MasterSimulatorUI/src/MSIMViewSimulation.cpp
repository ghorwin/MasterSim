#include "MSIMViewSimulation.h"
#include "ui_MSIMViewSimulation.h"

#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>

#include "MSIMProjectHandler.h"
#include "MSIMUIConstants.h"


MSIMViewSimulation::MSIMViewSimulation(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::MSIMViewSimulation)
{
	m_ui->setupUi(this);

	connect(&MSIMProjectHandler::instance(), SIGNAL(modified(int,void*)), this, SLOT(onModified(int,void*)));


}


MSIMViewSimulation::~MSIMViewSimulation() {
	delete m_ui;
}


void MSIMViewSimulation::onModified( int modificationType, void * data ) {
	switch ((MSIMProjectHandler::ModificationTypes)modificationType) {
		case MSIMProjectHandler::AllModified :
			break;
		default:
			return; // nothing to do for us
	}

}
