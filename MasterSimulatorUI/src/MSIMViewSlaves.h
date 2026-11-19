#ifndef MSIMVIEWSLAVES_H
#define MSIMVIEWSLAVES_H

#include <QWidget>

#include <MSIM_Project.h>

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
class QTableWidgetItem;
class MSIMExportConnectionGraphDialog;

/*! The view containing FMU and slave definition tables. */
class MSIMViewSlaves : public QWidget {
	Q_OBJECT

public:

	/*! Describes what is currently selected in the view, which determines the page shown in the
		property stacked widget (right side of the view) and which edit buttons are enabled.

		The selection can originate from the slave table (tableWidgetSlaves) or from the network
		schematic (BlockMod scene). Both are kept consistent via updatePropertyStackedWidget(), which is
		the only function that changes m_selectionState.

		Invariants (enforced by updatePropertyStackedWidget()):
		- SS_SlaveSelected     : tableWidgetSlaves->currentRow() is a valid row index, m_selectedEdgeIdx == -1
		- SS_ConnectorSelected : m_selectedEdgeIdx is a valid index into project().m_graph, the slave table
								 has neither a selection nor a current cell
		- SS_NothingSelected   : m_selectedEdgeIdx == -1, the slave table has neither a selection nor a current cell
	*/
	enum SelectionState {
		/*! A slave is selected (row in the slave table, or block in the scene).
			Property page 0 (slave parameters) is shown, the "remove slave" button is enabled.
		*/
		SS_SlaveSelected,
		/*! A connector (graph edge) is selected in the scene, index is stored in m_selectedEdgeIdx.
			Property page 1 (connector properties) is shown, the "delete connection" button is enabled.
		*/
		SS_ConnectorSelected,
		/*! Nothing is selected.
			Property page 2 (project properties/description) is shown, both remove/delete buttons are disabled.
		*/
		SS_NothingSelected
	};


	/*! C'tor */
	explicit MSIMViewSlaves(QWidget *parent = nullptr);
	/*! D'tor */
	~MSIMViewSlaves();

	/*! Looks up the block item corresponding to the given slave and opens the block editor. */
	void editBlockItem(const QString & slaveName);

	/*! Opens print/export dialog and exports/prints scene. */
	void printScene();

	/*! Extract the FMU with the given absolute file path, and determined properties for analysis.
		If successful, returns true and the modelDesc data structure can be accessed to provide the
		required information.
		The msgLog is a log of the operations done and can be shown in a plain text edit field.
	*/
	static bool extractFMUAndParseModelDesc(const IBK::Path & fmuFilePath,
									QString & msgLog,
									MASTER_SIM::ModelDescription & modelDesc,
									QPixmap & modelPixmap);

public slots:
	/*! Connected to MSIMProjectHandler::modified() */
	void onModified(unsigned int modificationType, void * data );

	/*! Clears the selection in slave table and network scene and shows the project properties page.
		Connected to the Escape key shortcut.
	*/
	void clearSelection();

signals:
	/*! Emitted when a new slave has been added.
		The second argument passed is the **absolute file path** to the slave (FMU or tsv/csv table).
	*/
	void newSlaveAdded(const QString & slaveName, const QString & fmuFilePath);

private slots:
	void on_toolButtonAddSlave_clicked();

	void on_toolButtonRemoveSlave_clicked();

	void on_tableWidgetSlaves_cellChanged(int row, int column);

	void on_checkBoxRelativeFMUPaths_toggled(bool checked);

	void on_tableWidgetSlaves_currentCellChanged(int , int , int , int );

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

	/*! Called from sceneManager() whenever a block has been selected. */
	void onBlockSelected(const QString & blockName);

	/*! Called from sceneManager() whenever a connector has been selected. */
	void onConnectorSelected(const QString & sourceSocketName, const QString & targetSocketName);

	/*! Called from sceneManager() whenever selection has changed and is now empty (there is nothing selected) */
	void onSelectionCleared();

	/*! User has edited a parameter of a slave. */
	void on_widgetProperties_itemChanged(QTableWidgetItem *item);

	void on_widgetConnectors_itemChanged(QTableWidgetItem *item);

	void on_pushButtonDeleteConnection_clicked();

	void on_doubleSpinBoxLinewidth_valueChanged(double arg1);

	void on_pushButtonSelectColor_colorChanged();

	void on_checkBoxShowEquations_clicked(bool checked);

	void on_checkBoxMergeConnectors_clicked(bool checked);


	void on_textEditDescription_editingFinished();

	void on_pushButtonExportImage_clicked();

private:
	/*! Updates the table with all slaves defined for this simulation scenario. */
	void updateSlaveTable();

	/*! Populates the slave-specific parameter table which shows all parameters set by MasterSim.
		\param slaveIndex An index of a slave/simulator in the project. -1 means non selected/available.
	*/
	void updateSlaveParameterTable(unsigned int slaveIndex);

	/*! Populates the table widget with offset, scaleFactor values and updates the linewidth spinBox
		\param edge the current
	*/
	void updateGraphProperties();

	/*! Updates Ui property page. Shows slave properties, connector properties or nothing based on current selection state.
		Sets m_selectionState and enforces the invariants documented for SelectionState. If the requested
		state cannot be established (no current slave row, or invalid m_selectedEdgeIdx), SS_NothingSelected is used.
		\note Does not modify the selection in the network scene, see syncSceneSelection().
	*/
	void updatePropertyStackedWidget(SelectionState selectionState);

	/*! Transfers the current selection state (m_selectionState) to the network scene, i.e. selects
		the block of the current slave or all segments of the selected connector.
		Needed when selection is changed from the slave table/keyboard, and after the scene's items
		have been re-created (syncCoSimNetworkToBlocks() resets the network and thus loses the selection).
	*/
	void syncSceneSelection();

	Ui::MSIMViewSlaves				*m_ui;

	/*! The manager for the properties. */
	QPW::VariantPropertyManager		*m_variantManager = nullptr;

	MSIMExportConnectionGraphDialog	*m_exportConnectionGraphDialog = nullptr;

	MSIMBlockEditorDialog			*m_blockEditorDialog;

	/*! Index of currently selected connector/graph edge in project().m_graph, -1 if no connector is selected. */
	int								m_selectedEdgeIdx = -1;

	/*! Currently shown selection state, only modified in updatePropertyStackedWidget(). */
	SelectionState					m_selectionState = SS_NothingSelected;

	/*! Set to true while syncSceneSelection() modifies the scene selection, so that selection
		signals from the scene manager are ignored.
	*/
	bool							m_syncingSceneSelection = false;
};

#endif // MSIMVIEWSLAVES_H
