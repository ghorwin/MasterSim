#ifndef MSIMVIEWSLAVES_H
#define MSIMVIEWSLAVES_H

#include <QWidget>

namespace Ui {
class MSIMViewSlaves;
}

/*! The view containing FMU and slave definition tables. */
class MSIMViewSlaves : public QWidget {
	Q_OBJECT
public:
	explicit MSIMViewSlaves(QWidget *parent = 0);
	~MSIMViewSlaves();

public slots:
	/*! Connected to MSIMProjectHandler::modified() */
	void onModified( int modificationType, void * data );

private slots:
	void on_toolButtonAddSlave_clicked();

	void on_toolButtonRemoveSlave_clicked();

	void on_tableWidgetSlaves_cellChanged(int row, int column);

	void on_checkBoxRelativeFMUPaths_toggled(bool checked);

private:
	void updateSlaveTable();

	Ui::MSIMViewSlaves *m_ui;
};

#endif // MSIMVIEWSLAVES_H
