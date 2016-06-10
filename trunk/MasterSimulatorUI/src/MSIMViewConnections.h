#ifndef MSIMVIEWCONNECTIONS_H
#define MSIMVIEWCONNECTIONS_H

#include <QWidget>

namespace Ui {
class MSIMViewConnections;
}

class MSIMViewConnections : public QWidget
{
	Q_OBJECT

public:
	explicit MSIMViewConnections(QWidget *parent = 0);
	~MSIMViewConnections();

public slots:
	/*! Connected to MSIMProjectHandler::modified() */
	void onModified( int modificationType, void * data );

private slots:
	void on_pushButton_clicked();

	void on_tableWidgetSlaves_cellChanged(int row, int column);

	void on_toolButtonAddConnection_clicked();

	void on_toolButtonRemoveConnection_clicked();

private:
	void updateConnectionsTable();

	Ui::MSIMViewConnections *m_ui;
};

#endif // MSIMVIEWCONNECTIONS_H
