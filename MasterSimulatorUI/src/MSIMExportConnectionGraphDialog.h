#ifndef MSIMExportConnectionGraphDialogH
#define MSIMExportConnectionGraphDialogH

#include <QDialog>

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

private:
	Ui::MSIMExportConnectionGraphDialog	*m_ui;

	BLOCKMOD::ZoomMeshGraphicsView		*m_blockModWidget;
};

#endif // MSIMExportConnectionGraphDialogH
