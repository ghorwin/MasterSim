#ifndef MSIMABOUTDIALOG_H
#define MSIMABOUTDIALOG_H

#include <QDialog>

namespace Ui {
class MSIMAboutDialog;
}

/*! The about dialog. */
class MSIMAboutDialog : public QDialog {
	Q_OBJECT

public:
	explicit MSIMAboutDialog(QWidget *parent = 0);
	~MSIMAboutDialog();

private:
	Ui::MSIMAboutDialog *m_ui;
};

#endif // MSIMABOUTDIALOG_H
