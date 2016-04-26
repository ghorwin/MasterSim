#include "MSIM_FMUManager.h"

#include <memory> // for std::autoptr
#include <cstdlib>

#include <IBK_messages.h>

#include "MSIM_FMUSlave.h"

#include <fmilib.h>
#include <FMI/fmi_zip_unzip.h>

// FMI Library Callbacks

void logger_callback(jm_callbacks* c, jm_string module, jm_log_level_enu_t log_level, jm_string message) {
	switch(log_level) {
		case jm_log_level_fatal:
		case jm_log_level_error:
			IBK::IBK_Message(IBK::FormatString("FMI-Error in Module '%1' at Log Level %2: %3")
							 .arg(module).arg(log_level).arg(message), IBK::MSG_ERROR);
			break;
		default:
			IBK::IBK_Message(IBK::FormatString("%1\n").arg(message), IBK::MSG_PROGRESS, module);
	}
}


namespace MASTER_SIM {


FMUManager::FMUManager() :
	m_unzipFMUs(true),
	m_debugLogging(true)
{
}


FMUManager::~FMUManager() {
	for (std::vector<FMUObject*>::iterator it = m_fmus.begin(); it != m_fmus.end(); ++it) {
		delete *it;
	}
	m_fmus.clear();
}


FMUObject * FMUManager::importFMU(const IBK::Path & fmuBaseDirectory, const IBK::Path & fmuFilePath) {
	const char * const FUNC_ID = "[FMUManager::importFMU]";
	// generate unique file path
	IBK::Path extractionPath = generateFilePath(fmuBaseDirectory, fmuFilePath);
	IBK::IBK_Message(IBK::FormatString("Importing FMU: %1\n").arg(fmuFilePath), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::MessageIndentor indent; (void)indent;

	return importFMU(fmuBaseDirectory, fmuFilePath, extractionPath);
}


FMUObject * FMUManager::importFMU(const IBK::Path & fmuBaseDirectory, const IBK::Path & fmuFilePath,
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

		// set call back functions for FMI library
		jm_callbacks callbacks;
		callbacks.malloc = malloc;
		callbacks.calloc = calloc;
		callbacks.realloc = realloc;
		callbacks.free = free;
		callbacks.logger = logger_callback;
		if (m_debugLogging)
			callbacks.log_level = jm_log_level_debug;
		else
			callbacks.log_level = jm_log_level_error;
		callbacks.context = NULL;

		// unzip FMU
		jm_status_enu_t status = fmi_zip_unzip(fmuFilePath.str().c_str(), unzipPath.str().c_str(), &callbacks);
		if (status == jm_status_error) {
			IBK::IBK_Message("Error uncompressing FMU file.", IBK::MSG_ERROR);
			return false;
		}
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

} // namespace MASTER_SIM

