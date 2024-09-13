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


	// General stuff

	void on_buttonBox_clicked(QAbstractButton *button);

	// QWidget interface
protected:
	void showEvent(QShowEvent * event) override;

private:
	void print(QPrinter * printer, bool showMarginFrame);

	Ui::MSIMExportConnectionGraphDialog	*m_ui;

	/*! The printer object (owned). */
	QPrinter							*m_printer = nullptr;
	QPrintPreviewWidget					*m_printPreviewWidget = nullptr;
	BLOCKMOD::ZoomMeshGraphicsView		*m_blockModWidget = nullptr;

};

#endif // MSIMExportConnectionGraphDialogH
