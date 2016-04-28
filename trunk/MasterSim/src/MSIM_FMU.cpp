#include "MSIM_FMU.h"

#include <cstdlib>
#include <iostream>

extern "C" {

#include <dlfcn.h>

}

#include <IBK_bitfield.h>

// shared library loading on Unix systems
#if defined(_WIN32)

#if defined(__MINGW32__)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#elif defined(_MSC_VER) // Definitions specific for MS Visual Studio (Visual C/C++).

#pragma warning( disable : 4251 ) /// \FIXME Is this really a good idea? What about the solutions suggested in http://www.unknownroad.com/rtfm/VisualStudio/warningC4251.html ???
#pragma warning( disable : 4482 ) // This is a warning about scoping of enums. It is valid C++11 syntax, though.
#pragma message( "ATTENTION: Warnings 4251 and 4482 have been disabled." )

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <errno.h>

#else
	#error Windows compiler includes needs to be configured here
#endif

#else  // defined(_WIN32)

	/* See http://www.yolinux.com/TUTORIALS/LibraryArchives-StaticAndDynamic.html */
	#include <dlfcn.h>
#endif // defined(_WIN32)

#include <miniunz.h>

#include <IBK_Exception.h>
#include <IBK_messages.h>

#include "fmi/fmi2Functions.h"


namespace MASTER_SIM {

#if defined(_WIN32)
	std::string GetLastErrorStdStr();
#endif //

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
#if defined(_WIN32)
		m_dllHandle(0)
#else
		m_soHandle(NULL)
#endif
	{
	}

	/*! Destructor, cleans up memory. */
	~FMUPrivate();

	/*! This imports the function pointers from the DLL.
		\param modelIdentifier ID name of model (used to compose file name of shared library)
		\param fmuDir Directory where FMU archive was extracted in.
	*/
	void import(ModelDescription::FMUType, const ModelDescription & modelIdentifier, const IBK::Path & fmuDir);

#if defined(_WIN32)
	void * importFunctionAddress(const char* functionName ) {
		void * ptr = reinterpret_cast<void*>( GetProcAddress( m_dllHandle, functionName ) );
		if (ptr == NULL)
			throw IBK::Exception( IBK::FormatString("Cannot import function '%1' from shared/dynamic library").arg(functionName), "[FMUPrivate::importFunctionAddress]");
		return ptr;
	}
#else
	void * importFunctionAddress(const char* functionName ) {
		void * ptr = dlsym( m_soHandle, functionName );
		if (ptr == NULL) {
			std::cout << dlerror() << std::endl;
//			throw IBK::Exception( IBK::FormatString("Cannot import function '%1' from shared/dynamic library").arg(std::string(functionName)), "[FMUPrivate::importFunctionAddress]");
		}
		return ptr;
	}
#endif

	/*! Function pointers to all functions provided by FMI v2. */
	FMI2FunctionSet		m_fmi2Functions;

#if defined(_WIN32)
	HMODULE				m_dllHandle; // fmu.dll handle
#else
	void				*m_soHandle;
#endif
};


FMUPrivate::~FMUPrivate() {
#if defined(_WIN32)
	if (m_dllHandle != 0)
		FreeLibrary( m_dllHandle );
#else
	if (m_soHandle != NULL)
		dlclose( m_soHandle );
#endif
}

// ------------------------------------------------------------------------

// **** Class FMU Implementation ****


FMU::FMU(const IBK::Path &fmuFilePath, const IBK::Path &fmuDir) :
	m_fmuFilePath(fmuFilePath),
	m_fmuDir(fmuDir),
	m_impl(new FMUPrivate)
{
}


FMU::~FMU() {
	delete m_impl;
}


void FMU::readModelDescription() {
	m_modelDescription.parseModelDescription(m_fmuDir / "modelDescription.xml");
}


void FMU::import(ModelDescription::FMUType fmu2import) {
	const char * const FUNC_ID = "[FMU::import]";
	if (!m_fmuDir.exists())
		throw IBK::Exception(IBK::FormatString("FMU directory '%1' does not exist.").arg(m_fmuDir), FUNC_ID);

	// compose platform specific shared library name
	IBK::Path sharedLibraryPath = m_fmuDir / FMU::binarySubDirectory();

	// first check if selected FMU-type is provided by the FMU
	if (!(m_modelDescription.m_fmuType & fmu2import))
		throw IBK::Exception("Requested FMU type is not provided by the FMU.", FUNC_ID);

	// append model identifier for selected model
	switch (fmu2import) {
		case ModelDescription::ME_v1 : ; // same as for CS_v1
		case ModelDescription::CS_v1 : sharedLibraryPath /= m_modelDescription.m_modelIdentifier; break;
		case ModelDescription::ME_v2 : sharedLibraryPath /= m_modelDescription.m_meV2ModelIdentifier; break;
		case ModelDescription::CS_v2 : sharedLibraryPath /= m_modelDescription.m_csV2ModelIdentifier; break;
		default :
			throw IBK::Exception("Invalid selection of model type (can only import a single FMU at a time).", FUNC_ID);
	}

	// load DLL
#if defined(_WIN32)
	sharedLibraryPath.addExtension(".dll");
	if (!sharedLibraryPath.exists())
		throw IBK::Exception(IBK::FormatString("DLL '%1' does not exist.").arg(sharedLibraryPath), FUNC_ID);
	// use wide-char version of LoadLibrary
	std::wstring dllPath = sharedLibraryPath.wstrOS();
	m_impl->m_dllHandle = LoadLibraryExW( dllPath.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);

	if ( m_impl->m_dllHandle == 0 ) {
		throw IBK::Exception(IBK::FormatString("%1\nCannot load DLL '%2'")
							 .arg(GetLastErrorStdStr()).arg(sharedLibraryPath), FUNC_ID);
	}
#else
	sharedLibraryPath.addExtension(".so");
	if (!sharedLibraryPath.exists())
		throw IBK::Exception(IBK::FormatString("Shared library '%1' does not exist.").arg(sharedLibraryPath), FUNC_ID);

	m_impl->m_soHandle = dlopen( sharedLibraryPath.c_str(), RTLD_LAZY );

	if (m_impl->m_soHandle == NULL) {
		throw IBK::Exception(IBK::FormatString("%1\nCannot load shared library from FMU '%2'")
							 .arg(dlerror()).arg(m_fmuDir), FUNC_ID);
	}

	if ((fmu2import & ModelDescription::ME_v1) || (fmu2import & ModelDescription::CS_v1)) {

	}
	else {
		importFMIv2Functions();
	}

#endif
	IBK::IBK_Message("Shared library imported successfully.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
}


void FMU::importFMIv2Functions() {
	/***************************************************
	Common Functions
	****************************************************/
	m_impl->m_fmi2Functions.getTypesPlatform = reinterpret_cast<fmi2GetTypesPlatformTYPE*>(m_impl->importFunctionAddress("fmi2GetTypesPlatform"));
	m_impl->m_fmi2Functions.getVersion = reinterpret_cast<fmi2GetVersionTYPE*>(m_impl->importFunctionAddress("fmi2GetVersions"));
//	fmi2SetDebugLoggingTYPE          *setDebugLogging;
//	fmi2InstantiateTYPE              *instantiate;
//	fmi2FreeInstanceTYPE             *freeInstance;
//	fmi2SetupExperimentTYPE          *setupExperiment;
//	fmi2EnterInitializationModeTYPE  *enterInitializationMode;
//	fmi2ExitInitializationModeTYPE   *exitInitializationMode;
//	fmi2TerminateTYPE                *terminate;
//	fmi2ResetTYPE                    *reset;
//	fmi2GetRealTYPE                  *getReal;
//	fmi2GetIntegerTYPE               *getInteger;
//	fmi2GetBooleanTYPE               *getBoolean;
//	fmi2GetStringTYPE                *getString;
//	fmi2SetRealTYPE                  *setReal;
//	fmi2SetIntegerTYPE               *setInteger;
//	fmi2SetBooleanTYPE               *setBoolean;
//	fmi2SetStringTYPE                *setString;
//	fmi2GetFMUstateTYPE              *getFMUstate;
//	fmi2SetFMUstateTYPE              *setFMUstate;
//	fmi2FreeFMUstateTYPE             *freeFMUstate;
//	fmi2SerializedFMUstateSizeTYPE   *serializedFMUstateSize;
//	fmi2SerializeFMUstateTYPE        *serializeFMUstate;
//	fmi2DeSerializeFMUstateTYPE      *deSerializeFMUstate;
//	fmi2GetDirectionalDerivativeTYPE *getDirectionalDerivative;
//	/***************************************************
//	Functions for FMI2 for Co-Simulation
//	****************************************************/
//	fmi2SetRealInputDerivativesTYPE  *setRealInputDerivatives;
//	fmi2GetRealOutputDerivativesTYPE *getRealOutputDerivatives;
//	fmi2DoStepTYPE                   *doStep;
//	fmi2CancelStepTYPE               *cancelStep;
//	fmi2GetStatusTYPE                *getStatus;
//	fmi2GetRealStatusTYPE            *getRealStatus;
//	fmi2GetIntegerStatusTYPE         *getIntegerStatus;
//	fmi2GetBooleanStatusTYPE         *getBooleanStatus;
//	fmi2GetStringStatusTYPE          *getStringStatus;
//	/***************************************************
//	Functions for FMI2 for Model Exchange
//	****************************************************/
//	fmi2EnterEventModeTYPE                *enterEventMode;
//	fmi2NewDiscreteStatesTYPE             *newDiscreteStates;
//	fmi2EnterContinuousTimeModeTYPE       *enterContinuousTimeMode;
//	fmi2CompletedIntegratorStepTYPE       *completedIntegratorStep;
//	fmi2SetTimeTYPE                       *setTime;
//	fmi2SetContinuousStatesTYPE           *setContinuousStates;
//	fmi2GetDerivativesTYPE                *getDerivatives;
//	fmi2GetEventIndicatorsTYPE            *getEventIndicators;
//	fmi2GetContinuousStatesTYPE           *getContinuousStates;
//	fmi2GetNominalsOfContinuousStatesTYPE *getNominalsOfContinuousStates;


//	m_impl->m_fmi2Functions.instantiateSlave =
//		reinterpret_cast<fInstantiateSlave>( importFunctionAddress("fmiInstantiateSlave" ) );
//	m_impl->m_fmi2Functions.initializeSlave=
//		reinterpret_cast<fInitializeSlave>( importFunctionAddress("fmiInitializeSlave" ) );
//	m_impl->m_fmi2Functions.terminateSlave =
//		reinterpret_cast<fTerminateSlave>( importFunctionAddress("fmiTerminateSlave" ) );
//	m_impl->m_fmi2Functions.resetSlave =
//		reinterpret_cast<fResetSlave>( importFunctionAddress("fmiResetSlave" ) );
//	m_impl->m_fmi2Functions.freeSlaveInstance=
//		reinterpret_cast<fFreeSlaveInstance>( importFunctionAddress("fmiFreeSlaveInstance" ) );
//	m_impl->m_fmi2Functions.cancelStep =
//		reinterpret_cast<fCancelStep>( importFunctionAddress("fmiCancelStep" ) );
//	m_impl->m_fmi2Functions.doStep =
//		reinterpret_cast<fDoStep>( importFunctionAddress("fmiDoStep" ) );
//	m_impl->m_fmi2Functions.getStatus=
//		reinterpret_cast<fGetStatus>( importFunctionAddress("fmiGetStatus" ) );
//	m_impl->m_fmi2Functions.getRealStatus=
//		reinterpret_cast<fGetRealStatus>( importFunctionAddress("fmiGetRealStatus" ) );
//	m_impl->m_fmi2Functions.getIntegerStatus =
//		reinterpret_cast<fGetIntegerStatus>( importFunctionAddress("fmiGetIntegerStatus" ) );
//	m_impl->m_fmi2Functions.getBooleanStatus =
//		reinterpret_cast<fGetBooleanStatus>( importFunctionAddress("fmiGetBooleanStatus" ) );
//	m_impl->m_fmi2Functions.getStringStatus=
//		reinterpret_cast<fGetStringStatus>( importFunctionAddress("fmiGetStringStatus" ) );
//	m_impl->m_fmi2Functions.getVersion =
//		reinterpret_cast<fGetVersion>( importFunctionAddress("fmiGetVersion" ) );
//	m_impl->m_fmi2Functions.setDebugLogging=
//		reinterpret_cast<fSetDebugLogging>( importFunctionAddress("fmiSetDebugLogging" ) );
//	m_impl->m_fmi2Functions.setReal=
//		reinterpret_cast<fSetReal>( importFunctionAddress("fmiSetReal" ) );
//	m_impl->m_fmi2Functions.setInteger =
//		reinterpret_cast<fSetInteger>( importFunctionAddress("fmiSetInteger" ) );
//	m_impl->m_fmi2Functions.setBoolean =
//		reinterpret_cast<fSetBoolean>( importFunctionAddress("fmiSetBoolean" ) );
//	m_impl->m_fmi2Functions.setString=
//		reinterpret_cast<fSetString>( importFunctionAddress("fmiSetString" ) );
//	m_impl->m_fmi2Functions.setRealInputDerivatives=
//		reinterpret_cast<fSetRealInputDerivatives>( importFunctionAddress("fmiSetRealInputDerivatives" ) );
//	m_impl->m_fmi2Functions.getReal=
//		reinterpret_cast<fGetReal>( importFunctionAddress("fmiGetReal" ) );
//	m_impl->m_fmi2Functions.getInteger =
//		reinterpret_cast<fGetInteger>( importFunctionAddress("fmiGetInteger" ) );
//	m_impl->m_fmi2Functions.getBoolean =
//		reinterpret_cast<fGetBoolean>( importFunctionAddress("fmiGetBoolean" ) );
//	m_impl->m_fmi2Functions.getString=
//		reinterpret_cast<fGetString>( importFunctionAddress("fmiGetString" ) );
//	m_impl->m_fmi2Functions.getRealOutputDerivatives=
//		reinterpret_cast<fGetRealOutputDerivatives>( importFunctionAddress("fmiGetRealOutputDerivatives" ) );
}

// **** STATIC FUNCTIONS ****

void FMU::unzipFMU(const IBK::Path & pathToFMU, const IBK::Path & extractionPath) {
	const char * const FUNC_ID = "[FMU::unzipFMU]";
	const char *argv[6];
	argv[0]="miniunz";
	argv[1]="-x";
	argv[2]="-o";
	argv[3]=pathToFMU.c_str();
	argv[4]="-d";
	argv[5]=extractionPath.c_str();

	if (!IBK::Path::makePath(extractionPath))
		throw IBK::Exception(IBK::FormatString("Cannot create extraction path '%1'").arg(extractionPath), FUNC_ID);

	// Mind: miniunz changes the current working directory
	IBK::Path currentWd = IBK::Path::current();
	int res = miniunz(6, (char**)argv);
	IBK::Path::setCurrent(currentWd); // reset working directory
	if (res != 0)
		throw IBK::Exception(IBK::FormatString("Error extracting fmu '%1' into target directory '%2'")
							 .arg(pathToFMU).arg(extractionPath), "[FMUManager::unzipFMU]");
}


IBK::Path FMU::binarySubDirectory() {
#ifdef _WIN32

#ifdef _WIN64
	return IBK::Path("binaries/win64");
#else
	return IBK::Path("binaries/win32");
#endif

#elif defined(__APPLE__)
	if (sizeof(void*) == 8)
		return IBK::Path("binaries/darwin64");
	else
		return IBK::Path("binaries/darwin32");
#else
	if (sizeof(void*) == 8)
		return IBK::Path("binaries/linux64");
	else
		return IBK::Path("binaries/linux32");
#endif
}


#if defined(_WIN32)
// Create a string with last error message
// taken from http://www.codeproject.com/Tips/479880/GetLastError-as-std-string
// BSD License, thanks to Orjan Westin
std::string GetLastErrorStdStr() {
  DWORD error = GetLastError();
  if (error)
  {
	LPVOID lpMsgBuf;
	DWORD bufLen = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		error,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,
		0, NULL );
	if (bufLen)
	{
	  LPCSTR lpMsgStr = (LPCSTR)lpMsgBuf;
	  std::string result(lpMsgStr, lpMsgStr+bufLen);

	  LocalFree(lpMsgBuf);

	  return result;
	}
  }
  return std::string();
}
#endif

} // namespace MASTER_SIM
