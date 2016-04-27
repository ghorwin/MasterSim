#include "MSIM_FMU.h"

#include <cstdlib>

#include <miniunz.h>

#include <IBK_Exception.h>
#include <IBK_messages.h>

#include "fmi/fmi2Functions.h"


namespace MASTER_SIM {

/*! Implementation class for the FMU interface, hides all details about
	loading and interfacing an FMU.
*/
class FMUPrivate {
public:
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

	FMUPrivate() :
#if Q_OS_WIN
		m_dllHandle(0)
#else
		m_soHandle(NULL)
#endif
	{
	}

	~FMUPrivate() {
		// release memory
	}

	/*! This imports the function pointers from the DLL. */
	void import(const IBK::Path & fmuPath) {

	}


	/*! Function pointers to all functions provided by FMI v2. */
	FMI2FunctionSet		m_fmi2Functions;

#if Q_OS_WIN
	HMODULE				m_dllHandle; // fmu.dll handle
#else
	void				*m_soHandle;
#endif


};

// ------------------------------------------------------------------------

FMU::FMU() : m_impl(new FMUPrivate)
{
}


FMU::~FMU() {
	delete m_impl;
}


void FMU::import(const IBK::Path &fmuFilePath, const IBK::Path &fmuDir) {
	m_fmuFilePath = fmuFilePath;
	m_fmuDir = fmuDir;
	m_impl->import(fmuDir);
}


void FMU::readModelDescription() {

}


void FMU::unzipFMU(const IBK::Path & pathToFMU, const IBK::Path & extractionPath) {
	const char * const FUNC_ID = "[FMU::unzipFMU]";
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
