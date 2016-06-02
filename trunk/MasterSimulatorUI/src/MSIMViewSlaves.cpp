#include "MSIMViewSlaves.h"
#include "ui_MSIMViewSlaves.h"

#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>

#include "MSIMProjectHandler.h"
#include "MSIMUIConstants.h"
#include "MSIMSlaveItemDelegate.h"


MSIMViewSlaves::MSIMViewSlaves(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::MSIMViewSlaves)
{
	m_ui->setupUi(this);
	m_ui->verticalLayout_2->setContentsMargins(9,0,9,9);

	connect(&MSIMProjectHandler::instance(), SIGNAL(modified(int,void*)), this, SLOT(onModified(int,void*)));

	// setup tables
	m_ui->tableWidgetFMUs->horizontalHeader()->setVisible(false);
	m_ui->tableWidgetFMUs->verticalHeader()->setVisible(false);
	m_ui->tableWidgetFMUs->setColumnCount(1);
	QStringList headers;
	headers << tr("Full path");
	m_ui->tableWidgetFMUs->setHorizontalHeaderLabels(headers);
	m_ui->tableWidgetFMUs->horizontalHeader()->setStretchLastSection(true);

//	m_ui->tableWidgetSlaves->horizontalHeader()->setVisible(false);
	m_ui->tableWidgetSlaves->verticalHeader()->setVisible(false);
	m_ui->tableWidgetSlaves->setColumnCount(4);
	headers.clear();
	headers << "" << tr("Name") << tr("FMU") << tr("Cycle Nr.");
	m_ui->tableWidgetSlaves->setHorizontalHeaderLabels(headers);

	formatTable(m_ui->tableWidgetFMUs);
	formatTable(m_ui->tableWidgetSlaves);

	m_ui->tableWidgetSlaves->horizontalHeader()->resizeSection(0,m_ui->tableWidgetSlaves->verticalHeader()->defaultSectionSize());
	m_ui->tableWidgetSlaves->setItemDelegate(new MSIMSlaveItemDelegate(this));
}


MSIMViewSlaves::~MSIMViewSlaves() {
	delete m_ui;
}


void MSIMViewSlaves::onModified( int modificationType, void * data ) {
	switch ((MSIMProjectHandler::ModificationTypes)modificationType) {
		case MSIMProjectHandler::AllModified :
			break;
		default:
			return; // nothing to do for us
	}

	// update tables based on project file content
	m_ui->tableWidgetFMUs->clear();
	m_ui->tableWidgetSlaves->clearContents();
	QSet<QString> fmuPaths;

	m_ui->tableWidgetSlaves->setRowCount(project().m_simulators.size());
	for (unsigned int i=0; i<project().m_simulators.size(); ++i) {
		const MASTER_SIM::Project::SimulatorDef & simDef = project().m_simulators[i];
		QTableWidgetItem * item = new QTableWidgetItem();
		item->setData(Qt::UserRole, QColor(simDef.m_color.toQRgb()));
		m_ui->tableWidgetSlaves->setItem(i, 0, item);

		item = new QTableWidgetItem( QString::fromUtf8(simDef.m_name.c_str()) );
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
		m_ui->tableWidgetSlaves->setItem(i, 1, item);

		QString fmuPath = QString::fromUtf8(simDef.m_pathToFMU.c_str());
		fmuPaths.insert(fmuPath);
		item = new QTableWidgetItem( fmuPath );
		m_ui->tableWidgetSlaves->setItem(i, 2, item);

		item = new QTableWidgetItem( QString("%1").arg(simDef.m_cycle) );
		m_ui->tableWidgetSlaves->setItem(i, 3, item);
	}
	m_ui->tableWidgetSlaves->resizeColumnsToContents();
	m_ui->tableWidgetSlaves->horizontalHeader()->resizeSection(0,m_ui->tableWidgetSlaves->verticalHeader()->defaultSectionSize());
	m_ui->tableWidgetFMUs->setRowCount(fmuPaths.count());
	unsigned int rowIdx = 0;
	for (QSet<QString>::const_iterator it = fmuPaths.constBegin(); it != fmuPaths.constEnd(); ++it, ++rowIdx) {
		QTableWidgetItem * item = new QTableWidgetItem( *it );
		item->setFlags(Qt::ItemIsEnabled);
		m_ui->tableWidgetFMUs->setItem(rowIdx, 0, item);
	}
	m_ui->tableWidgetFMUs->resizeColumnsToContents();

}
