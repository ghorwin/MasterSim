#include "MSIMUndoCommandBase.h"

#include "MSIMProjectHandler.h"
#include "MSIMMainWindow.h"

void MSIMUndoCommandBase::push() {
	MSIMMainWindow::addUndoCommand(this);
}

MASTER_SIM::Project & MSIMUndoCommandBase::theProject() const {
	return const_cast<MASTER_SIM::Project &>( project() );
}
