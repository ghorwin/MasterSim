#ifndef MSIMUndoCommandBaseH
#define MSIMUndoCommandBaseH

#include <QUndoCommand>
#include <QCoreApplication>

#include <MSIM_Project.h>
#include "MSIMProjectHandler.h"

/*! Abstract base class for all undo commands.
	It provides the member function push() which puts the command to the global stack.
	Also, via theProject() read/write access to the project data is granted.
*/
class MSIMUndoCommandBase : public QUndoCommand {
	Q_DECLARE_TR_FUNCTIONS(MSIMUndoCommands)
public:
	/*! Pushes the command to the global undo-stack (in the main window). */
	void push();

protected:
	/*! Returns a read/write reference to the project data, so that commands can
		change the project data. */
	MASTER_SIM::Project & theProject() const;

	/*! Gives write access to the network managed in parallel to the project(). */
	void setNetwork(const BLOCKMOD::Network & network, bool projectOnly = false);
};

#endif // MSIMUndoCommandBaseH
