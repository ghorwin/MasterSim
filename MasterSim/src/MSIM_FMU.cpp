#include "MSIM_FMU.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

#if defined(_WIN32)

	#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
	#define NOMINMAX
#endif
	#include <windows.h>

#else  // defined(_WIN32)

	// See http://www.yolinux.com/TUTORIALS/LibraryArchives-StaticAndDynamic.html
	#include <dlfcn.h>	// shared library loading on Unix systems

#endif // defined(_WIN32)


#include <miniunz.h>

#include <IBK_Exception.h>
#include <IBK_messages.h>


namespace MASTER_SIM {

/*! Implementation class for the FMU interface, hides all platform specific details about
	loading and interfacing an FMU and keeps all platform specific includes limited to this file.
*/
class FMUPrivate {
public:

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

	/*! Imports a function address from shared library. */
	void * importFunctionAddress(const char* functionName);
	/*! Imports a function address from shared library. */
	void * importFunctionAddress(const std::string & functionName) {
		return importFunctionAddress(functionName.c_str());
	}

	/*! Import library.
		\param sharedLibraryPath Path to directory containing the shared libraries (not the path to an individual dll/so file).
	*/
	void loadLibrary(const IBK::Path & sharedLibraryDir);


#if defined(_WIN32)
	HMODULE				m_dllHandle; // fmu.dll handle
#else
	void				*m_soHandle;
#endif
}; // class FMUPrivate


// **** Class FMU Implementation ****

FMU::FMU(const IBK::Path &fmuFilePath, const IBK::Path &fmuDir) :
	m_fmuFilePath(fmuFilePath),
	m_fmuDir(fmuDir),
	m_impl(new FMUPrivate)
{
	m_resourcePath = IBK::Path("file://") / m_fmuDir / "resources";
}


FMU::~FMU() {
	delete m_impl;
}


void FMU::readModelDescription() {
	m_modelDescription.parseModelDescription(m_fmuDir / "modelDescription.xml");
}


void FMU::import(ModelDescription::FMUType typeToImport) {
	const char * const FUNC_ID = "[FMU::import]";
	if (!m_fmuDir.exists())
		throw IBK::Exception(IBK::FormatString("FMU directory '%1' does not exist.").arg(m_fmuDir), FUNC_ID);

	// compose platform specific shared library name
	IBK::Path sharedLibraryPath = m_fmuDir / FMU::binarySubDirectory();

	// first check if selected FMU-type is provided by the FMU
	if (!(m_modelDescription.m_fmuType & typeToImport))
		throw IBK::Exception("Requested FMU type is not provided by the FMU.", FUNC_ID);

	// append model identifier for selected model
	switch (typeToImport) {
		case ModelDescription::ME_v1 : ; // same as for CS_v1
		case ModelDescription::CS_v1 : sharedLibraryPath /= m_modelDescription.m_modelIdentifier; break;
		case ModelDescription::ME_v2 : sharedLibraryPath /= m_modelDescription.m_meV2ModelIdentifier; break;
		case ModelDescription::CS_v2 : sharedLibraryPath /= m_modelDescription.m_csV2ModelIdentifier; break;
		default :
			throw IBK::Exception("Invalid selection of model type (can only import a single FMU at a time).", FUNC_ID);
	}

	try {
		// load library
		m_impl->loadLibrary(sharedLibraryPath);

		if ((typeToImport & ModelDescription::ME_v1) || (typeToImport & ModelDescription::CS_v1)) {
			importFMIv1Functions();
		}
		else {
			importFMIv2Functions();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Error importing library/function symbols.", FUNC_ID);
	}

	IBK::IBK_Message("Shared library imported successfully.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
}


void FMU::importFMIv1Functions() {
	try {
		std::string modelPrefix = m_modelDescription.m_modelName + "_";

		m_fmi1Functions.getReal						= reinterpret_cast<fmiGetRealTYPE*>(m_impl->importFunctionAddress(modelPrefix+"fmiGetReal"));
		m_fmi1Functions.getInteger					= reinterpret_cast<fmiGetIntegerTYPE*>(m_impl->importFunctionAddress(modelPrefix+"fmiGetInteger"));
		m_fmi1Functions.getBoolean					= reinterpret_cast<fmiGetBooleanTYPE*>(m_impl->importFunctionAddress(modelPrefix+"fmiGetBoolean"));
		m_fmi1Functions.getString					= reinterpret_cast<fmiGetStringTYPE*>(m_impl->importFunctionAddress(modelPrefix+"fmiGetString"));
		m_fmi1Functions.setReal						= reinterpret_cast<fmiSetRealTYPE*>(m_impl->importFunctionAddress(modelPrefix+"fmiSetReal"));
		m_fmi1Functions.setInteger					= reinterpret_cast<fmiSetIntegerTYPE*>(m_impl->importFunctionAddress(modelPrefix+"fmiSetInteger"));
		m_fmi1Functions.setBoolean					= reinterpret_cast<fmiSetBooleanTYPE*>(m_impl->importFunctionAddress(modelPrefix+"fmiSetBoolean"));
		m_fmi1Functions.setString					= reinterpret_cast<fmiSetStringTYPE*>(m_impl->importFunctionAddress(modelPrefix+"fmiSetString"));

		m_fmi1Functions.instantiateSlave			= reinterpret_cast<fmiInstantiateSlaveTYPE*>(m_impl->importFunctionAddress(modelPrefix+"fmiInstantiateSlave"));
		m_fmi1Functions.freeSlaveInstance			= reinterpret_cast<fmiFreeSlaveInstanceTYPE*>(m_impl->importFunctionAddress(modelPrefix+"fmiFreeSlaveInstance"));
		m_fmi1Functions.doStep						= reinterpret_cast<fmiDoStepTYPE*>(m_impl->importFunctionAddress(modelPrefix+"fmiDoStep"));
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Error importing FMI version 1 functions from shared library.", "[FMU::importFMIv1Functions]");
	}
}


void FMU::importFMIv2Functions() {
	try {
		/***************************************************
		Common Functions
		****************************************************/
		m_fmi2Functions.getTypesPlatform			= reinterpret_cast<fmi2GetTypesPlatformTYPE*>(m_impl->importFunctionAddress("fmi2GetTypesPlatform"));
		m_fmi2Functions.getVersion					= reinterpret_cast<fmi2GetVersionTYPE*>(m_impl->importFunctionAddress("fmi2GetVersion"));
		m_fmi2Functions.setDebugLogging				= reinterpret_cast<fmi2SetDebugLoggingTYPE*>(m_impl->importFunctionAddress("fmi2SetDebugLogging"));
		m_fmi2Functions.instantiate					= reinterpret_cast<fmi2InstantiateTYPE*>(m_impl->importFunctionAddress("fmi2Instantiate"));
		m_fmi2Functions.freeInstance				= reinterpret_cast<fmi2FreeInstanceTYPE*>(m_impl->importFunctionAddress("fmi2FreeInstance"));
		m_fmi2Functions.setupExperiment				= reinterpret_cast<fmi2SetupExperimentTYPE*>(m_impl->importFunctionAddress("fmi2SetupExperiment"));
		m_fmi2Functions.enterInitializationMode		= reinterpret_cast<fmi2EnterInitializationModeTYPE*>(m_impl->importFunctionAddress("fmi2EnterInitializationMode"));
		m_fmi2Functions.exitInitializationMode		= reinterpret_cast<fmi2ExitInitializationModeTYPE*>(m_impl->importFunctionAddress("fmi2ExitInitializationMode"));
		m_fmi2Functions.terminate					= reinterpret_cast<fmi2TerminateTYPE*>(m_impl->importFunctionAddress("fmi2Terminate"));
		m_fmi2Functions.reset						= reinterpret_cast<fmi2ResetTYPE*>(m_impl->importFunctionAddress("fmi2Reset"));
		m_fmi2Functions.getReal						= reinterpret_cast<fmi2GetRealTYPE*>(m_impl->importFunctionAddress("fmi2GetReal"));
		m_fmi2Functions.getInteger					= reinterpret_cast<fmi2GetIntegerTYPE*>(m_impl->importFunctionAddress("fmi2GetInteger"));
		m_fmi2Functions.getBoolean					= reinterpret_cast<fmi2GetBooleanTYPE*>(m_impl->importFunctionAddress("fmi2GetBoolean"));
		m_fmi2Functions.getString					= reinterpret_cast<fmi2GetStringTYPE*>(m_impl->importFunctionAddress("fmi2GetString"));
		m_fmi2Functions.setReal						= reinterpret_cast<fmi2SetRealTYPE*>(m_impl->importFunctionAddress("fmi2SetReal"));
		m_fmi2Functions.setInteger					= reinterpret_cast<fmi2SetIntegerTYPE*>(m_impl->importFunctionAddress("fmi2SetInteger"));
		m_fmi2Functions.setBoolean					= reinterpret_cast<fmi2SetBooleanTYPE*>(m_impl->importFunctionAddress("fmi2SetBoolean"));
		m_fmi2Functions.setString					= reinterpret_cast<fmi2SetStringTYPE*>(m_impl->importFunctionAddress("fmi2SetString"));
		if (m_modelDescription.m_canGetAndSetFMUstate) {
			m_fmi2Functions.getFMUstate					= reinterpret_cast<fmi2GetFMUstateTYPE*>(m_impl->importFunctionAddress("fmi2GetFMUstate"));
			m_fmi2Functions.setFMUstate					= reinterpret_cast<fmi2SetFMUstateTYPE*>(m_impl->importFunctionAddress("fmi2SetFMUstate"));
			m_fmi2Functions.freeFMUstate				= reinterpret_cast<fmi2FreeFMUstateTYPE*>(m_impl->importFunctionAddress("fmi2FreeFMUstate"));
		}
		if (m_modelDescription.m_canSerializeFMUstate) {
			m_fmi2Functions.serializedFMUstateSize		= reinterpret_cast<fmi2SerializedFMUstateSizeTYPE*>(m_impl->importFunctionAddress("fmi2SerializedFMUstateSize"));
			m_fmi2Functions.serializeFMUstate			= reinterpret_cast<fmi2SerializeFMUstateTYPE*>(m_impl->importFunctionAddress("fmi2SerializeFMUstate"));
			m_fmi2Functions.deSerializeFMUstate			= reinterpret_cast<fmi2DeSerializeFMUstateTYPE*>(m_impl->importFunctionAddress("fmi2DeSerializeFMUstate"));
		}
		if (m_modelDescription.m_providesDirectionalDerivative) {
			m_fmi2Functions.getDirectionalDerivative	= reinterpret_cast<fmi2GetDirectionalDerivativeTYPE*>(m_impl->importFunctionAddress("fmi2GetDirectionalDerivative"));
		}
		/***************************************************
		Functions for FMI2 for Co-Simulation
		****************************************************/
		m_fmi2Functions.setRealInputDerivatives		= reinterpret_cast<fmi2SetRealInputDerivativesTYPE*>(m_impl->importFunctionAddress("fmi2SetRealInputDerivatives"));
		m_fmi2Functions.getRealOutputDerivatives	= reinterpret_cast<fmi2GetRealOutputDerivativesTYPE*>(m_impl->importFunctionAddress("fmi2GetRealOutputDerivatives"));
		m_fmi2Functions.doStep						= reinterpret_cast<fmi2DoStepTYPE*>(m_impl->importFunctionAddress("fmi2DoStep"));
		m_fmi2Functions.cancelStep					= reinterpret_cast<fmi2CancelStepTYPE*>(m_impl->importFunctionAddress("fmi2CancelStep"));
		m_fmi2Functions.getStatus					= reinterpret_cast<fmi2GetStatusTYPE*>(m_impl->importFunctionAddress("fmi2GetStatus"));
		m_fmi2Functions.getRealStatus				= reinterpret_cast<fmi2GetRealStatusTYPE*>(m_impl->importFunctionAddress("fmi2GetRealStatus"));
		m_fmi2Functions.getIntegerStatus			= reinterpret_cast<fmi2GetIntegerStatusTYPE*>(m_impl->importFunctionAddress("fmi2GetIntegerStatus"));
		m_fmi2Functions.getBooleanStatus			= reinterpret_cast<fmi2GetBooleanStatusTYPE*>(m_impl->importFunctionAddress("fmi2GetBooleanStatus"));
		m_fmi2Functions.getStringStatus				= reinterpret_cast<fmi2GetStringStatusTYPE*>(m_impl->importFunctionAddress("fmi2GetStringStatus"));
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
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Error importing FMI version 2 functions from shared library.", "[FMU::importFMIv2Functions]");
	}
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



// ------------------------------------------------------------------------
// *** Class FMUPrivate Implementation ***
// ------------------------------------------------------------------------


// Windows-specific implementation
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


FMUPrivate::~FMUPrivate() {
	if (m_dllHandle != 0)
		FreeLibrary( m_dllHandle );
}

void * FMUPrivate::importFunctionAddress(const char* functionName ) {
	void * ptr = reinterpret_cast<void*>( GetProcAddress( m_dllHandle, functionName ) );
	if (ptr == NULL)
		throw IBK::Exception( IBK::FormatString("Cannot import function '%1' from shared/dynamic library").arg(functionName), "[FMUPrivate::importFunctionAddress]");
	return ptr;
}

void FMUPrivate::loadLibrary(const IBK::Path & sharedLibraryDir) {
	const char * const FUNC_ID = "[FMUPrivate::loadLibrary]";
	IBK::Path sharedLibraryPath = sharedLibraryDir;
	sharedLibraryPath.addExtension(".dll");
	IBK::IBK_Message(IBK::FormatString("Loading DLL '%1'.\n").arg(sharedLibraryPath), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	if (!sharedLibraryPath.exists())
		throw IBK::Exception(IBK::FormatString("DLL '%1' does not exist.").arg(sharedLibraryPath), FUNC_ID);
	// use wide-char version of LoadLibrary
	std::wstring dllPath = sharedLibraryPath.wstrOS();
	m_dllHandle = LoadLibraryExW( dllPath.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);

	if ( m_dllHandle == 0 )
		throw IBK::Exception(IBK::FormatString("%1\nCannot load DLL '%2' (maybe missing dependencies).")
							 .arg(GetLastErrorStdStr()).arg(sharedLibraryPath), FUNC_ID);
}

#else // _WIN32

// Linux/Mac implementation

FMUPrivate::~FMUPrivate() {
	if (m_soHandle != NULL) {
		dlclose( m_soHandle );
	}
}


void * FMUPrivate::importFunctionAddress(const char* functionName) {
	void * ptr = dlsym( m_soHandle, functionName );
	if (ptr == NULL) {
		throw IBK::Exception( IBK::FormatString("Cannot import function '%1' from shared/dynamic library")
							  .arg(std::string(functionName)), "[FMUPrivate::importFunctionAddress]");
	}
	return ptr;
}


void FMUPrivate::loadLibrary(const IBK::Path & sharedLibraryDir) {
	const char * const FUNC_ID = "[FMUPrivate::loadLibrary]";
	IBK::Path sharedLibraryPath = sharedLibraryDir;
	sharedLibraryPath.addExtension(".so");
	IBK::IBK_Message(IBK::FormatString("Loading shared library '%1'.\n").arg(sharedLibraryPath), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	if (!sharedLibraryPath.exists())
		throw IBK::Exception(IBK::FormatString("Shared library '%1' does not exist.").arg(sharedLibraryPath), FUNC_ID);

	/// \bug On Unix/Linux system the absolute file path of a shared library files may be different but, due to symlinks,
	///		 point to the same file. In this case, the library will only be loaded once and two instances of FMU
	///		 will hold the same so-handle. When the destructor of the first FMU is called the library gets released. But
	///		 when dlclose() is called again with the same pointer, an access violation/segfault occurs.
	///		 There should be a sanity check here that whenever a handle is returned that previously had been returned already,
	///		 the import should fail.
	m_soHandle = dlopen( sharedLibraryPath.c_str(), RTLD_NOW );

	if (m_soHandle == NULL)
		throw IBK::Exception(IBK::FormatString("%1\nCannot load shared library '%2' (maybe missing dependencies).")
							 .arg(dlerror()).arg(sharedLibraryPath), FUNC_ID);
}

#endif // _WIN32

// ------------------------------------------------------------------------


} // namespace MASTER_SIM
