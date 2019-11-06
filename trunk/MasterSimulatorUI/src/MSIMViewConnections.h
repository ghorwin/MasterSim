#ifndef MSIMVIEWCONNECTIONS_H
#define MSIMVIEWCONNECTIONS_H

#include <QWidget>

namespace Ui {
class MSIMViewConnections;
}

class QTableWidgetItem;
class MSIMConnectionPropertiesEditDialog;

/*! The view where connections can be defined. */
class MSIMViewConnections : public QWidget {
	Q_OBJECT

public:
	explicit MSIMViewConnections(QWidget *parent = nullptr);
	~MSIMViewConnections() override;

public slots:
	/*! Connected to MSIMProjectHandler::modified() */
	void onModified(unsigned int modificationType, void * data );


protected:
	/*! Adjusts table column widths. */
	void resizeEvent(QResizeEvent *event) override;

	/*! Calls resizeTableColumns() on first show. */
	void showEvent(QShowEvent *event) override;

private slots:

	void on_tableWidgetSlaves_cellChanged(int row, int column);

	void on_toolButtonAddConnection_clicked();
	void on_toolButtonRemoveConnection_clicked();
	void on_pushButtonConnectByVariableName_clicked();

	void on_tableWidgetOutputVariable_itemDoubleClicked(QTableWidgetItem *item);

	void on_tableWidgetInputVariable_itemDoubleClicked(QTableWidgetItem *item);

	void on_tableWidgetConnections_itemDoubleClicked(QTableWidgetItem *item);

private:
	void updateConnectionsTable();
	void updateInputOutputVariablesTables();
	void resizeTableColumns();

	Ui::MSIMViewConnections				*m_ui;

	MSIMConnectionPropertiesEditDialog	*m_connectionPropertiesEditDialog;
};

#endif // MSIMVIEWCONNECTIONS_H
