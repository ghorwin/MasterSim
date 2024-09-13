#ifndef MSIMExportConnectionGraphDialogH
#define MSIMExportConnectionGraphDialogH

#include <QDialog>
class QPrinter;
class QPrintPreviewWidget;
class QAbstractButton;

namespace Ui {
class MSIMExportConnectionGraphDialog;
}

namespace BLOCKMOD {
	class ZoomMeshGraphicsView;
}

/*! The image export/print dialog. */
class MSIMExportConnectionGraphDialog : public QDialog {
	Q_OBJECT

public:
	explicit MSIMExportConnectionGraphDialog(QWidget *parent, BLOCKMOD::ZoomMeshGraphicsView * blockModWidget);
	~MSIMExportConnectionGraphDialog() override;

private slots:

	// Print-related stuff
	void renderPrintPreview(QPrinter * printer) { print(printer, true); }

	void on_spinBoxScaleFactor_valueChanged(int);

	void on_pushButtonPageSetup_clicked();
	void on_pushButtonPrint_clicked();

	// Image export stuff
	void on_spinBoxWidth_valueChanged(int arg1);
	void on_spinBoxHeight_valueChanged(int arg1);


	// General stuff

	void on_buttonBox_clicked(QAbstractButton *button);


	void on_radioButtonBitmap_toggled(bool checked);

	void on_pushButtonExportImage_clicked();

protected:
	void showEvent(QShowEvent * event) override;

private:
	void print(QPrinter * printer, bool showMarginFrame);
	void updatePreviewPixmap();

	Ui::MSIMExportConnectionGraphDialog	*m_ui;

	/*! The printer object (owned). */
	QPrinter							*m_printer = nullptr;
	QPrintPreviewWidget					*m_printPreviewWidget = nullptr;
	BLOCKMOD::ZoomMeshGraphicsView		*m_blockModWidget = nullptr;

};

#endif // MSIMExportConnectionGraphDialogH
