#ifndef MSIM_FMU_H
#define MSIM_FMU_H

#include <IBK_Path.h>


#include "MSIM_ModelDescription.h"

namespace MASTER_SIM {

class FMUPrivate;

/*! Holds all data about an imported FMU. */
class FMU {
public:
	/*! Default constructor.
		\param fmuFilePath Path to FMU archive file, not needed for importing since FMU is expected to be extracted already,
			but the file path serves as unique identification of an FMU file (file path must be unique).
		\param fmuDir Path where FMU had been extracted to, must hold the modelDescription.xml file.
	*/
	FMU(const IBK::Path & fmuFilePath, const IBK::Path & fmuDir);
	/*! Destructor, releases loaded shared library. */
	~FMU();

	/*! Reads model description file.
		This function should be called before import, so that the model identifier is known (same as file name of
		shared library).
	*/
	void readModelDescription();

	/*! This function imports the FMU, loads dynamic library, imports function pointers, reads model description.
		Throws an IBK::Exception in case of any error.
	*/
	void import();


	/*! File path to FMU as referenced in project file (should be an absolute file path). */
	IBK::Path			m_fmuFilePath;
	/*! Directory where FMU was extracted to (also absolute file path). */
	IBK::Path			m_fmuDir;

	/*! Content of model description. */
	ModelDescription	m_modelDescription;


	/*! Utility function to unzip an FMU archive into an existing directory.
		This is a static function because unzipping is done in an optional step before importing the FMU.
		\param pathToFMU Holds path to FMU.
		\param extractionPath Directory to extract contents of FMU in, must exist.
	*/
	static void unzipFMU(const IBK::Path & pathToFMU, const IBK::Path & extractionPath);

	/*! Composes the shared library directory for the current platform. */
	static IBK::Path binarySubDirectory();

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
