#include "MSIM_FMUManager.h"

#include <memory> // for std::autoptr

#include <IBK_messages.h>

#include "MSIM_FMUSlave.h"


namespace MASTER_SIM {

FMUManager::FMUManager() : m_unzipFMUs(true)
{
}


FMUManager::~FMUManager() {
	for (std::vector<FMUSlave*>::iterator it = m_fmus.begin(); it != m_fmus.end(); ++it) {
		delete *it;
	}
	m_fmus.clear();
}


FMUSlave * FMUManager::importFMU(const IBK::Path & fmuBaseDirectory, const IBK::Path & fmuFilePath) {
	const char * const FUNC_ID = "[FMUManager::importFMU]";
	// generate unique file path
	IBK::Path extractionPath = generateFilePath(fmuBaseDirectory, fmuFilePath);
	IBK::IBK_Message(IBK::FormatString("Importing FMU: %1\n").arg(fmuFilePath), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::MessageIndentor indent; (void)indent;
	IBK::IBK_Message(IBK::FormatString("Unzipping into directory: %1\n").arg(extractionPath), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

	return importFMU(fmuBaseDirectory, fmuFilePath, extractionPath);
}


FMUSlave * FMUManager::importFMU(const IBK::Path & fmuBaseDirectory, const IBK::Path & fmuFilePath, const IBK::Path & userOverridePath) {
	if (m_unzipFMUs) {
		/// \todo check if target directory exists

	}

	/// \todo sanity checks:
	///   dll/shared lib for current platform available
	///   ModelDescription.xml available

	return NULL;
}


IBK::Path FMUManager::generateFilePath(const IBK::Path & fmuBaseDirectory, const IBK::Path & fmuFilePath) {
	IBK::Path p = fmuBaseDirectory / fmuFilePath.filename().withoutExtension();

	/// \todo Ensure uniqueness of path by checking already instantiated FMUs
	return p;
}

} // namespace MASTER_SIM

