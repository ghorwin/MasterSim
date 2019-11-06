#ifndef MSIMConnectionPropertiesEditDialogH
#define MSIMConnectionPropertiesEditDialogH

#include <QDialog>

namespace Ui {
	class MSIMConnectionPropertiesEditDialog;
}

/*! Dialog for editing graph edge/connection properties.
	Currently it allows editing of offset and scaling factors.
*/
class MSIMConnectionPropertiesEditDialog : public QDialog {
	Q_OBJECT
public:

	explicit MSIMConnectionPropertiesEditDialog(QWidget *parent = nullptr);
	~MSIMConnectionPropertiesEditDialog();

	/*! Populates editor with values and when accepted, returns values in arguments. */
	int edit(double & offset, double & scaleFactor);

protected:
	/*! QDialog::accept() re-implemented for input data checking (called indirectly from buttonBox). */
	void accept();

private:
	Ui::MSIMConnectionPropertiesEditDialog *m_ui;
};

#endif // MSIMConnectionPropertiesEditDialogH
