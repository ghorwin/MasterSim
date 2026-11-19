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
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QPushButton>

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

	// progress bar is only shown while a background simulation is running
	m_ui->progressBarSimulation->setRange(0, 1000); // 0.1 % resolution
	m_ui->progressBarSimulation->setValue(0);
	m_ui->progressBarSimulation->setVisible(false);

	m_progressTimer = new QTimer(this);
	m_progressTimer->setInterval(500);
	connect(m_progressTimer, &QTimer::timeout, this, &MSIMViewSimulation::onUpdateProgress);

	blockMySignals(this, false);
}


MSIMViewSimulation::~MSIMViewSimulation() {
	if (m_solverProcess != nullptr) {
		// do not report errors/finished state of a process that we terminate ourselves
		m_solverProcess->disconnect(this);
		if (m_solverProcess->state() != QProcess::NotRunning) {
			m_solverProcess->kill();
			m_solverProcess->waitForFinished(3000);
		}
	}
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
#if defined(Q_OS_LINUX)
	// background process: run solver ourselves to monitor errors and progress
	if (runOption == MSIMSettings::TE_None) {
		startBackgroundProcess(projectFile, commandLineArgs);
		return;
	}
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
			case MSIMSettings::TE_Ptyxis:
				QMessageBox::critical(this, QString(), tr("Could not run Ptyxis terminal emulator (ptyxis). "
														  "Please install Ptyxis or select a different terminal emulator from the list!") );
				break;
		}
#else
		QMessageBox::critical(this, QString(), tr("Could not run solver '%1'").arg(m_solverName));
#endif
		return;
	}

}


bool MSIMViewSimulation::isBackgroundSimulationRunning() const {
	return m_solverProcess != nullptr && m_solverProcess->state() != QProcess::NotRunning;
}


void MSIMViewSimulation::startBackgroundProcess(const QString & projectFile, const QStringList & commandLineArgs) {
	if (m_solverProcess != nullptr && m_solverProcess->state() != QProcess::NotRunning) {
		QMessageBox::information(this, QString(), tr("A simulation is already running."));
		return;
	}

	// compose paths to log and progress files (solver's working directory is project file path without extension)
	QFileInfo finfo(projectFile);
	QString workingDir = finfo.dir().absoluteFilePath(finfo.completeBaseName());
	m_logFile = workingDir + "/log/screenlog.txt";
	m_progressFile = workingDir + "/log/progress.txt";
	m_runningProjectFile = projectFile;

	// remove progress file of a previous run (may still exist when "skip-unzip" is used),
	// so that we do not show outdated progress
	QFile::remove(m_progressFile);

	if (m_solverProcess == nullptr) {
		m_solverProcess = new QProcess(this);
		connect(m_solverProcess, &QProcess::errorOccurred, this, &MSIMViewSimulation::onSolverProcessErrorOccurred);
		connect(m_solverProcess, &QProcess::finished, this, &MSIMViewSimulation::onSolverProcessFinished);
	}
	// solver writes everything relevant to screenlog.txt; discard console output so that
	// the process does not block on full stdout/stderr pipes
	m_solverProcess->setStandardOutputFile(QProcess::nullDevice());
	m_solverProcess->setStandardErrorFile(QProcess::nullDevice());

	QStringList args = commandLineArgs;
	args << projectFile; // no quotes needed, Qt takes care of that

	m_ui->toolButtonStartInTerminal->setEnabled(false);
	m_ui->progressBarSimulation->setValue(0);
	m_ui->progressBarSimulation->setFormat(tr("Starting simulation..."));
	m_ui->progressBarSimulation->setVisible(true);

	m_solverProcess->start(m_solverName, args);
	// errors during startup are reported asynchronously via errorOccurred()
	m_progressTimer->start();
}


void MSIMViewSimulation::onSolverProcessErrorOccurred(QProcess::ProcessError error) {
	// Crashed is reported by finished() as well, and other errors (read/write/timeout) are not
	// relevant since we do not communicate with the process; only handle launch failures here.
	if (error != QProcess::FailedToStart)
		return;
	backgroundProcessEnded();
	m_ui->progressBarSimulation->setVisible(false);
	QMessageBox::critical(this, tr("Solver error"), tr("Could not run solver '%1':\n%2")
						  .arg(m_solverName, m_solverProcess->errorString()));
}


void MSIMViewSimulation::onSolverProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
	onUpdateProgress(); // read final progress
	backgroundProcessEnded();
	if (exitStatus == QProcess::CrashExit) {
		m_ui->progressBarSimulation->setFormat(tr("Simulation crashed"));
		showSolverErrorMessage(tr("The solver crashed."));
	}
	else if (exitCode != 0) {
		m_ui->progressBarSimulation->setFormat(tr("Simulation failed"));
		showSolverErrorMessage(tr("The simulation stopped with an error (exit code %1).").arg(exitCode));
	}
	else {
		m_ui->progressBarSimulation->setValue(m_ui->progressBarSimulation->maximum());
		m_ui->progressBarSimulation->setFormat(tr("Simulation finished"));
	}
}


void MSIMViewSimulation::onUpdateProgress() {
	if (m_progressFile.isEmpty())
		return;
	QFile f(m_progressFile);
	if (!f.open(QIODevice::ReadOnly))
		return; // file not yet created
	// only read the tail of the file - last line is all we need
	const qint64 TAIL_SIZE = 512;
	if (f.size() > TAIL_SIZE)
		f.seek(f.size() - TAIL_SIZE);
	QStringList lines = QString::fromUtf8(f.readAll()).split('\n', Qt::SkipEmptyParts);
	// search backwards for the last complete data line (header line or incomplete lines fail to convert)
	for (int i = lines.size() - 1; i >= 0; --i) {
		QStringList cols = lines[i].split('\t');
		if (cols.size() < 3)
			continue;
		bool ok;
		double percentage = cols.last().trimmed().toDouble(&ok);
		if (!ok)
			continue;
		percentage = qBound(0.0, percentage, 100.0);
		m_ui->progressBarSimulation->setValue(qRound(percentage*10));
		m_ui->progressBarSimulation->setFormat(QString("%1 %").arg(percentage, 0, 'f', 1));
		break;
	}
}


void MSIMViewSimulation::backgroundProcessEnded() {
	m_progressTimer->stop();
	m_ui->toolButtonStartInTerminal->setEnabled(true);
}


void MSIMViewSimulation::showSolverErrorMessage(const QString & msg) {
	QMessageBox msgBox(QMessageBox::Critical, tr("Simulation error"),
					   tr("%1\n\nSee log file '%2' for details.").arg(msg, QDir::toNativeSeparators(m_logFile)),
					   QMessageBox::Close, this);
	QPushButton * showLogButton = msgBox.addButton(tr("Show log file..."), QMessageBox::ActionRole);
	msgBox.exec();
	if (msgBox.clickedButton() == showLogButton) {
		MSIMLogFileDialog dlg(this);
		dlg.setLogFile(m_logFile, m_runningProjectFile, false);
		dlg.exec();
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



