#include "MSIMUndoCommandBase.h"

#include "MSIMProjectHandler.h"
#include "MSIMMainWindow.h"

#include <BM_SceneManager.h>

void MSIMUndoCommandBase::push() {
	MSIMMainWindow::addUndoCommand(this);
}


MASTER_SIM::Project & MSIMUndoCommandBase::theProject() const {
	return const_cast<MASTER_SIM::Project &>( project() );
}


void MSIMUndoCommandBase::setNetwork(const BLOCKMOD::Network & network, bool projectOnly) {
	MSIMProjectHandler::instance().m_network = network;
	if (!projectOnly)
		MSIMProjectHandler::instance().m_sceneManager->setNetwork(network);
}

