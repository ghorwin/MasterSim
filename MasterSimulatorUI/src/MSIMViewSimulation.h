#ifndef MSIMVIEWSIMULATION_H
#define MSIMVIEWSIMULATION_H

#include <QWidget>
#include <QProcess>

class QTimer;

namespace Ui {
class MSIMViewSimulation;
}

class QLineEdit;
class QComboBox;

namespace IBK {
	class Parameter;
}

/*! The view containing FMU and slave definition tables. */
class MSIMViewSimulation : public QWidget {
	Q_OBJECT
public:
	explicit MSIMViewSimulation(QWidget *parent = nullptr);
	~MSIMViewSimulation();

	/*! Returns true if a simulation started as background process is still running. */
	bool isBackgroundSimulationRunning() const;

public slots:
	/*! Connected to MSIMProjectHandler::modified() */
	void onModified(unsigned int modificationType, void * data );

	void on_toolButtonStartInTerminal_clicked();

private slots:

	void on_comboBoxVerbosityLevel_currentIndexChanged(int index);


	void on_comboBoxMasterAlgorithm_currentIndexChanged(int index);
	void on_comboBoxErrorControl_currentIndexChanged(int index);

	void on_spinBoxMaxIteration_valueChanged(int arg1);
	void on_spinBoxMaxInputDerivativeOrder_valueChanged(int arg1);

	void on_comboBoxStartTimeUnit_currentIndexChanged(int index);
	void on_comboBoxEndTimeUnit_currentIndexChanged(int index);
	void on_comboBoxMinDtUnit_currentIndexChanged(int index);
	void on_comboBoxMaxDtUnit_currentIndexChanged(int index);
	void on_comboBoxDtIterLimitUnit_currentIndexChanged(int index);
	void on_comboBoxDtOutputUnit_currentIndexChanged(int index);

	void on_lineEditStartTime_editingFinished();
	void on_lineEditEndTime_editingFinished();

	void on_lineEditRelTol_editingFinished();
	void on_lineEditAbsTol_editingFinished();


	void on_lineEditDtMin_editingFinished();
	void on_lineEditDtMax_editingFinished();
	void on_lineEditDtIterLimit_editingFinished();
	void on_lineEditDtOutput_editingFinished();

	void on_checkBoxAdjustStepSize_toggled(bool checked);

	void on_comboBoxDtStartUnit_currentIndexChanged(int index);

	void on_lineEditDtStart_editingFinished();

	void on_pushButtonShowLogfile_clicked();

	void on_checkBoxWriteInternalVariables_toggled(bool checked);
	void on_checkBoxWriteUnconnectedVariables_toggled(bool checked);

	void on_checkBoxPreventOversteppingOfEndTime_toggled(bool checked);

	void on_comboBoxTermEmulator_currentIndexChanged(int index);

	void on_checkBoxSkipUnzip_toggled(bool checked);

	/*! Connected to QProcess::errorOccurred() of background solver process. */
	void onSolverProcessErrorOccurred(QProcess::ProcessError error);
	/*! Connected to QProcess::finished() of background solver process. */
	void onSolverProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
	/*! Connected to progress timer, reads progress.txt and updates progress bar. */
	void onUpdateProgress();

private:
	void updateCommandLine();

	/*! Starts solver as background process (no terminal), monitors errors and progress. */
	void startBackgroundProcess(const QString & projectFile, const QStringList & commandLineArgs);

	/*! Re-enables UI after background process has ended. */
	void backgroundProcessEnded();

	/*! Shows an error message with reference to the solver log file, and offers to show the log file. */
	void showSolverErrorMessage(const QString & msg);

	static void setupLineEditUnitCombo(QLineEdit * lineEdit, QComboBox * combo, const IBK::Parameter & p);

	QStringList				m_commandLineArgs;
	QString					m_solverName;
	QString					m_commandLine;

	/*! Background solver process (only used for run option "None (background process)"). */
	QProcess				*m_solverProcess = nullptr;
	/*! Timer to poll progress.txt while background solver process is running. */
	QTimer					*m_progressTimer = nullptr;
	/*! Path to progress.txt of currently running background simulation. */
	QString					m_progressFile;
	/*! Path to screenlog.txt of currently running background simulation. */
	QString					m_logFile;
	/*! Project file of currently running background simulation. */
	QString					m_runningProjectFile;

	Ui::MSIMViewSimulation *m_ui;
};

#endif // MSIMVIEWSIMULATION_H
