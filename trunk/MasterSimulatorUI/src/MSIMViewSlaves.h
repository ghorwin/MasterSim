#ifndef MSIMVIEWSLAVES_H
#define MSIMVIEWSLAVES_H

#include <QWidget>

namespace QPW {
	class VariantPropertyManager;
}

namespace Ui {
	class MSIMViewSlaves;
}

namespace IBK {
	class Path;
}

namespace MASTER_SIM {
	class ModelDescription;
}

namespace BLOCKMOD {
	class BlockItem;
}

class MSIMBlockEditorDialog;

/*! The view containing FMU and slave definition tables. */
class MSIMViewSlaves : public QWidget {
	Q_OBJECT
public:
	/*! C'tor */
	explicit MSIMViewSlaves(QWidget *parent = 0);
	/*! D'tor */
	~MSIMViewSlaves();

	/*! Extract the FMU with the given absolute file path, and determined properties for analysis.
		If successful, returns true and the modelDesc data structure can be accessed to provide the
		required information.
		The msgLog is a log of the operations done and can be shown in a plain text edit field.
	*/
	static bool extractFMUAndParseModelDesc(const IBK::Path & fmuFilePath,
									QString & msgLog,
									MASTER_SIM::ModelDescription & modelDesc);

public slots:
	/*! Connected to MSIMProjectHandler::modified() */
	void onModified( int modificationType, void * data );

signals:
	/*! Emitted when a new slave has been added.
		The argument passed is the **absolute file path** to the slave (FMU or tsv/csv table).
	*/
	void newSlaveAdded(const QString & fmuFilePath);

private slots:
	void on_toolButtonAddSlave_clicked();

	void on_toolButtonRemoveSlave_clicked();

	void on_tableWidgetSlaves_cellChanged(int row, int column);

	void on_checkBoxRelativeFMUPaths_toggled(bool checked);

	void on_tableWidgetSlaves_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

	void on_toolButtonCreateConnection_clicked();

	/*! Connected to the scene's block triggering action (usually double-click on block item). */
	void onBlockActionTriggered(const BLOCKMOD::BlockItem * blockItem);

	/*! Function is called from single-shot timer once a block-action trigger slot has finished.
		Creates an undo-action for a block change.
	*/
	void onBlockEditingCompleted();

	/*! Called from sceneManager() whenever user has finished creating a connection. */
	void onNewConnectionCreated();

	/*! Called from sceneManager() whenever a block or connector has been moved. */
	void onNetworkGeometryChanged();

private:
	/*! Updates the table with all slaves defined for this simulation scenario. */
	void updateSlaveTable();

	/*! This function is called from onModified(), whenever slave data or FMU specs have changed.
		The function processes all slaves and associated FMU data (if read), and checks if
		the block names and socket number/types/names match those of the FMU slave.
	*/
	void syncCoSimNetworkToBlocks();


	Ui::MSIMViewSlaves				*m_ui;

	/*! The manager for the properties. */
	QPW::VariantPropertyManager		*m_variantManager;

	MSIMBlockEditorDialog			*m_blockEditorDialog;
};

#endif // MSIMVIEWSLAVES_H
