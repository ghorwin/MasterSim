#ifndef MSIMBLOCKEDITORDIALOG_H
#define MSIMBLOCKEDITORDIALOG_H

#include <QDialog>
#include <IBK_Path.h>
#include <BM_Block.h>

namespace Ui {
	class MSIMBlockEditorDialog;
}


/*! The editor for block definitions/geometry. */
class MSIMBlockEditorDialog : public QDialog {
	Q_OBJECT

public:
	explicit MSIMBlockEditorDialog(QWidget *parent = 0);
	~MSIMBlockEditorDialog();

	/*! Launches the editor to edit the block.
		\note The storage of the resultant block in member variable m_modifiedBlock is done on purpose.
			  This way, it is possible the send a QTimer::singleShot() call to delay the execution of the
			  network modification so that the original BlockItem-triggered slot can be finished first.

		\param b The block object to be edited (must be stored in the currently used network in the project's scene manager!).
			Will be copied into the dialog and upon successful return, the modified block object
			can be retrieved from m_modifiedBlock.
		\param fmuPath Full path to FMU, can be used to check for already existing block definitions
		\param inletSockets List of inlet sockets/variables defined by the FMU.
		\param outletSockets List of outlet sockets/variables defined by the FMU.

		\return Returns QDialog::Accepted when dialog was confirmed and block has been modified, otherweise QDialog::Rejected.
	*/
	int editBlock(const BLOCKMOD::Block & b, const IBK::Path & fmuPath, const QStringList & inletSockets, const QStringList & outletSockets);

	/*! Stores the block that was modified during last call of editBlock() and is updated only
		when editBlock() returns QDialog::Accepted.
	*/
	BLOCKMOD::Block m_modifiedBlock;

	/*! Index of the block in the network's block list. */
	int				m_modifiedBlockIdx;

private:
	Ui::MSIMBlockEditorDialog	*m_ui;
};

#endif // MSIMBLOCKEDITORDIALOG_H
