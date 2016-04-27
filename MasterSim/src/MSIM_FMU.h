#ifndef MSIM_FMU_H
#define MSIM_FMU_H

#include <IBK_Path.h>

namespace MASTER_SIM {

class FMUPrivate;

/*! Holds all data about an imported FMU. */
class FMU {
public:
	/*! Default constructor. */
	FMU();
	/*! Destructor, releases loaded shared library. */
	~FMU();

	/*! This function imports the FMU, loads dynamic library, imports function pointers, reads model description.
		Throws an IBK::Exception in case of any error.
		\param fmuFilePath Path to FMU archive file, not needed for importing since FMU is expected to be extracted already,
			but the file path serves as unique identification of an FMU file (file path must be unique).
		\param fmuDir Path where FMU had been extracted to, must hold the modelDescription.xml file.
	*/
	void import(const IBK::Path & fmuFilePath, const IBK::Path & fmuDir);

	/*! Reads model description file. */
	void readModelDescription();

	/*! File path to FMU as referenced in project file. */
	IBK::Path	m_fmuFilePath;
	/*! Directory where FMU was extracted to. */
	IBK::Path	m_fmuDir;

	/*! Utility function to unzip an FMU archive into an existing directory.
		This is a static function because unzipping is done in an optional step before importing the FMU.
		\param pathToFMU Holds path to FMU.
		\param extractionPath Directory to extract contents of FMU in, must exist.
	*/
	static void unzipFMU(const IBK::Path & pathToFMU, const IBK::Path & extractionPath);

private:
	/*! Disable copy. */
	FMU(const FMU &);
	/*! Disable assignment operator. */
	FMU & operator=(const FMU &);

	/*! Holds the actual implementation so that details of importing are hidden to user of class. */
	FMUPrivate	*m_impl;
};

} // namespace MASTER_SIM


#endif // MSIM_FMU_H
