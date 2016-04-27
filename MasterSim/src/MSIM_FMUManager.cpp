#include "MSIM_FMUManager.h"

#include <memory> // for std::autoptr
#include <cstdlib>


#include <IBK_messages.h>
#include <IBK_Exception.h>

#include "MSIM_FMU.h"


namespace MASTER_SIM {


FMUManager::FMUManager() :
	m_unzipFMUs(true),
	m_debugLogging(true)
{
}


FMUManager::~FMUManager() {
	for (std::vector<FMU*>::iterator it = m_fmus.begin(); it != m_fmus.end(); ++it) {
		delete *it;
	}
	m_fmus.clear();
}


void FMUManager::importFMU(const IBK::Path & fmuTargetDirectory, const IBK::Path & fmuFilePath) {
	const char * const FUNC_ID = "[FMUManager::importFMU]";
	// generate unique file path
	IBK::Path extractionPath = generateFilePath(fmuTargetDirectory, fmuFilePath);
	IBK::IBK_Message(IBK::FormatString("Importing FMU: %1\n").arg(fmuFilePath), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::MessageIndentor indent; (void)indent;

	importFMUAt(fmuFilePath, extractionPath);
}


void FMUManager::importFMUAt(const IBK::Path & fmuFilePath, const IBK::Path & unzipPath) {
	const char * const FUNC_ID = "[FMUManager::importFMUAt]";
	if (m_unzipFMUs) {
		IBK::IBK_Message(IBK::FormatString("Unzipping into directory: %1\n").arg(unzipPath), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		// check if target directory exists
		if (unzipPath.exists()) {
			IBK::IBK_Message(IBK::FormatString("Directory exists, unzipping will overwrite files!"), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
		}

		// extract FMU into target directory
		FMU::unzipFMU(fmuFilePath, unzipPath);
	}

	// create FMU instance
	std::auto_ptr<FMU> fmu(new FMU(fmuFilePath, unzipPath));

	// parse modelDescription.xml so that we get the model identifyer
	fmu->readModelDescription();

	// Import dll/shared lib for current platform
	fmu->import();

}


FMU * FMUManager::fmuByPath(const IBK::Path & fmuFilePath) {
	const char * const FUNC_ID = "[FMUManager::fmuByPath]";
	for (unsigned int i=0; i<m_fmus.size(); ++i) {
		if (m_fmus[i]->m_fmuFilePath == fmuFilePath) {
			return m_fmus[i];
		}
	}
	throw IBK::Exception(IBK::FormatString("FMU with file path '%1' has not been imported, yet.").arg(fmuFilePath), FUNC_ID);
}


IBK::Path FMUManager::generateFilePath(const IBK::Path & fmuBaseDirectory, const IBK::Path & fmuFilePath) {
	IBK::Path pBase = fmuBaseDirectory / fmuFilePath.filename().withoutExtension();

	IBK::Path p = pBase;
	// Ensure uniqueness of path by checking already instantiated FMUs
	unsigned int counter = 1;
	bool found = false;
	while (found) {
		found = false;
		for (unsigned int i=0; i<m_fmus.size(); ++i) {
			if (m_fmus[i]->m_fmuDir == p) {
				found = true;
				break;
			}
		}
		if (found) {
			p = IBK::Path(IBK::FormatString("%1_%2").arg(pBase).arg(++counter).str());
		}
	}

	return p;
}



} // namespace MASTER_SIM

