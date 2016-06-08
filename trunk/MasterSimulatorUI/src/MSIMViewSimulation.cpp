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
#include "MSIMConversion.h"
#include "MSIMUndoSimulationSettings.h"


MSIMViewSimulation::MSIMViewSimulation(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::MSIMViewSimulation)
{
	m_ui->setupUi(this);

	connect(&MSIMProjectHandler::instance(), SIGNAL(modified(int,void*)), this, SLOT(onModified(int,void*)));

	blockMySignals(this, true);

	// setup combo boxes
	QStringList units;
	units << "ms" << "s" << "min" << "h" <<"d" << "a";
	m_ui->comboBoxStartTimeUnit->addItems(units);
	m_ui->comboBoxEndTimeUnit->addItems(units);
	m_ui->comboBoxDtStartUnit->addItems(units);
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
	m_ui->comboBoxTerminalCommand->addItem("gnome-terminal --working-directory=\"%wd\" -x %cmdline");

#ifdef Q_OS_WIN
	m_ui->labelTerminalCommand->setVisible(false);
	m_ui->comboBoxTerminalCommand->setVisible(false);
#else
#endif
	m_ui->checkBoxCloseOnExit->setChecked(false);

	blockMySignals(this, false);
}


MSIMViewSimulation::~MSIMViewSimulation() {
	delete m_ui;
}


void MSIMViewSimulation::onModified( int modificationType, void * data ) {
	switch ((MSIMProjectHandler::ModificationTypes)modificationType) {
		case MSIMProjectHandler::AllModified :
		case MSIMProjectHandler::SimulationSettingsModified :
			break;

		default:
			return; // nothing to do for us
	}

	blockMySignals(this, true);
	setupLineEditUnitCombo(m_ui->lineEditStartTime, m_ui->comboBoxStartTimeUnit, project().m_tStart);
	setupLineEditUnitCombo(m_ui->lineEditEndTime, m_ui->comboBoxEndTimeUnit, project().m_tEnd);
	setupLineEditUnitCombo(m_ui->lineEditDtStart, m_ui->comboBoxDtStartUnit, project().m_hStart);

	setupLineEditUnitCombo(m_ui->lineEditDtMax, m_ui->comboBoxMaxDtUnit, project().m_hMax);
	setupLineEditUnitCombo(m_ui->lineEditDtMin, m_ui->comboBoxMinDtUnit, project().m_hMin);
	setupLineEditUnitCombo(m_ui->lineEditDtIterLimit, m_ui->comboBoxDtIterLimitUnit, project().m_hFallBackLimit);

	setupLineEditUnitCombo(m_ui->lineEditDtOutput, m_ui->comboBoxDtOutputUnit, project().m_tOutputStepMin);

	m_ui->lineEditRelTol->setText( QString("%L1").arg(project().m_relTol));
	m_ui->lineEditAbsTol->setText( QString("%L1").arg(project().m_absTol));

	m_ui->spinBoxMaxIteration->setValue(project().m_maxIterations);

	m_ui->comboBoxMasterAlgorithm->setCurrentIndex(project().m_masterMode);
	m_ui->comboBoxErrorControl->setCurrentIndex(project().m_errorControlMode);
	m_ui->checkBoxAdjustStepSize->setChecked( project().m_adjustStepSize);
	m_ui->checkBoxBinaryOutputFiles->setChecked( project().m_binaryOutputFiles);

	blockMySignals(this, false);

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
//	bool success;

#ifdef Q_OS_LINUX
	// open terminal and start solver in terminal
	std::string terminalCommand = m_ui->comboBoxTerminalCommand->currentText().toUtf8().data();
	terminalCommand = IBK::replace_string(terminalCommand, "%wd", MSIMSettings::instance().m_installDir.toUtf8().data());
	std::string bashCmdLine = (m_solverName + " " + commandLineArgs.join(" ")).toUtf8().data();
	terminalCommand = IBK::replace_string(terminalCommand, "%cmdline", bashCmdLine);

	QString allCmdLine = QString::fromUtf8(terminalCommand.c_str());
	/*int res = */ myProcess->execute(allCmdLine);
//	success = (res == 0);
#else
	/// \todo check how to spawn a terminal on mac
	success = myProcess->startDetached(m_solverName, commandLineArgs);
#endif

	// release process
	myProcess.release();

#endif // Q_OS_WIN
}


void MSIMViewSimulation::updateCommandLine() {
	// command line depends on Windows/MacOSX/Linux

	m_commandLineArgs.clear();

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
	QString uname = QString::fromStdString(p.IO_unit.name());
	unsigned int idx = combo->findText( uname, Qt::MatchExactly );
	combo->setCurrentIndex(idx);
	combo->blockSignals(false);
}


void MSIMViewSimulation::on_checkBoxCloseOnExit_clicked() {
	updateCommandLine();
}


void MSIMViewSimulation::on_comboBoxVerbosityLevel_currentIndexChanged(int) {
	updateCommandLine();
}


void MSIMViewSimulation::on_comboBoxMasterAlgorithm_currentIndexChanged(int index) {
	MASTER_SIM::Project p = project(); // create copy of project
	p.m_masterMode = (MASTER_SIM::Project::MasterMode)index;

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push(); // reset focus on combo box
}


void MSIMViewSimulation::on_comboBoxErrorControl_currentIndexChanged(int index) {
	MASTER_SIM::Project p = project(); // create copy of project
	p.m_errorControlMode = (MASTER_SIM::Project::ErrorControlMode)index;

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push(); // reset focus on combo box
}


void MSIMViewSimulation::on_spinBoxMaxIteration_valueChanged(int arg1) {
	MASTER_SIM::Project p = project(); // create copy of project
	p.m_maxIterations = arg1;

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push(); // reset focus on combo box
}





void MSIMViewSimulation::on_comboBoxStartTimeUnit_currentIndexChanged(int) {
	MASTER_SIM::Project p = project(); // create copy of project
	p.m_tStart.IO_unit.set( m_ui->comboBoxStartTimeUnit->currentText().toStdString());

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push(); // reset focus on combo box
}


void MSIMViewSimulation::on_comboBoxEndTimeUnit_currentIndexChanged(int) {
	MASTER_SIM::Project p = project(); // create copy of project
	p.m_tEnd.IO_unit.set( m_ui->comboBoxEndTimeUnit->currentText().toStdString());

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push(); // reset focus on combo box
}


void MSIMViewSimulation::on_comboBoxMinDtUnit_currentIndexChanged(int) {
	MASTER_SIM::Project p = project(); // create copy of project
	p.m_hMin.IO_unit.set( m_ui->comboBoxMinDtUnit->currentText().toStdString());

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push(); // reset focus on combo box
}


void MSIMViewSimulation::on_comboBoxMaxDtUnit_currentIndexChanged(int) {
	MASTER_SIM::Project p = project(); // create copy of project
	p.m_hMax.IO_unit.set( m_ui->comboBoxMaxDtUnit->currentText().toStdString());

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push(); // reset focus on combo box
}


void MSIMViewSimulation::on_comboBoxDtIterLimitUnit_currentIndexChanged(int) {
	MASTER_SIM::Project p = project(); // create copy of project
	p.m_hFallBackLimit.IO_unit.set( m_ui->comboBoxDtIterLimitUnit->currentText().toStdString());

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push(); // reset focus on combo box
}


void MSIMViewSimulation::on_comboBoxDtOutputUnit_currentIndexChanged(int) {
	MASTER_SIM::Project p = project(); // create copy of project
	p.m_tOutputStepMin.IO_unit.set( m_ui->comboBoxDtOutputUnit->currentText().toStdString());

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push(); // reset focus on combo box
}


void MSIMViewSimulation::on_comboBoxDtStartUnit_currentIndexChanged(int) {
	MASTER_SIM::Project p = project(); // create copy of project
	p.m_hStart.IO_unit.set( m_ui->comboBoxDtStartUnit->currentText().toStdString());

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push(); // reset focus on combo box
}


void MSIMViewSimulation::on_lineEditStartTime_editingFinished() {
	IBK::Parameter par;
	if (!lineEditToParameter(this, "tstart", par, m_ui->lineEditStartTime, m_ui->comboBoxStartTimeUnit))
		return;
	if (par.value < 0) {
		QMessageBox::critical(this, tr("Invalid input"), tr("Time must be >= 0."));
		m_ui->lineEditStartTime->selectAll();
		return;
	}

	MASTER_SIM::Project p = project(); // create copy of project
	p.m_tStart = par;

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push(); // reset focus on combo box
}


void MSIMViewSimulation::on_lineEditEndTime_editingFinished() {
	IBK::Parameter par;
	if (!lineEditToParameter(this, "tend", par, m_ui->lineEditEndTime, m_ui->comboBoxEndTimeUnit))
		return;
	if (par.value < 0) {
		QMessageBox::critical(this, tr("Invalid input"), tr("Time must be >= 0."));
		m_ui->lineEditEndTime->selectAll();
		return;
	}

	MASTER_SIM::Project p = project(); // create copy of project
	p.m_tEnd = par;

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push(); // reset focus on combo box
}


void MSIMViewSimulation::on_lineEditRelTol_editingFinished() {
	IBK::Parameter par;
	if (!lineEditToParameter(this, "it_tol_rel", par, m_ui->lineEditRelTol, NULL))
		return;
	if (par.value <= 0) {
		QMessageBox::critical(this, tr("Invalid input"), tr("Tolerance must be > 0."));
		m_ui->lineEditRelTol->selectAll();
		return;
	}

	MASTER_SIM::Project p = project(); // create copy of project
	p.m_relTol = par.value;

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push(); // reset focus on combo box
}


void MSIMViewSimulation::on_lineEditAbsTol_editingFinished() {
	IBK::Parameter par;
	if (!lineEditToParameter(this, "it_tol_abs", par, m_ui->lineEditAbsTol, NULL))
		return;
	if (par.value <= 0) {
		QMessageBox::critical(this, tr("Invalid input"), tr("Tolerance must be > 0."));
		m_ui->lineEditAbsTol->selectAll();
		return;
	}

	MASTER_SIM::Project p = project(); // create copy of project
	p.m_absTol = par.value;

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push(); // reset focus on combo box
}


void MSIMViewSimulation::on_checkBoxAdjustStepSize_toggled(bool checked) {
	MASTER_SIM::Project p = project(); // create copy of project
	p.m_adjustStepSize = checked;

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push(); // reset focus on combo box
}


void MSIMViewSimulation::on_checkBoxBinaryOutputFiles_toggled(bool checked) {
	MASTER_SIM::Project p = project(); // create copy of project
	p.m_binaryOutputFiles = checked;

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push(); // reset focus on combo box
}


void MSIMViewSimulation::on_lineEditDtMin_editingFinished() {
	IBK::Parameter par;
	if (!lineEditToParameter(this, "tstepmin", par, m_ui->lineEditDtMin, m_ui->comboBoxMinDtUnit))
		return;
	if (par.value < 0) {
		QMessageBox::critical(this, tr("Invalid input"), tr("Step size must be >= 0."));
		m_ui->lineEditDtMin->selectAll();
		return;
	}

	MASTER_SIM::Project p = project(); // create copy of project
	p.m_hMin = par;

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push(); // reset focus on combo box
}


void MSIMViewSimulation::on_lineEditDtMax_editingFinished() {
	IBK::Parameter par;
	if (!lineEditToParameter(this, "tstepmax", par, m_ui->lineEditDtMax, m_ui->comboBoxMaxDtUnit))
		return;
	if (par.value < 0) {
		QMessageBox::critical(this, tr("Invalid input"), tr("Step size must be >= 0."));
		m_ui->lineEditDtMax->selectAll();
		return;
	}

	MASTER_SIM::Project p = project(); // create copy of project
	p.m_hMax = par;

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push(); // reset focus on combo box
}


void MSIMViewSimulation::on_lineEditDtIterLimit_editingFinished() {
	IBK::Parameter par;
	if (!lineEditToParameter(this, "tstepiterlimit", par, m_ui->lineEditDtIterLimit, m_ui->comboBoxDtIterLimitUnit))
		return;
	if (par.value < 0) {
		QMessageBox::critical(this, tr("Invalid input"), tr("Step size must be >= 0."));
		m_ui->lineEditDtIterLimit->selectAll();
		return;
	}

	MASTER_SIM::Project p = project(); // create copy of project
	p.m_hFallBackLimit = par;

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push(); // reset focus on combo box
}


void MSIMViewSimulation::on_lineEditDtOutput_editingFinished() {
	IBK::Parameter par;
	if (!lineEditToParameter(this, "toutputstepmin", par, m_ui->lineEditDtOutput, m_ui->comboBoxDtOutputUnit))
		return;
	if (par.value < 0) {
		QMessageBox::critical(this, tr("Invalid input"), tr("Step size must be >= 0."));
		m_ui->lineEditDtOutput->selectAll();
		return;
	}

	MASTER_SIM::Project p = project(); // create copy of project
	p.m_tOutputStepMin = par;

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push(); // reset focus on combo box
}


void MSIMViewSimulation::on_lineEditDtStart_editingFinished() {
	IBK::Parameter par;
	if (!lineEditToParameter(this, "tstart", par, m_ui->lineEditDtStart, m_ui->comboBoxDtStartUnit))
		return;
	if (par.value < 0) {
		QMessageBox::critical(this, tr("Invalid input"), tr("Step size must be >= 0."));
		m_ui->lineEditDtStart->selectAll();
		return;
	}

	MASTER_SIM::Project p = project(); // create copy of project
	p.m_hStart = par;

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push(); // reset focus on combo box
}


