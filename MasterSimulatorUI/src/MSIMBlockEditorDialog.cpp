#include "MSIMBlockEditorDialog.h"
#include "ui_MSIMBlockEditorDialog.h"

#include "MSIMSettings.h"
#include "MSIMProjectHandler.h"

#include <BM_Network.h>
#include <BM_SceneManager.h>

MSIMBlockEditorDialog::MSIMBlockEditorDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::MSIMBlockEditorDialog)
{

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

	return QDialog::Accepted;
//	return QDialog::Rejected;
}
