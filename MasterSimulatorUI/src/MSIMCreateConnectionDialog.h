#ifndef MSIMCREATECONNECTIONDIALOG_H
#define MSIMCREATECONNECTIONDIALOG_H

#include <QDialog>

#include <string>

namespace Ui {
class MSIMCreateConnectionDialog;
}

class MSIMCreateConnectionDialog : public QDialog {
	Q_OBJECT

public:
	explicit MSIMCreateConnectionDialog(QWidget *parent = 0);
	~MSIMCreateConnectionDialog();

	void updateTables();

private slots:

private:
	Ui::MSIMCreateConnectionDialog *m_ui;
};

#endif // MSIMCREATECONNECTIONDIALOG_H
