#include "MSIMViewSimulation.h"
#include "ui_MSIMViewSimulation.h"

#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QDialogButtonBox>

#include <memory>
#include <cstring>

#include <IBK_UnitList.h>
#include <IBK_Unit.h>

#ifdef Q_OS_WIN
#undef UNICODE
#include <windows.h>
#endif

#include "MSIMProjectHandler.h"
#include "MSIMUIConstants.h"
#include "MSIMSettings.h"
#include "MSIMMainWindow.h"


MSIMViewSimulation::MSIMViewSimulation(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::MSIMViewSimulation)
{
	m_ui->setupUi(this);

	connect(&MSIMProjectHandler::instance(), SIGNAL(modified(int,void*)), this, SLOT(onModified(int,void*)));

	// setup combo boxes
	QStringList units;
	units << "ms" << "s" << "min" << "h" <<"d" << "a";
	m_ui->comboBoxStartTimeUnit->addItems(units);
	m_ui->comboBoxEndTimeUnit->addItems(units);
	m_ui->comboBoxDtIterLimitUnit->addItems(units);
	m_ui->comboBoxMaxDtUnit->addItems(units);
	m_ui->comboBoxMinDtUnit->addItems(units);
	m_ui->comboBoxDtOutputUnit->addItems(units);

	QStringList masterAlgorithms;
	masterAlgorithms << "Gauss-Jacobi" << "Gauss-Seidel" << "Newton";
	m_ui->comboBoxMasterAlgorithm->addItems(masterAlgorithms);

	QStringList errorControlModes;
	errorControlModes << tr("None") << tr("Monitor") << tr("Richardson (adjust step size)");
	m_ui->comboBoxErrorControl->addItems(errorControlModes);

	QStringList verbosityLevels;
	verbosityLevels << tr("Minimal") << tr("Normal") << tr("Informative") << tr("Detailed") << tr("Developer");
	m_ui->comboBoxVerbosityLevel->addItems(verbosityLevels);
	m_ui->comboBoxVerbosityLevel->setCurrentIndex(MSIMSettings::instance().m_userLogLevelConsole);

	// preset for terminal command
	m_ui->comboBoxTerminalCommand->addItem("gnome-terminal %cmdline");

#ifdef Q_OS_WIN
	m_ui->labelTerminalCommand->setVisible(false);
	m_ui->comboBoxTerminalCommand->setVisible(false);
	m_ui->checkBoxCloseOnExit->setChecked(false);
#else
	m_ui->checkBoxCloseOnExit->setVisible(false);
	m_ui->checkBoxCloseOnExit->setChecked(false);
#endif
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

	blockSignals(true);
	setupLineEditUnitCombo(m_ui->lineEditStartTime, m_ui->comboBoxStartTimeUnit, project().m_tStart);
	setupLineEditUnitCombo(m_ui->lineEditEndTime, m_ui->comboBoxEndTimeUnit, project().m_tEnd);

	setupLineEditUnitCombo(m_ui->lineEditDtMax, m_ui->comboBoxMaxDtUnit, project().m_hMax);
	setupLineEditUnitCombo(m_ui->lineEditDtMin, m_ui->comboBoxMinDtUnit, project().m_hMin);
	setupLineEditUnitCombo(m_ui->lineEditDtIterLimit, m_ui->comboBoxDtIterLimitUnit, project().m_hFallBackLimit);

	setupLineEditUnitCombo(m_ui->lineEditDtOutput, m_ui->comboBoxDtOutputUnit, project().m_tOutputStepMin);

	m_ui->lineEditRelTol->setText( QString("%L1").arg(project().m_relTol));
	m_ui->lineEditAbsTol->setText( QString("%L1").arg(project().m_absTol));

	m_ui->comboBoxMasterAlgorithm->setCurrentIndex(project().m_masterMode);
	m_ui->comboBoxErrorControl->setCurrentIndex(project().m_errorControlMode);
	m_ui->checkBoxAdjustStepSize->setChecked( project().m_adjustStepSize);
	m_ui->checkBoxBinaryOutputFiles->setChecked( project().m_binaryOutputFiles);
	blockSignals(false);

	updateCommandLine();
}



void MSIMViewSimulation::on_toolButtonStartInTerminal_clicked() {
	// first save project if modified
	QString projectFile = MSIMProjectHandler::instance().projectFile();
	if (MSIMProjectHandler::instance().isModified() || projectFile.isEmpty()) {
		if (!MSIMMainWindow::instance().saveProject())
			return;
	}
	projectFile = MSIMProjectHandler::instance().projectFile();

	// check if solver exists
	if (!QFileInfo(m_solverName).exists()) {
		QMessageBox::critical(this, tr("Solver error"), tr("Cannot find solver executable '%1'.").arg(m_solverName));
		return;
	}

	// now update command line with current project file name
	updateCommandLine();
	QStringList commandLineArgs = m_commandLineArgs;

	// spawn process
#ifdef Q_OS_WIN

	/// \todo use wide-string version of API and/or encapsulate spawn process into a function

	// Use WinAPI to create a solver process
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	std::string utf8String = projectFile.toUtf8().data();
	si.lpTitle = (LPSTR)utf8String.c_str();
//	std::strcpy(si.lpTitle, );
	ZeroMemory( &pi, sizeof(pi) );
	const unsigned int lower_priority = 0x00004000;
	QString cmdLine = QString("%1 %2 \"%3\"")
		.arg(m_solverName)
		.arg(commandLineArgs.join(" "))
			.arg(projectFile);

	std::string cmd = cmdLine.toUtf8().data();
	// Start the child process.
	if( !CreateProcess( NULL,   // No module name (use command line).
		&cmd[0], 				// Command line.
		NULL,             		// Process handle not inheritable.
		NULL,             		// Thread handle not inheritable.
		FALSE,            		// Set handle inheritance to FALSE.
		lower_priority,   		// Create with priority lower then normal.
		NULL,             		// Use parent's environment block.
		NULL,             		// Use parent's starting directory.
		&si,              		// Pointer to STARTUPINFO structure.
		&pi )             		// Pointer to PROCESS_INFORMATION structure.
	)
	{
		QMessageBox::critical(this, tr("Error running solver"),
							  tr("Could not start solver executable '%1'.").arg(m_solverName));
		return;
	}
#else // Q_OS_WIN

	std::auto_ptr<QProcess> myProcess (new QProcess(this));
	commandLineArgs.append(projectFile);
	/// \todo Bug: startDetached returns true even if solver fails to start due to missing shared libs.
	bool success;

#ifdef Q_OS_LINUX
	// open terminal and start solver in terminal
	QString terminalCommand = m_ui->comboBoxTerminalCommand->currentText();
	QStringList cmdLineArgs;
	cmdLineArgs << QString("--working-directory=\"%1\"").arg(MSIMSettings::instance().m_installDir);
	QString bashCmdLine = "-x " + m_solverName + " " + commandLineArgs.join(" ");
	cmdLineArgs << bashCmdLine;
	QString allCmdLine = terminalCommand + " " + cmdLineArgs.join(" ");
	int res = myProcess->execute(allCmdLine);
	success = (res == 0);
#else
	/// \todo check how to spawn a terminal on mac
	success = myProcess->startDetached(m_solverName, commandLineArgs);
#endif

	// check if solver thread is running
	myProcess.release();

#endif // Q_OS_WIN
}


void MSIMViewSimulation::updateCommandLine() {
	// command line depends on Windows/MacOSX/Linux

	// arguments, currently only project file
	if (m_ui->checkBoxCloseOnExit->isChecked())
		m_commandLineArgs.append("--close-on-exit");

	m_commandLineArgs.append(QString("--verbosity-level=%1").arg(m_ui->comboBoxVerbosityLevel->currentIndex()));

	QString appName = MSIMSettings::instance().m_installDir;
#ifdef Q_OS_WIN
	appName += "/MasterSimulator.exe";
#else // Q_OS_WIN
	appName += "/MasterSimulator";
#endif // Q_OS_WIN
	m_solverName = appName;

	QString projectFile = MSIMProjectHandler::instance().projectFile();

	m_commandLine = QString("%1 %2 \"%3\"")
		.arg(appName)
		.arg(m_commandLineArgs.join(" "))
		.arg(projectFile);

	m_ui->lineEditCommandLine->setText( m_commandLine );
}


void MSIMViewSimulation::setupLineEditUnitCombo(QLineEdit * lineEdit, QComboBox * combo, const IBK::Parameter & p) {
	lineEdit->setText( QString("%L1").arg(p.get_value()));
	combo->blockSignals(true);
	unsigned int idx = combo->findText( QString::fromStdString(p.unit().name()));
	combo->setCurrentIndex(idx);
	combo->blockSignals(false);
}
