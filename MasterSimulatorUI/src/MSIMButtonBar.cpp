#include "MSIMButtonBar.h"

#include <QVBoxLayout>
#include <QToolButton>
#include <QIcon>
#include <QSpacerItem>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QTranslator>
#include <QKeySequence>

#include "MSIMLanguageHandler.h"
#include <MSIM_Constants.h>

void setupToolButton(QToolButton * btn, const QString & iconFile, const QString & hint) {
	btn->setIcon(QIcon(iconFile));
	btn->setIconSize(QSize(32,32));
	btn->setAutoRaise(true);
	btn->setToolTip(hint);
}

MSIMButtonBar::MSIMButtonBar(QWidget * parent) :
	QWidget(parent)
{
	QVBoxLayout * lay  = new QVBoxLayout(this);

	// create tool buttons and assign resource files
	toolButtonAbout = new QToolButton(this); lay->addWidget(toolButtonAbout);

	toolButtonNew = new QToolButton(this); lay->addWidget(toolButtonNew);
	toolButtonNew->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_N) );

	toolButtonLoad = new QToolButton(this); lay->addWidget(toolButtonLoad);
	toolButtonLoad->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O) );
	toolButtonSave = new QToolButton(this); lay->addWidget(toolButtonSave);
	toolButtonSave->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S) );
	toolButtonAnalyze = new QToolButton(this); lay->addWidget(toolButtonAnalyze);
	lay->addSpacerItem( new QSpacerItem(20,20, QSizePolicy::Preferred, QSizePolicy::Fixed) );
	toolButtonEditFMUs = new QToolButton(this); lay->addWidget(toolButtonEditFMUs);
	toolButtonEditConnections = new QToolButton(this); lay->addWidget(toolButtonEditConnections);
	toolButtonSimulate = new QToolButton(this); lay->addWidget(toolButtonSimulate);
	toolButtonSimulate->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R) );
	lay->addSpacerItem( new QSpacerItem(20,20, QSizePolicy::Preferred, QSizePolicy::Fixed) );
	toolButtonUndo = new QToolButton(this); lay->addWidget(toolButtonUndo);
	toolButtonRedo = new QToolButton(this); lay->addWidget(toolButtonRedo);
	lay->addSpacerItem( new QSpacerItem(20,20, QSizePolicy::Preferred, QSizePolicy::Fixed) );

	lay->addStretch(1);
	toolButtonSwitchLanguage = new QToolButton(this); lay->addWidget(toolButtonSwitchLanguage);
	toolButtonQuit = new QToolButton(this); lay->addWidget(toolButtonQuit);

	setupToolButton(toolButtonAbout, ":/gfx/views/32x32/info_32x32.png", tr("Shows information about the program."));
	setupToolButton(toolButtonNew, ":/gfx/views/32x32/filenew_32x32.png", tr("Create a new default project (Ctrl + N)."));
	setupToolButton(toolButtonLoad, ":/gfx/views/32x32/fileopen_32x32.png", tr("Loads a project (Ctrl + O)."));
	setupToolButton(toolButtonSave, ":/gfx/views/32x32/filesave_32x32.png", tr("Saves current project (Ctrl + S)."));
	setupToolButton(toolButtonAnalyze, ":/gfx/views/32x32/filesave_32x32.png", tr("Saves current project (Ctrl + S)."));

	setupToolButton(toolButtonEditFMUs, ":/gfx/views/32x32/MasterSim_Simulators_32x32.png", tr("Switches to slave edit mode."));
	setupToolButton(toolButtonEditConnections, ":/gfx/views/32x32/MasterSim_Connections_32x32.png", tr("Switches to connection edit mode."));
	setupToolButton(toolButtonSimulate, ":/gfx/views/32x32/MasterSim_Simulation_Settings_32x32.png", tr("Switches to simulation settings mode."));

	setupToolButton(toolButtonUndo, ":/gfx/actions/24x24/undo.png", tr("Undo of last change."));
	setupToolButton(toolButtonRedo, ":/gfx/actions/24x24/redo.png", tr("Redo of last change."));

	setupToolButton(toolButtonSwitchLanguage, ":/gfx/views/32x32/switch_language_32x32.png", tr("Change currently used language."));
	setupToolButton(toolButtonQuit, ":/gfx/views/32x32/exit_32x32.png", tr("Closes the program (asks for saving confirmation first)."));

	// open context menu with languages
	m_languageMenu = new QMenu(this);

	QAction * action = new QAction("English", m_languageMenu);
	action->setIcon(QIcon(":/gfx/icons/gb.png"));
	action->setData("en");
	connect(action, SIGNAL(triggered()), this, SLOT(onLanguageActionTriggered()));
	m_languageMenu->addAction(action);

	action = new QAction("Deutsch", m_languageMenu);
	action->setIcon(QIcon(":/gfx/icons/de.png"));
	action->setData("de");
	connect(action, SIGNAL(triggered()), this, SLOT(onLanguageActionTriggered()));
	m_languageMenu->addAction(action);

	connect(toolButtonSwitchLanguage, SIGNAL(clicked()),
			this, SLOT(onToolButtonSwitchLanguageClicked()));

	lay->setMargin(4);
	setLayout(lay);
}


MSIMButtonBar::~MSIMButtonBar() {
}


void MSIMButtonBar::setEnabled(bool enabled) {
	toolButtonNew->setEnabled(enabled);
	toolButtonLoad->setEnabled(enabled);
	toolButtonSave->setEnabled(enabled);
	toolButtonSimulate->setEnabled(enabled);
}


void MSIMButtonBar::onToolButtonSwitchLanguageClicked() {
	m_languageMenu->popup(mapToGlobal(toolButtonSwitchLanguage->pos()));
}


void MSIMButtonBar::onLanguageActionTriggered() {
	QAction * action = static_cast<QAction*>(sender());
	QString langId = action->data().toString();
	// save data in language handler

	MSIMLanguageHandler::instance().installTranslator(langId);
	QMessageBox::information(0, tr("Language Changed"),
		tr("Please restart the program to activate the selected language!"));
}
