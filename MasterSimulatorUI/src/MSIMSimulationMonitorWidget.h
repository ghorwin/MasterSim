#ifndef MSIMSIMULATIONMONITORWIDGET_H
#define MSIMSIMULATIONMONITORWIDGET_H

#include <QWidget>

#include <IBK_Path.h>

namespace Ui {
	class MSIMSimulationMonitorWidget;
}

class MSIMSimulationMonitorWidget : public QWidget {
	Q_OBJECT
public:
	explicit MSIMSimulationMonitorWidget(QWidget *parent = nullptr);
	~MSIMSimulationMonitorWidget();

	void run(const IBK::Path & projectFile, int verbosityLevel);

public slots:
	/*! A signal that passes a new message in html format. */
	void onNewMessageReceived(const QString & message);
	/*! A signal that passes only the last line of the output in plain text format. */
	void lastLineOfMessage(const QString & lastLineOfMessage);

private:
	Ui::MSIMSimulationMonitorWidget *ui;
};

#endif // MSIMSIMULATIONMONITORWIDGET_H
