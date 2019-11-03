#include "MSIMBlockEditorDialog.h"
#include "ui_MSIMBlockEditorDialog.h"

#include <QLineEdit>

#include "MSIMSettings.h"
#include "MSIMProjectHandler.h"
#include "MSIMMainWindow.h"

#include <BM_Network.h>
#include <BM_SceneManager.h>
#include <BM_Globals.h>
#include <BM_BlockItem.h>


MSIMBlockEditorDialog::MSIMBlockEditorDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::MSIMBlockEditorDialog),
	m_blockItem(nullptr)
{
	m_ui->setupUi(this);
	m_ui->verticalLayoutGraphicsView->setMargin(0);
	m_ui->graphicsView->setResolution(1);
	m_ui->graphicsView->setGridStep(BLOCKMOD::Globals::GridSpacing*10);
}


MSIMBlockEditorDialog::~MSIMBlockEditorDialog() {
	delete m_ui;
}



int MSIMBlockEditorDialog::editBlock(const BLOCKMOD::Block & b, const IBK::Path & /*fmuPath*/,
									 const QStringList & inletSockets, const QStringList & outletSockets)
{
	m_modifiedBlock = b; // create copy
	m_inletSockets = inletSockets;
	m_outletSockets = outletSockets;

	// first store block index in network's block list
	// Mind to use network() in sceneManager() here, so that the pointers of the graphics item match those of
	// the scene.
	const BLOCKMOD::Network & n = MSIMProjectHandler::instance().sceneManager()->network();
	// determine index of block to modify
	m_modifiedBlockIdx = n.m_blocks.size();
	auto bit = n.m_blocks.begin();
	for (unsigned int i=0; i<n.m_blocks.size(); ++i, ++bit) {
		if (&(*bit) == &b) {
			m_modifiedBlockIdx = i;
			break;
		}
	}
	Q_ASSERT(m_modifiedBlockIdx != n.m_blocks.size());

	// populate GUI
	m_ui->lineEditName->setText(b.m_name);
	m_ui->lineEditSocketInfo->setText(tr("%1 inlets, %2 outlets").arg(inletSockets.count()).arg(outletSockets.count()));

	// determine number of block grid lines
	int rowCount = (int)std::floor(b.m_size.height() / (double)BLOCKMOD::Globals::GridSpacing + 0.5);
	int colCount = (int)std::floor(b.m_size.width() / (double)BLOCKMOD::Globals::GridSpacing + 0.5);

	m_ui->spinBoxRows->setValue(rowCount);
	m_ui->spinBoxColumns->setValue(colCount);

	// finally, create block item and a scene and show in graphics view
	QGraphicsScene * scene = new QGraphicsScene(this);
	m_ui->graphicsView->setScene(scene);

	// now auto-matically layout sockets and create missing sockets
	m_modifiedBlock.autoUpdateSockets(m_inletSockets, m_outletSockets);

	QPixmap p = MSIMMainWindow::instance().modelPixmap(m_modifiedBlock.m_name.toStdString());
	if (p.isNull())
		m_modifiedBlock.m_properties.remove("Pixmap");
	else
		m_modifiedBlock.m_properties["Pixmap"] = QVariant(p);
	m_modifiedBlock.m_properties["ShowPixmap"] = m_ui->checkBoxShowFMUPixmap->isChecked();

	// TODO : use different block item?
	m_blockItem = new BLOCKMOD::BlockItem(&m_modifiedBlock);
	m_blockItem->setRect(0, 0, m_modifiedBlock.m_size.width(), m_modifiedBlock.m_size.height());
	m_blockItem->setPos(0, 0);
	m_blockItem->setFlags(QGraphicsItem::GraphicsItemFlags());
	m_ui->graphicsView->scene()->addItem(m_blockItem);

	return exec();
}


void MSIMBlockEditorDialog::on_pushButtonLayoutSockets_clicked() {
	// get block size from spin boxes
	int rowCount = m_ui->spinBoxRows->value();
	int colCount = m_ui->spinBoxColumns->value();

	m_modifiedBlock.m_size = QSizeF(BLOCKMOD::Globals::GridSpacing*colCount, BLOCKMOD::Globals::GridSpacing*rowCount);
	m_modifiedBlock.m_sockets.clear(); // remove existing sockets to cause a relayouting

	// now auto-matically layout sockets and create missing sockets
	m_modifiedBlock.autoUpdateSockets(m_inletSockets, m_outletSockets);

	// TODO : use different block item?
	delete m_blockItem;
	m_blockItem = new BLOCKMOD::BlockItem(&m_modifiedBlock);
	m_blockItem->setRect(0, 0, m_modifiedBlock.m_size.width(), m_modifiedBlock.m_size.height());
	m_blockItem->setPos(0, 0);
	m_blockItem->setFlags(QGraphicsItem::GraphicsItemFlags());
	m_ui->graphicsView->scene()->addItem(m_blockItem);
}


void MSIMBlockEditorDialog::on_spinBoxColumns_valueChanged(int) {
	int rowCount = m_ui->spinBoxRows->value();
	int colCount = m_ui->spinBoxColumns->value();
	if (m_blockItem != nullptr)
		m_blockItem->resize((int)BLOCKMOD::Globals::GridSpacing*colCount, (int)BLOCKMOD::Globals::GridSpacing*rowCount);
}


void MSIMBlockEditorDialog::on_spinBoxRows_valueChanged(int) {
	on_spinBoxColumns_valueChanged(0);
}

void MSIMBlockEditorDialog::on_checkBoxShowFMUPixmap_clicked() {
	// if checked, set pixmap property in block
	m_modifiedBlock.m_properties["ShowPixmap"] = m_ui->checkBoxShowFMUPixmap->isChecked();
	m_blockItem->update();
}
