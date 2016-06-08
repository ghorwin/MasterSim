#ifndef MSIMVIEWSIMULATION_H
#define MSIMVIEWSIMULATION_H

#include <QWidget>

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
	explicit MSIMViewSimulation(QWidget *parent = 0);
	~MSIMViewSimulation();

public slots:
	/*! Connected to MSIMProjectHandler::modified() */
	void onModified( int modificationType, void * data );

private slots:
	void on_toolButtonStartInTerminal_clicked();

	void on_checkBoxCloseOnExit_clicked();

	void on_comboBoxVerbosityLevel_currentIndexChanged(int index);

	void on_comboBoxMasterAlgorithm_currentIndexChanged(int index);

	void on_spinBoxMaxIteration_valueChanged(int arg1);

	void on_lineEditStartTime_editingFinished();


	void on_comboBoxStartTimeUnit_currentIndexChanged(int index);

	void on_comboBoxEndTimeUnit_currentIndexChanged(int index);

	void on_comboBoxMinDtUnit_currentIndexChanged(int index);

	void on_comboBoxMaxDtUnit_currentIndexChanged(int index);

	void on_comboBoxDtIterLimitUnit_currentIndexChanged(int index);

	void on_comboBoxDtOutputUnit_currentIndexChanged(int index);

private:
	void updateCommandLine();

	static void setupLineEditUnitCombo(QLineEdit * lineEdit, QComboBox * combo, const IBK::Parameter & p);

	QStringList				m_commandLineArgs;
	QString					m_solverName;
	QString					m_commandLine;

	Ui::MSIMViewSimulation *m_ui;
};

#endif // MSIMVIEWSIMULATION_H
