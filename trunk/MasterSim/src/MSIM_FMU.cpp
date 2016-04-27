#include "MSIM_FMU.h"

#include <cstdlib>

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
	void import(const std::string & modelIdentifier, const IBK::Path & fmuDir);


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

void FMUPrivate::import(const std::string & modelIdentifier, const IBK::Path & fmuDir) {
	const char * const FUNC_ID = "[FMUPrivate::import]";
	if (!fmuDir.exists())
		throw IBK::Exception(IBK::FormatString("Shared library '%1' does not exist.").arg(fmuDir), FUNC_ID);

	// compose platform specific dll name
	IBK::Path sharedLibraryPath = fmuDir / FMU::binarySubDirectory() / modelIdentifier;

	// load DLL
#if defined(_WIN32)
	sharedLibraryPath.addExtension(".dll");
#if defined(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS)
	// Used instead of LoadLibrary to include the DLL's directory in dependency
	// lookups. The flags require KB2533623 to be installed.
	m_dllHandle = LoadLibraryEx( sharedLibraryPath.wstr().c_str(), NULL,
		LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR );
#else
	// use wide-char version of LoadLibrary
	m_dllHandle = LoadLibrary( sharedLibraryPath.wstr().c_str() );
#endif
	if ( m_dllHandle != 0 ) {
		throw IBK::Exception(IBK::FormatString("%1\nCannot load DLL '%2'")
							 .arg(GetLastErrorStdStr()).arg(sharedLibraryPath), FUNC_ID);
	}
#else
	sharedLibraryPath.addExtension(".so");

	m_soHandle = dlopen( sharedLibraryPath.str().c_str(), RTLD_LAZY );

	if (m_soHandle == NULL) {
		throw IBK::Exception(IBK::FormatString("%1\nCannot load shared library from FMU '%2'")
							 .arg(dlerror()).arg(fmuDir), FUNC_ID);
	}
#endif
	IBK::IBK_Message("Shared library imported successfully.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
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


void FMU::import() {
	m_impl->import(m_modelDescription.m_csModelIdentifier, m_fmuDir);
}


// **** STATIC FUNCTIONS ****

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
