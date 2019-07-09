#ifndef MSIM_FMU_H
#define MSIM_FMU_H

#include <IBK_Path.h>

#include "fmi/fmiFunctions.h"
#include "fmi/fmi2Functions.h"

#include "MSIM_ModelDescription.h"

namespace MASTER_SIM {

class FMUPrivate;

/*! Holds all data about an imported FMU. */
class FMU {
public:
	/*! Function pointers to functions published by FMIv1 FMUs. */
	struct FMI1FunctionSet {
		/***************************************************
		Common Functions
		****************************************************/
		fmiGetRealTYPE					*getReal;
		fmiGetIntegerTYPE				*getInteger;
		fmiGetBooleanTYPE				*getBoolean;
		fmiGetStringTYPE				*getString;
		fmiSetRealTYPE					*setReal;
		fmiSetIntegerTYPE				*setInteger;
		fmiSetBooleanTYPE				*setBoolean;
		fmiSetStringTYPE				*setString;
		/***************************************************
		Functions for FMI2 for Co-Simulation
		****************************************************/
		fmiInstantiateSlaveTYPE			*instantiateSlave;
		fmiInitializeSlaveTYPE			*initializeSlave;
		fmiFreeSlaveInstanceTYPE		*freeSlaveInstance;
		fmiDoStepTYPE					*doStep;
	};

	/*! Function pointers to functions published by FMIv2 FMUs. */
	struct FMI2FunctionSet {
		/***************************************************
		Common Functions
		****************************************************/
		fmi2GetTypesPlatformTYPE         *getTypesPlatform;
		fmi2GetVersionTYPE               *getVersion;
		fmi2SetDebugLoggingTYPE          *setDebugLogging;
		fmi2InstantiateTYPE              *instantiate;
		fmi2FreeInstanceTYPE             *freeInstance;
		fmi2SetupExperimentTYPE          *setupExperiment;
		fmi2EnterInitializationModeTYPE  *enterInitializationMode;
		fmi2ExitInitializationModeTYPE   *exitInitializationMode;
		fmi2TerminateTYPE                *terminate;
		fmi2ResetTYPE                    *reset;
		fmi2GetRealTYPE                  *getReal;
		fmi2GetIntegerTYPE               *getInteger;
		fmi2GetBooleanTYPE               *getBoolean;
		fmi2GetStringTYPE                *getString;
		fmi2SetRealTYPE                  *setReal;
		fmi2SetIntegerTYPE               *setInteger;
		fmi2SetBooleanTYPE               *setBoolean;
		fmi2SetStringTYPE                *setString;
		fmi2GetFMUstateTYPE              *getFMUstate;
		fmi2SetFMUstateTYPE              *setFMUstate;
		fmi2FreeFMUstateTYPE             *freeFMUstate;
		fmi2SerializedFMUstateSizeTYPE   *serializedFMUstateSize;
		fmi2SerializeFMUstateTYPE        *serializeFMUstate;
		fmi2DeSerializeFMUstateTYPE      *deSerializeFMUstate;
		fmi2GetDirectionalDerivativeTYPE *getDirectionalDerivative;
		/***************************************************
		Functions for FMI2 for Co-Simulation
		****************************************************/
		fmi2SetRealInputDerivativesTYPE  *setRealInputDerivatives;
		fmi2GetRealOutputDerivativesTYPE *getRealOutputDerivatives;
		fmi2DoStepTYPE                   *doStep;
		fmi2CancelStepTYPE               *cancelStep;
		fmi2GetStatusTYPE                *getStatus;
		fmi2GetRealStatusTYPE            *getRealStatus;
		fmi2GetIntegerStatusTYPE         *getIntegerStatus;
		fmi2GetBooleanStatusTYPE         *getBooleanStatus;
		fmi2GetStringStatusTYPE          *getStringStatus;
		/***************************************************
		Functions for FMI2 for Model Exchange
		****************************************************/
		fmi2EnterEventModeTYPE                *enterEventMode;
		fmi2NewDiscreteStatesTYPE             *newDiscreteStates;
		fmi2EnterContinuousTimeModeTYPE       *enterContinuousTimeMode;
		fmi2CompletedIntegratorStepTYPE       *completedIntegratorStep;
		fmi2SetTimeTYPE                       *setTime;
		fmi2SetContinuousStatesTYPE           *setContinuousStates;
		fmi2GetDerivativesTYPE                *getDerivatives;
		fmi2GetEventIndicatorsTYPE            *getEventIndicators;
		fmi2GetContinuousStatesTYPE           *getContinuousStates;
		fmi2GetNominalsOfContinuousStatesTYPE *getNominalsOfContinuousStates;
	};


	/*! Default constructor.
		\param fmuFilePath Path to FMU archive file, not needed for importing since FMU is expected to be extracted already,
			but the file path serves as unique identification of an FMU file (file path must be unique).
		\param fmuDir Path where FMU had been extracted to, must hold the modelDescription.xml file.
	*/
	FMU(const IBK::Path & fmuFilePath, const IBK::Path & fmuDir);

	/*! Destructor, releases loaded shared library. */
	~FMU();

	/*! Reads model description file.
		This function should be called before import, so that the model identifier is known
		(need to compose file name of shared library).
	*/
	void readModelDescription();

	/*! Populates the vectors m_xxxValueRefsOutput.
		This function is called during simulation runs (not needed for user interface).
	*/
	void collectOutputVariableReferences(bool includeInternalVariables);

	/*! This function imports the FMU, loads dynamic library, imports function pointers, reads model description.
		Throws an IBK::Exception in case of any error.
		\param typeToImport Selection of one of the interface types to import.
	*/
	void import(ModelDescription::FMUType typeToImport);

	/*! File path to FMU as referenced in project file (should be an absolute file path). */
	const IBK::Path & fmuFilePath() const { return m_fmuFilePath;}
	/*! Directory where FMU was extracted to (as absolute file path). */
	const IBK::Path & fmuDir() const { return m_fmuDir;}

	/*! Returns a persistant pointer to the resource directory of the extracted FMU. */
	const char * resourcePath() const { return m_resourcePath.c_str(); }

	/*! Returns the index of the references output variable in the corresponding vector with output quantities. */
	unsigned int localOutputIndex(FMIVariable::VarType t, unsigned int valueReference) const;

	/*! Content of model description. */
	ModelDescription	m_modelDescription;

	/*! Function pointers to all functions provided by FMI v1.
		Access these functions only after successful import of matching FMU.
	*/
	FMI1FunctionSet		m_fmi1Functions;
	/*! Function pointers to all functions provided by FMI v2.
		Access these functions only after successful import of matching FMU.
	*/
	FMI2FunctionSet		m_fmi2Functions;

	// Value references for output variables.

	std::vector<unsigned int>	m_boolValueRefsOutput;
	std::vector<unsigned int>	m_intValueRefsOutput;
	std::vector<unsigned int>	m_stringValueRefsOutput;
	std::vector<unsigned int>	m_doubleValueRefsOutput;

	/*! Utility function to unzip an FMU archive into an existing directory.
		This is a static function because unzipping is done in an optional step before importing the FMU.
		\param pathToFMU Holds path to FMU.
		\param extractionPath Directory to extract contents of FMU in, must exist.
	*/
	static void unzipFMU(const IBK::Path & pathToFMU, const IBK::Path & extractionPath);

	/*! Composes the shared library directory for the current platform. */
	static IBK::Path binarySubDirectory();


private:
	/*! Imports functions for Version 1.0 FMU. */
	void importFMIv1Functions();
	/*! Imports functions for Version 2.0 FMU. */
	void importFMIv2Functions();

	/*! Disable copy. */
	FMU(const FMU &);
	/*! Disable assignment operator. */
	FMU & operator=(const FMU &);

	/*! File path to FMU as referenced in project file (should be an absolute file path). */
	IBK::Path			m_fmuFilePath;
	/*! Directory where FMU was extracted to (as absolute file path). */
	IBK::Path			m_fmuDir;
	/*! Resource path within FMU directory.
		\note This variable is needed as persistant string
	*/
	std::string			m_resourcePath;

	/*! Holds the actual implementation and data so that details of importing are hidden to user of class. */
	FMUPrivate	*m_impl;
};

} // namespace MASTER_SIM


#endif // MSIM_FMU_H
