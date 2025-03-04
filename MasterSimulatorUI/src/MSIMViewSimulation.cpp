#include "MSIMViewSimulation.h"
#include "ui_MSIMViewSimulation.h"

#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QTimer>
#include <QThread>
#include <QStandardPaths>
#include <QTextStream>

#include <memory>
#include <cstring>

#include <IBK_UnitList.h>
#include <IBK_Unit.h>
#include <IBK_configuration.h>

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
#include "MSIMLogFileDialog.h"
#include "MSIMSimulationMonitorWidget.h"

MSIMViewSimulation::MSIMViewSimulation(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::MSIMViewSimulation)
{
	m_ui->setupUi(this);

	connect(&MSIMProjectHandler::instance(), &MSIMProjectHandler::modified,
			this, &MSIMViewSimulation::onModified);

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

#if defined(Q_OS_LINUX)
	m_ui->comboBoxTermEmulator->blockSignals(true);
	m_ui->comboBoxTermEmulator->setCurrentIndex(MSIMSettings::instance().m_terminalEmulator);
	m_ui->comboBoxTermEmulator->blockSignals(false);
#else
	// win and mac have no terminal selection
	m_ui->labelTerminalEmulator->setVisible(false);
	m_ui->comboBoxTermEmulator->setVisible(false);
#endif

	blockMySignals(this, false);
}


MSIMViewSimulation::~MSIMViewSimulation() {
	delete m_ui;
}


void MSIMViewSimulation::onModified(unsigned int modificationType, void * /*data*/ ) {
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

	setupLineEditUnitCombo(m_ui->lineEditDtOutput, m_ui->comboBoxDtOutputUnit, project().m_hOutputMin);

	m_ui->lineEditRelTol->setText( QString("%L1").arg(project().m_relTol));
	m_ui->lineEditAbsTol->setText( QString("%L1").arg(project().m_absTol));

	m_ui->spinBoxMaxIteration->setValue((int)project().m_maxIterations);

	m_ui->comboBoxMasterAlgorithm->setCurrentIndex(project().m_masterMode);
	m_ui->comboBoxErrorControl->setCurrentIndex(project().m_errorControlMode);
	m_ui->checkBoxAdjustStepSize->setChecked( project().m_adjustStepSize);

	m_ui->checkBoxWriteInternalVariables->setChecked( project().m_writeInternalVariables);
	m_ui->checkBoxWriteUnconnectedVariables->setChecked( project().m_writeUnconnectedFileReaderVars);
	m_ui->checkBoxPreventOversteppingOfEndTime->setChecked( project().m_preventOversteppingOfEndTime);

	blockMySignals(this, false);

	updateCommandLine();
}



void MSIMViewSimulation::on_toolButtonStartInTerminal_clicked() {
	// first move focus away from any inputs to update project data structure
	m_ui->pushButtonShowLogfile->setFocus();
	qApp->processEvents(); // process undo events

	// first save project if modified
	QString projectFile = MSIMProjectHandler::instance().projectFile();
	if (MSIMProjectHandler::instance().isModified() || projectFile.isEmpty()) {
		if (!MSIMMainWindow::instance().saveProject())
			return;
	}
	projectFile = MSIMProjectHandler::instance().projectFile();

#if 0
//	QThread * runThread = new QThread(this);

	MSIMSimulationMonitorWidget msimWidget(this);
	msimWidget.show();
	IBK::Path projectFilePath(projectFile.toStdString());
//	msimWidget.moveToThread(runThread);
	msimWidget.run(projectFilePath, m_ui->comboBoxVerbosityLevel->currentIndex());
//	runThread->start();
	return;
#endif

	// check if solver exists
	if (!QFileInfo::exists(m_solverName)) {
		QMessageBox::critical(this, tr("Solver error"), tr("Cannot find solver executable '%1'.").arg(m_solverName));
		return;
	}

	// now update command line with current project file name
	updateCommandLine();
	QStringList commandLineArgs = m_commandLineArgs;

	// remove existing directory structure
	QString targetPath = projectFile.left(projectFile.lastIndexOf('.'));
	// remove result directory completed, unless we have "skip-unzip" checked
	if (!m_ui->checkBoxSkipUnzip->isChecked() ) {
		QDir resultDir( targetPath );
		resultDir.removeRecursively();
	}

#if defined(Q_OS_LINUX)
	MSIMSettings::TerminalEmulators runOption = (MSIMSettings::TerminalEmulators)m_ui->comboBoxTermEmulator->currentIndex();
#else
	MSIMSettings::TerminalEmulators runOption = (MSIMSettings::TerminalEmulators)-1;
#endif
	bool success = MSIMSettings::startProcess(m_solverName, commandLineArgs, projectFile, runOption);
	if (!success) {
#if defined(Q_OS_LINUX)
		switch (runOption) {
			case MSIMSettings::TE_None:
				QMessageBox::critical(this, QString(), tr("Could not run solver '%1'").arg(m_solverName));
				break;
			case MSIMSettings::TE_XTerm:
				QMessageBox::critical(this, QString(), tr("Could not run XTerm terminal emulator (xterm). "
														  "Please install XTerm or select a different terminal emulator from the list!") );
				break;
			case MSIMSettings::TE_GnomeTerminal:
				QMessageBox::critical(this, QString(), tr("Could not run Gnome terminal emulator (gnome-terminal). "
														  "Please install GNOME terminal or select a different terminal emulator from the list!") );
				break;
		}
#else
		QMessageBox::critical(this, QString(), tr("Could not run solver '%1'").arg(m_solverName));
#endif
		return;
	}

}


void MSIMViewSimulation::updateCommandLine() {
	// command line depends on Windows/MacOSX/Linux

	m_commandLineArgs.clear();

	m_commandLineArgs.append(QString("--verbosity-level=%1").arg(m_ui->comboBoxVerbosityLevel->currentIndex()));

	if (m_ui->checkBoxSkipUnzip->isChecked())
		m_commandLineArgs.append("--skip-unzip");

	QString appName = MSIMSettings::instance().m_installDir;
#ifdef Q_OS_WIN
	appName += "/mastersim.exe";
#else // Q_OS_WIN
	appName += "/mastersim";


#ifdef Q_OS_MAC
	// on MAC we check if the executable is inside the application bundle (deployment mode), or if not, we
	// expect it alongside the mastersim-ui.app file
	if (!QFile::exists(appName))
		appName = MSIMSettings::instance().m_installDir + "/../../../mastersim";
#endif

#endif // Q_OS_WIN
	m_solverName = appName;

	QString projectFile = MSIMProjectHandler::instance().projectFile();

	m_commandLine = QString("%1 %2 \"%3\"").arg(
				appName,
				m_commandLineArgs.join(" "),
				projectFile
			);

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


void MSIMViewSimulation::on_comboBoxVerbosityLevel_currentIndexChanged(int) {
	updateCommandLine();
}


void MSIMViewSimulation::on_comboBoxMasterAlgorithm_currentIndexChanged(int index) {
	MASTER_SIM::Project p = project(); // create copy of project
	p.m_masterMode = (MASTER_SIM::Project::MasterMode)index;

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push();
}


void MSIMViewSimulation::on_comboBoxErrorControl_currentIndexChanged(int index) {
	MASTER_SIM::Project p = project(); // create copy of project
	p.m_errorControlMode = (MASTER_SIM::Project::ErrorControlMode)index;

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push();
}


void MSIMViewSimulation::on_spinBoxMaxIteration_valueChanged(int arg1) {
	MASTER_SIM::Project p = project(); // create copy of project
	p.m_maxIterations = arg1;

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push();
}





void MSIMViewSimulation::on_comboBoxStartTimeUnit_currentIndexChanged(int) {
	MASTER_SIM::Project p = project(); // create copy of project
	p.m_tStart.IO_unit.set( m_ui->comboBoxStartTimeUnit->currentText().toStdString());

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push();
}


void MSIMViewSimulation::on_comboBoxEndTimeUnit_currentIndexChanged(int) {
	MASTER_SIM::Project p = project(); // create copy of project
	p.m_tEnd.IO_unit.set( m_ui->comboBoxEndTimeUnit->currentText().toStdString());

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push();
}


void MSIMViewSimulation::on_comboBoxMinDtUnit_currentIndexChanged(int) {
	MASTER_SIM::Project p = project(); // create copy of project
	p.m_hMin.IO_unit.set( m_ui->comboBoxMinDtUnit->currentText().toStdString());

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push();
}


void MSIMViewSimulation::on_comboBoxMaxDtUnit_currentIndexChanged(int) {
	MASTER_SIM::Project p = project(); // create copy of project
	p.m_hMax.IO_unit.set( m_ui->comboBoxMaxDtUnit->currentText().toStdString());

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push();
}


void MSIMViewSimulation::on_comboBoxDtIterLimitUnit_currentIndexChanged(int) {
	MASTER_SIM::Project p = project(); // create copy of project
	p.m_hFallBackLimit.IO_unit.set( m_ui->comboBoxDtIterLimitUnit->currentText().toStdString());

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push();
}


void MSIMViewSimulation::on_comboBoxDtOutputUnit_currentIndexChanged(int) {
	MASTER_SIM::Project p = project(); // create copy of project
	p.m_hOutputMin.IO_unit.set( m_ui->comboBoxDtOutputUnit->currentText().toStdString());

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push();
}


void MSIMViewSimulation::on_comboBoxDtStartUnit_currentIndexChanged(int) {
	MASTER_SIM::Project p = project(); // create copy of project
	p.m_hStart.IO_unit.set( m_ui->comboBoxDtStartUnit->currentText().toStdString());

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push();
}


void MSIMViewSimulation::on_lineEditStartTime_editingFinished() {
	IBK::Parameter par;
	if (!lineEditToParameter(this, "tStart", par, m_ui->lineEditStartTime, m_ui->comboBoxStartTimeUnit))
		return;
	if (par.value < 0) {
		QMessageBox::critical(this, tr("Invalid input"), tr("Time must be >= 0."));
		m_ui->lineEditStartTime->selectAll();
		return;
	}

	MASTER_SIM::Project p = project(); // create copy of project
	p.m_tStart = par;

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push();
}


void MSIMViewSimulation::on_lineEditEndTime_editingFinished() {
	IBK::Parameter par;
	if (!lineEditToParameter(this, "tEnd", par, m_ui->lineEditEndTime, m_ui->comboBoxEndTimeUnit))
		return;
	if (par.value < 0) {
		QMessageBox::critical(this, tr("Invalid input"), tr("Time must be >= 0."));
		m_ui->lineEditEndTime->selectAll();
		return;
	}

	MASTER_SIM::Project p = project(); // create copy of project
	p.m_tEnd = par;

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push();
}


void MSIMViewSimulation::on_lineEditRelTol_editingFinished() {
	IBK::Parameter par;
	if (!lineEditToParameter(this, "relTol", par, m_ui->lineEditRelTol, NULL))
		return;
	if (par.value <= 0) {
		QMessageBox::critical(this, tr("Invalid input"), tr("Tolerance must be > 0."));
		m_ui->lineEditRelTol->selectAll();
		return;
	}

	MASTER_SIM::Project p = project(); // create copy of project
	p.m_relTol = par.value;

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push();
}


void MSIMViewSimulation::on_lineEditAbsTol_editingFinished() {
	IBK::Parameter par;
	if (!lineEditToParameter(this, "absTol", par, m_ui->lineEditAbsTol, NULL))
		return;
	if (par.value <= 0) {
		QMessageBox::critical(this, tr("Invalid input"), tr("Tolerance must be > 0."));
		m_ui->lineEditAbsTol->selectAll();
		return;
	}

	MASTER_SIM::Project p = project(); // create copy of project
	p.m_absTol = par.value;

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push();
}


void MSIMViewSimulation::on_checkBoxAdjustStepSize_toggled(bool checked) {
	MASTER_SIM::Project p = project(); // create copy of project
	p.m_adjustStepSize = checked;

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push();
}


void MSIMViewSimulation::on_lineEditDtMin_editingFinished() {
	IBK::Parameter par;
	if (!lineEditToParameter(this, "hMin", par, m_ui->lineEditDtMin, m_ui->comboBoxMinDtUnit))
		return;
	if (par.value < 0) {
		QMessageBox::critical(this, tr("Invalid input"), tr("Step size must be >= 0."));
		m_ui->lineEditDtMin->selectAll();
		return;
	}

	MASTER_SIM::Project p = project(); // create copy of project
	p.m_hMin = par;

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push();
}


void MSIMViewSimulation::on_lineEditDtMax_editingFinished() {
	IBK::Parameter par;
	if (!lineEditToParameter(this, "hMax", par, m_ui->lineEditDtMax, m_ui->comboBoxMaxDtUnit))
		return;
	if (par.value < 0) {
		QMessageBox::critical(this, tr("Invalid input"), tr("Step size must be >= 0."));
		m_ui->lineEditDtMax->selectAll();
		return;
	}

	MASTER_SIM::Project p = project(); // create copy of project
	p.m_hMax = par;

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push();
}


void MSIMViewSimulation::on_lineEditDtIterLimit_editingFinished() {
	IBK::Parameter par;
	if (!lineEditToParameter(this, "hFallBackLimit", par, m_ui->lineEditDtIterLimit, m_ui->comboBoxDtIterLimitUnit))
		return;
	if (par.value < 0) {
		QMessageBox::critical(this, tr("Invalid input"), tr("Step size must be >= 0."));
		m_ui->lineEditDtIterLimit->selectAll();
		return;
	}

	MASTER_SIM::Project p = project(); // create copy of project
	p.m_hFallBackLimit = par;

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push();
}


void MSIMViewSimulation::on_lineEditDtOutput_editingFinished() {
	IBK::Parameter par;
	if (!lineEditToParameter(this, "hOutputMin", par, m_ui->lineEditDtOutput, m_ui->comboBoxDtOutputUnit))
		return;
	if (par.value < 0) {
		QMessageBox::critical(this, tr("Invalid input"), tr("Step size must be >= 0."));
		m_ui->lineEditDtOutput->selectAll();
		return;
	}

	MASTER_SIM::Project p = project(); // create copy of project
	p.m_hOutputMin = par;

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push();
}


void MSIMViewSimulation::on_lineEditDtStart_editingFinished() {
	IBK::Parameter par;
	if (!lineEditToParameter(this, "hStart", par, m_ui->lineEditDtStart, m_ui->comboBoxDtStartUnit))
		return;
	if (par.value < 0) {
		QMessageBox::critical(this, tr("Invalid input"), tr("Step size must be >= 0."));
		m_ui->lineEditDtStart->selectAll();
		return;
	}

	MASTER_SIM::Project p = project(); // create copy of project
	p.m_hStart = par;

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push();
}


void MSIMViewSimulation::on_pushButtonShowLogfile_clicked() {
	// compose log file name
	QString projectFile = MSIMProjectHandler::instance().projectFile();
	QString logfile = QFileInfo(projectFile).completeBaseName() + "/log/screenlog.txt";
	logfile = QFileInfo(projectFile).dir().absoluteFilePath(logfile);
	MSIMLogFileDialog dlg(this);
	dlg.setLogFile(logfile, projectFile, false);
	dlg.exec();
}


void MSIMViewSimulation::on_checkBoxWriteInternalVariables_toggled(bool checked) {
	MASTER_SIM::Project p = project(); // create copy of project
	p.m_writeInternalVariables = checked;

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push();
}


void MSIMViewSimulation::on_checkBoxWriteUnconnectedVariables_toggled(bool checked) {
	MASTER_SIM::Project p = project(); // create copy of project
	p.m_writeUnconnectedFileReaderVars = checked;

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push();
}


void MSIMViewSimulation::on_checkBoxPreventOversteppingOfEndTime_toggled(bool checked) {
	MASTER_SIM::Project p = project(); // create copy of project
	p.m_preventOversteppingOfEndTime = checked;

	MSIMUndoSimulationSettings * cmd = new MSIMUndoSimulationSettings(tr("Simulation setting changed"), p);
	cmd->push();
}


void MSIMViewSimulation::on_comboBoxTermEmulator_currentIndexChanged(int index) {
	MSIMSettings::instance().m_terminalEmulator = (MSIMSettings::TerminalEmulators)(index);
}

void MSIMViewSimulation::on_checkBoxSkipUnzip_toggled(bool) {
	updateCommandLine();
}



