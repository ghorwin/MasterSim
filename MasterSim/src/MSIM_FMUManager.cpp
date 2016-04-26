#include "MSIM_FMUManager.h"

#include <memory> // for std::autoptr
#include <cstdlib>

#include <miniunz.h>

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


FMU * FMUManager::importFMU(const IBK::Path & fmuBaseDirectory, const IBK::Path & fmuFilePath) {
	const char * const FUNC_ID = "[FMUManager::importFMU]";
	// generate unique file path
	IBK::Path extractionPath = generateFilePath(fmuBaseDirectory, fmuFilePath);
	IBK::IBK_Message(IBK::FormatString("Importing FMU: %1\n").arg(fmuFilePath), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::MessageIndentor indent; (void)indent;

	return importFMU(fmuBaseDirectory, fmuFilePath, extractionPath);
}


FMU * FMUManager::importFMU(const IBK::Path & fmuBaseDirectory, const IBK::Path & fmuFilePath,
								  const IBK::Path & unzipPath)
{
	const char * const FUNC_ID = "[FMUManager::importFMU]";
	if (m_unzipFMUs) {
		IBK::IBK_Message(IBK::FormatString("Unzipping into directory: %1\n").arg(unzipPath), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		// check if target directory exists
		if (unzipPath.exists()) {
			IBK::IBK_Message(IBK::FormatString("Directory exists, unzipping will overwrite files!"), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
		}

		// extract FMU into target directory
		unzipFMU(fmuFilePath, unzipPath);
	}

	/// \todo sanity checks:
	///   dll/shared lib for current platform available
	///   ModelDescription.xml available

	return NULL;
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
			if (m_fmus[i]->m_fmuUnzipDirectory == p) {
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


void FMUManager::unzipFMU(const IBK::Path & pathToFMU, const IBK::Path & extractionPath) {
	const char * const FUNC_ID = "[FMUManager::unzipFMU]";
	const char *argv[6];
	argv[0]="miniunz";
	argv[1]="-x";
	argv[2]="-o";
	argv[3]=pathToFMU.str().c_str();
	argv[4]="-d";
	argv[5]=extractionPath.str().c_str();

	if (!IBK::Path::makePath(extractionPath))
		throw IBK::Exception(IBK::FormatString("Cannot create extraction path '%1'").arg(extractionPath), FUNC_ID);

	int res = miniunz(6, (char**)argv);
	if (res != 0)
		throw IBK::Exception(IBK::FormatString("Error extracting fmu '%1' into target directory '%2'").arg(pathToFMU).arg(extractionPath), "[FMUManager::unzipFMU]");
}

} // namespace MASTER_SIM

