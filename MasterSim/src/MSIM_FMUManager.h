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
	/*! Constructor. */
	FMUManager() :
		m_unzipFMUs(true)
	{
	}

	/*! Releases all allocated memory. */
	~FMUManager();

	/*! Imports an FMU to an auto-generated unzip directory.
		\param fmuTargetDirectory Target directory for extracted FMUs.
		\param fmuFilePath Full path to FMU-file (from master project file).
		\return Returns a pointer to the FMU object that contains all data provided by this FMU.

		\note Throws an exception if an error occurs. In this case, all resources are deallocated again.
	*/
	void importFMU(const IBK::Path & fmuTargetDirectory, const IBK::Path & fmuFilePath);

	/*! Alternative version of FMU import where unzip directory is provided by user and not auto-generated.
		If another FMU had been instantiated with same unzip directory, an IBK::Exception will be thrown.
		\param fmuFilePath Full path to FMU-file (from master project file).
		\param unzipPath Directory where archive shall be extracted to.
	*/
	void importFMUAt(const IBK::Path & fmuFilePath, const IBK::Path & unzipPath);

	/*! Convenience function, returns FMU by file path to fmu archive as passed to importFMU. */
	FMU * fmuByPath(const IBK::Path & fmuFilePath);

	/*! Gives access to FMUs. */
	const std::vector<FMU*> & fmus() const { return m_fmus; }

	/*! Unloads libraries and releases handles. */
	void unloadLibraries();

	/*! If true, FMUs are unzipped to directories first (the default), before the modelDescription.xml file is read. */
	bool	m_unzipFMUs;

private:
	/*! Generates a unique FMU file path based on fmu base directory and fmuFilePath. */
	IBK::Path generateFilePath(const IBK::Path & fmuBaseDirectory, const IBK::Path & fmuFilePath);

	/*! All FMUs already imported (owned by FMUManager).
		\warning External functions shall only use read-only access to m_fmus vector.
	*/
	std::vector<FMU*>	m_fmus;

};

} // namespace MASTER_SIM


#endif // MSIM_FMUMANAGER_H
