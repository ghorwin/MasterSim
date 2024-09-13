#ifndef MSIMExportConnectionGraphDialogH
#define MSIMExportConnectionGraphDialogH

#include <QDialog>
class QPrinter;
class QPrintPreviewWidget;

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
	void renderPrintPreview(QPrinter * printer);

	void on_spinBoxScaleFactor_valueChanged(int arg1);

	void on_pushButtonPrintSetup_clicked();

private:
	Ui::MSIMExportConnectionGraphDialog	*m_ui;

	/*! The printer object (owned). */
	QPrinter							*m_printer = nullptr;
	QPrintPreviewWidget					*m_printPreviewWidget = nullptr;
	BLOCKMOD::ZoomMeshGraphicsView		*m_blockModWidget = nullptr;
};

#endif // MSIMExportConnectionGraphDialogH
