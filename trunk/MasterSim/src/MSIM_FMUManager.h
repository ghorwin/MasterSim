#ifndef MSIM_FMUMANAGER_H
#define MSIM_FMUMANAGER_H

#include <vector>

#include <IBK_Path.h>

namespace MASTER_SIM {

class FMU;

/*! Handles unzipping of FMUs and reading of ModelDescription files.
	The FMUManager only handles unzipping of FMU files and reading model description files. It does not
	instantiate the FMUs (there can be several model instances per FMU file).
*/
class FMUManager {
public:
	FMUManager();

	/*! Releases all allocated memory. */
	~FMUManager();

	/*! Imports an FMU.
		The actual import path (unzip directory) is auto-generated.
		\param fmuTargetDirectory Target directory for extracted FMUs.
		\param fmuFilePath Full path to FMU-file (from master project file).
		\return Returns a pointer to the FMUSlave instance that contains all data provided by this FMU.

		\note Throws an exception if an error occurs. In this case, all resources are deallocated again.
	*/
	void importFMU(const IBK::Path & fmuTargetDirectory, const IBK::Path & fmuFilePath);

	/*! Alternative version of FMU import where unzip directory is provided by user and not auto-generated.
		If another FMU had been instantiated with same unzip directory, an IBK::Exception will be thrown.
	*/
	void importFMUAt(const IBK::Path & fmuFilePath, const IBK::Path & unzipPath);

	/*! Convenience function, returns FMU by file path to fmu archive as passed to importFMU. */
	FMU * fmuByPath(const IBK::Path & fmuFilePath);

	/*! If true, FMUs are unzipped to directories first, before ModelDescriptions are read. */
	bool	m_unzipFMUs;

	/*! If true, debug logging during FMU unzipping/ModelDescription reading is enabled. */
	bool	m_debugLogging;

	/*! All FMUs already imported. */
	std::vector<FMU*>	m_fmus;

private:
	/*! Generates a unique FMU file path based on fmu base directory and fmuFilePath. */
	IBK::Path generateFilePath(const IBK::Path & fmuBaseDirectory, const IBK::Path & fmuFilePath);

};

} // namespace MASTER_SIM


#endif // MSIM_FMUMANAGER_H
