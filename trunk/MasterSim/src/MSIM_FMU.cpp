#include "MSIM_FMU.h"

#include <cstdlib>

// shared library loading on Unix systems
#if defined(_WIN32)

#else
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
#if Q_OS_WIN
		m_dllHandle(0)
#else
		m_soHandle(NULL)
#endif
	{
	}

	/*! Destructor, cleans up memory. */
	~FMUPrivate();

	/*! This imports the function pointers from the DLL. */
	void import(const IBK::Path & fmuPath);


	/*! Function pointers to all functions provided by FMI v2. */
	FMI2FunctionSet		m_fmi2Functions;

#if Q_OS_WIN
	HMODULE				m_dllHandle; // fmu.dll handle
#else
	void				*m_soHandle;
#endif
};


FMUPrivate::~FMUPrivate() {
#if defined(MINGW)
	if (m_dllHandle != 0)
		FreeLibrary( m_dllHandle );
#elif defined(_MSC_VER)
	if (m_dllHandle != 0)
		FreeLibrary( m_dllHandle );
#else
	if (m_soHandle != NULL)
		dlclose( m_soHandle );
#endif
}

void FMUPrivate::import(const IBK::Path & fmuPath) {
	const char * const FUNC_ID = "[FMUPrivate::import]";
	if (!fmuPath.exists())
		throw IBK::Exception(IBK::FormatString("Shared library '%1' does not exist.").arg(fmuPath), FUNC_ID);

	// compose platform specific dll name

	// load DLL
#if defined(MINGW) || defined(_MSC_VER)
#if defined(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS)
	/// \todo check UTF8 path rules here or use wide-char version of LoadLibrary
	// Used instead of LoadLibrary to include the DLL's directory in dependency
	// lookups. The flags require KB2533623 to be installed.
	m_dllHandle = LoadLibraryEx( fmuPath.str().c_str(), NULL,
		LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR );
#else
	m_dllHandle = LoadLibrary( fmuPath.str().c_str() );
#endif
	if ( m_dllHandle != 0 ) {
		throw IBK::Exception(IBK::FormatString("Cannot load DLL '%1': %2")
							 .arg(fmuPath).arg(GetLastErrorStdStr()), FUNC_ID);
	}
#else
	m_soHandle = dlopen( fmuPath.str().c_str(), RTLD_LAZY );

	if (m_soHandle == NULL) {
		throw IBK::Exception(IBK::FormatString("Cannot load shared library '%1': %2")
							 .arg(fmuPath).arg(dlerror()), FUNC_ID);
	}
#endif
}

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
