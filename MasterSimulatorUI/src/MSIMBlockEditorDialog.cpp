#include "MSIMBlockEditorDialog.h"
#include "ui_MSIMBlockEditorDialog.h"

#include <QLineEdit>

#include "MSIMSettings.h"
#include "MSIMProjectHandler.h"

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



int MSIMBlockEditorDialog::editBlock(const BLOCKMOD::Block & b, const IBK::Path & fmuPath,
									 const QStringList & inletSockets, const QStringList & outletSockets)
{
	m_modifiedBlock = b; // create copy

	// first store block index in network's block list
	const BLOCKMOD::Network & n = MSIMProjectHandler::instance().sceneManager()->network();
	// determine index of block to modify
	m_modifiedBlockIdx = n.m_blocks.count();
	for (int i=0; i<n.m_blocks.count(); ++i) {
		if (&n.m_blocks[i] == &b) {
			m_modifiedBlockIdx = i;
			break;
		}
	}
	Q_ASSERT(m_modifiedBlockIdx != n.m_blocks.count());

	// populate GUI
	m_ui->lineEditName->setText(b.m_name);
	m_ui->lineEditSocketInfo->setText(tr("%1 inlets, %2 outlets").arg(inletSockets.count()).arg(outletSockets.count()));

	// determine number of block grid lines
	int rowCount = std::floor(b.m_size.height() / (double)BLOCKMOD::Globals::GridSpacing + 0.5);
	int colCount = std::floor(b.m_size.width() / (double)BLOCKMOD::Globals::GridSpacing + 0.5);

	m_ui->spinBoxRows->setValue(rowCount);
	m_ui->spinBoxColumns->setValue(colCount);

	// now auto-matically layout sockets and create missing sockets
	m_modifiedBlock.autoUpdateSockets(inletSockets, outletSockets);

	// finally, create block item and a scene and show in graphics view
	QGraphicsScene * scene = new QGraphicsScene(this);

	// TODO : use different block item?
	m_blockItem = new BLOCKMOD::BlockItem(&m_modifiedBlock);
	m_blockItem->setRect(0, 0, b.m_size.width(), b.m_size.height());
	m_blockItem->setPos(0, 0);
	m_blockItem->setFlags(0);
	//m_blockItem->setFlags(Qt::ItemIsEnabled);
	scene->addItem(m_blockItem);
	m_ui->graphicsView->setScene(scene);

	return exec();
}
