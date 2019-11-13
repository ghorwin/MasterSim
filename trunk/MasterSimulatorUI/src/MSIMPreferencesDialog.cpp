#include "MSIMPreferencesDialog.h"
#include "ui_MSIMPreferencesDialog.h"

#include "MSIMPreferencesPageGeneral.h"
#include <QStackedWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QShortcut>

MSIMPreferencesDialog::MSIMPreferencesDialog(QWidget * parent) :
	QDialog(parent),
	m_ui(new Ui::MSIMPreferencesDialog)
{
	m_ui->setupUi(this);

	// populate list widget with icons
	QListWidgetItem * btn = new QListWidgetItem(m_ui->listWidget); // creates and adds item
	btn->setIcon(QIcon(":/gfx/master/preferences/config_96.png"));
	btn->setText(tr("General Settings"));
	btn->setTextAlignment(Qt::AlignHCenter);
	btn->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

//	btn = new QListWidgetItem(m_listWidget); // creates and adds item
//	btn->setIcon(QIcon(":/icons/update.png"));
//	btn->setText(tr("Default Project Info"));
//	btn->setTextAlignment(Qt::AlignHCenter);
//	btn->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

//	btn = new QListWidgetItem(m_listWidget); // creates and adds item
//	btn->setIcon(QIcon(":/icons/update.png"));
//	btn->setText(tr("Default Print Config"));
//	btn->setTextAlignment(Qt::AlignHCenter);
//	btn->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);


	// setup configuration pages

	// add pages
	m_pageGeneral = new MSIMPreferencesPageGeneral(this);
	m_ui->pages->addWidget(m_pageGeneral);

	m_ui->listWidget->setCurrentRow(0);

	/// \todo Qt Support: Column size of icon list view matching text width of icons...

//	setMinimumSize(750,670);
}


MSIMPreferencesDialog::~MSIMPreferencesDialog() {
	delete m_ui;
}


bool MSIMPreferencesDialog::edit() {
	// transfer settings data to User Interface
	updateUi();

	// execute dialog and pass on result
	return (exec() == QDialog::Accepted);
}


// ** slots **

void MSIMPreferencesDialog::on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous) {
	if (current == NULL)
		current = previous;
	m_ui->pages->setCurrentIndex(m_ui->listWidget->row(current));
	setWindowTitle(current->text());
}


// ** protected functions **

void MSIMPreferencesDialog::accept() {
	if (!storeConfig())
		return;

	QDialog::accept();
}


// ** private functions **

void MSIMPreferencesDialog::updateUi() {
	m_pageGeneral->updateUi();
}


bool MSIMPreferencesDialog::storeConfig() {
	if (!m_pageGeneral->storeConfig()) {
		m_ui->pages->setCurrentWidget(m_pageGeneral);
		return false;
	}
	return true;
}


void MSIMPreferencesDialog::on_listWidget_currentRowChanged(int currentRow) {
	m_ui->pages->setCurrentIndex(currentRow);
}
