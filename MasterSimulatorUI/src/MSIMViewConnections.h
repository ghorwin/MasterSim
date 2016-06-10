#ifndef MSIMVIEWCONNECTIONS_H
#define MSIMVIEWCONNECTIONS_H

#include <QWidget>

namespace Ui {
class MSIMViewConnections;
}

class MSIMCreateConnectionDialog;

/*! The view where connections can be defined. */
class MSIMViewConnections : public QWidget {
	Q_OBJECT

public:
	explicit MSIMViewConnections(QWidget *parent = 0);
	~MSIMViewConnections();

public slots:
	/*! Connected to MSIMProjectHandler::modified() */
	void onModified( int modificationType, void * data );

private slots:

	void on_tableWidgetSlaves_cellChanged(int row, int column);

	void on_toolButtonAddConnection_clicked();
	void on_toolButtonRemoveConnection_clicked();
	void on_pushButtonConnectByVariableName_clicked();

private:
	void updateConnectionsTable();

	MSIMCreateConnectionDialog *m_createConnectionDialog;

	Ui::MSIMViewConnections *m_ui;
};

#endif // MSIMVIEWCONNECTIONS_H
