#include "MSIM_FMU.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <algorithm>

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
#include <IBK_FormatString.h>
#include <IBK_assert.h>

// Platform-specific file extension of the shared library inside an FMU
#if defined(_WIN32)
static const char * const SHARED_LIBRARY_EXTENSION = ".dll";
#elif defined(__APPLE__)
static const char * const SHARED_LIBRARY_EXTENSION = ".dylib";
#else
static const char * const SHARED_LIBRARY_EXTENSION = ".so";
#endif

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
		m_soHandle(nullptr)
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
}


FMU::~FMU() {
	delete m_impl;
}


void FMU::readModelDescription() {
	m_modelDescription.read(m_fmuDir / "modelDescription.xml");

	// generate resource path
	if (m_modelDescription.m_fmuType == ModelDescription::CS_v1 ||
		m_modelDescription.m_fmuType == ModelDescription::ME_v1)
	{
		m_resourcePath = m_fmuDir.str();
	}
	else {
		m_resourcePath = (m_fmuDir / "resources").str();
	}
#ifdef _WIN32
	// this is actually wrong, the URI prefix should be "file://" so that
	// network paths become file:///path/to/network/share also on windows
	m_resourcePath = "file:///" + m_resourcePath;
#else
	m_resourcePath = "file://" + m_resourcePath;
#endif
}


void FMU::addIndexIfNotInList(std::vector<unsigned int> & valueRefList, const std::string & varName, FMIVariable::VarType varType, unsigned int valueReference, const std::string & unit) {
	if (std::find(valueRefList.begin(), valueRefList.end(), valueReference) == valueRefList.end()) {
		valueRefList.push_back(valueReference);
		if (varType == FMIVariable::VT_DOUBLE) {
			IBK::IBK_Message(IBK::FormatString("Variable '%1' (%2) [%3], valueRef = %4\n")
							 .arg(varName).arg(FMIVariable::varType2String(varType)).arg(unit).arg(valueReference),
								   IBK::MSG_PROGRESS, "[FMU::addIfNotInList]", IBK::VL_STANDARD);
		}
		else {
			IBK::IBK_Message(IBK::FormatString("Variable '%1' (%2), valueRef = %3\n")
							 .arg(varName).arg(FMIVariable::varType2String(varType)).arg(valueReference),
								   IBK::MSG_PROGRESS, "[FMU::addIfNotInList]", IBK::VL_STANDARD);
		}
	}
	else {
		// look up variable with given variable reference
		const FMIVariable & var = m_modelDescription.variableByRef(varType, valueReference);
		IBK::IBK_Message(IBK::FormatString("Variable '%1' [%2] has valueReference %3 which is already selected for output (variable '%4').")
						 .arg(varName).arg(FMIVariable::varType2String(varType)).arg(valueReference).arg(var.m_name),
							   IBK::MSG_WARNING, "[FMU::addIfNotInList]", IBK::VL_STANDARD);
		m_synonymousVars[std::make_pair(var.m_type, valueReference)].push_back(varName);
	}
}


void FMU::collectOutputVariableReferences(bool includeInternalVariables) {
	FUNCID(FMU::collectOutputVariableReferences);
	// clear map m_synonymousVars
	m_synonymousVars.clear();
	// now collect all output variable valueReferences
	for (unsigned int i=0; i<m_modelDescription.m_variables.size(); ++i) {
		const FMIVariable & var = m_modelDescription.m_variables[i];
		if (var.m_causality == FMIVariable::C_OUTPUT) {
			switch (var.m_type) {
				case FMIVariable::VT_BOOL	: addIndexIfNotInList(m_boolValueRefsOutput, var.m_name, var.m_type,  var.m_valueReference, std::string()); break;
				case FMIVariable::VT_INT	: addIndexIfNotInList(m_intValueRefsOutput, var.m_name, var.m_type,  var.m_valueReference, std::string()); break;
				case FMIVariable::VT_DOUBLE	: addIndexIfNotInList(m_doubleValueRefsOutput, var.m_name, var.m_type,  var.m_valueReference, var.m_unit); break;
				case FMIVariable::VT_STRING	: addIndexIfNotInList(m_stringValueRefsOutput, var.m_name, var.m_type,  var.m_valueReference, std::string()); break;
				default:
					IBK_ASSERT_X(false, "bad variable initialization");
			}
		}
	}
	if (includeInternalVariables) {
		IBK::IBK_Message( "\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		IBK::IBK_Message( "Outputs from internal (all) variables:\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		IBK::MessageIndentor indent; (void)indent;
		// now append internal variables
		for (unsigned int i=0; i<m_modelDescription.m_variables.size(); ++i) {
			const FMIVariable & var = m_modelDescription.m_variables[i];
			if (var.m_causality == FMIVariable::C_OUTPUT)
				continue; // skip outputs, as they have already been processed
			switch (var.m_type) {
				case FMIVariable::VT_BOOL	: addIndexIfNotInList(m_boolValueRefsOutput, var.m_name, var.m_type,  var.m_valueReference, std::string()); break;
				case FMIVariable::VT_INT	: addIndexIfNotInList(m_intValueRefsOutput, var.m_name, var.m_type,  var.m_valueReference, std::string()); break;
				case FMIVariable::VT_DOUBLE	: addIndexIfNotInList(m_doubleValueRefsOutput, var.m_name, var.m_type,  var.m_valueReference, var.m_unit); break;
				case FMIVariable::VT_STRING	: addIndexIfNotInList(m_stringValueRefsOutput, var.m_name, var.m_type,  var.m_valueReference, std::string()); break;
				default:
					IBK_ASSERT_X(false, "bad variable initialization");
			}
		}
	}

	// there is one problem to be adressed:
	// a variable "TRes" with valueRef 20 is defined as "output" -> valueRef 20 is stoed in m_doubleValuesRefs
	// another variable "Tx", also with valuesRef 20 is defined as "internal", but earlier in the list of variables

	// when creating the output files, the captions are created by looking up the variable by valueReference -> 20 yields "Tx", so
	// the caption of the output column will be "Tx", despite the "TRes" variable being the actual output variable

	// Solution: (quick and dirty)
	// - enable write internal variables and include "all variables" -> this will create the synonym variables list which will
	//   then also include "TRes" -> post-processing may then resolve the synonymous variables

	// Solution 2: (proper)
	// - implement lookup preference when resolving variables by valueRef - first glob all variables with same value reference, then
	//   pick "output" type variables before resolving "internal" or "parameter" variables
}


void FMU::import(ModelDescription::FMUType typeToImport) {
	const char * const FUNC_ID = "[FMU::import]";
	if (!m_fmuDir.exists())
		throw IBK::Exception(IBK::FormatString("FMU directory '%1' does not exist.").arg(m_fmuDir), FUNC_ID);

	// first check if selected FMU-type is provided by the FMU
	if (!(m_modelDescription.m_fmuType & typeToImport))
		throw IBK::Exception("Requested FMU type is not provided by the FMU.", FUNC_ID);

	// model identifier = file name of the shared library (without extension)
	std::string modelIdentifier;
	switch (typeToImport) {
		case ModelDescription::ME_v1 : // same as for CS_v1
		case ModelDescription::CS_v1 : modelIdentifier = m_modelDescription.m_modelIdentifier; break;
		case ModelDescription::ME_v2 : modelIdentifier = m_modelDescription.m_meV2ModelIdentifier; break;
		case ModelDescription::CS_v2 : modelIdentifier = m_modelDescription.m_csV2ModelIdentifier; break;
	}

	// compose platform specific shared library name: use the first binary directory that
	// contains the library; if none does, use the preferred one (loadLibrary() reports the error)
	std::vector<IBK::Path> subDirs = FMU::binarySubDirectories();
	IBK::Path sharedLibraryPath = m_fmuDir / subDirs.front() / modelIdentifier;
	for (const IBK::Path & subDir : subDirs) {
		IBK::Path candidate = m_fmuDir / subDir / modelIdentifier;
		IBK::Path candidateWithExt = candidate;
		candidateWithExt.addExtension(SHARED_LIBRARY_EXTENSION);
		if (candidateWithExt.exists()) {
			sharedLibraryPath = candidate;
			break;
		}
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
}


unsigned int FMU::localOutputIndex(FMIVariable::VarType t, unsigned int valueReference) const {
	switch (t) {
		case FMIVariable::VT_BOOL : {
			// search corresponding vector for valueReference and return index
			for (unsigned int i=0; i<m_boolValueRefsOutput.size(); ++i)
				if (m_boolValueRefsOutput[i] == valueReference)
					return i;
		} break;
		case FMIVariable::VT_INT : {
			// search corresponding vector for valueReference and return index
			for (unsigned int i=0; i<m_intValueRefsOutput.size(); ++i)
				if (m_intValueRefsOutput[i] == valueReference)
					return i;
		} break;
		case FMIVariable::VT_DOUBLE : {
			// search corresponding vector for valueReference and return index
			for (unsigned int i=0; i<m_doubleValueRefsOutput.size(); ++i)
				if (m_doubleValueRefsOutput[i] == valueReference)
					return i;
		} break;
		case FMIVariable::VT_STRING : {
			// search corresponding vector for valueReference and return index
			for (unsigned int i=0; i<m_stringValueRefsOutput.size(); ++i)
				if (m_stringValueRefsOutput[i] == valueReference)
					return i;
		} break;
		case FMIVariable::NUM_VT : ; // just to remove compiler warning
	}
	throw IBK::Exception( IBK::FormatString("Variable of type '%1' with value reference %2 is not defined in this FMU.")
		.arg(FMIVariable::varType2String(t)).arg(valueReference), "[FMU::localOutputIndex]");
}


void FMU::importFMIv1Functions() {
	try {
		std::string modelPrefix = m_modelDescription.m_modelIdentifier + "_";

		m_fmi1Functions.getReal						= reinterpret_cast<fmiGetRealTYPE*>(m_impl->importFunctionAddress(modelPrefix+"fmiGetReal"));
		m_fmi1Functions.getInteger					= reinterpret_cast<fmiGetIntegerTYPE*>(m_impl->importFunctionAddress(modelPrefix+"fmiGetInteger"));
		m_fmi1Functions.getBoolean					= reinterpret_cast<fmiGetBooleanTYPE*>(m_impl->importFunctionAddress(modelPrefix+"fmiGetBoolean"));
		m_fmi1Functions.getString					= reinterpret_cast<fmiGetStringTYPE*>(m_impl->importFunctionAddress(modelPrefix+"fmiGetString"));
		m_fmi1Functions.setReal						= reinterpret_cast<fmiSetRealTYPE*>(m_impl->importFunctionAddress(modelPrefix+"fmiSetReal"));
		m_fmi1Functions.setInteger					= reinterpret_cast<fmiSetIntegerTYPE*>(m_impl->importFunctionAddress(modelPrefix+"fmiSetInteger"));
		m_fmi1Functions.setBoolean					= reinterpret_cast<fmiSetBooleanTYPE*>(m_impl->importFunctionAddress(modelPrefix+"fmiSetBoolean"));
		m_fmi1Functions.setString					= reinterpret_cast<fmiSetStringTYPE*>(m_impl->importFunctionAddress(modelPrefix+"fmiSetString"));

		m_fmi1Functions.instantiateSlave			= reinterpret_cast<fmiInstantiateSlaveTYPE*>(m_impl->importFunctionAddress(modelPrefix+"fmiInstantiateSlave"));
		m_fmi1Functions.initializeSlave				= reinterpret_cast<fmiInitializeSlaveTYPE*>(m_impl->importFunctionAddress(modelPrefix+"fmiInitializeSlave"));
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

#if defined(_WIN32)
	std::string filenameAnsi = IBK::WstringToANSI(pathToFMU.wstr(), false);
	argv[3]=filenameAnsi.c_str();
#else
	argv[3]=pathToFMU.c_str();
#endif // _WIN32

	argv[4]="-d";
#if defined(_WIN32)
	std::string extractionPathAnsi = IBK::WstringToANSI(extractionPath.wstr(), false);
	argv[5]=extractionPathAnsi.c_str();
#else
	argv[5]=extractionPath.c_str();
#endif // _WIN32

	if (!IBK::Path::makePath(extractionPath))
		throw IBK::Exception(IBK::FormatString("Cannot create extraction path '%1'").arg(extractionPath), FUNC_ID);

	// Mind: miniunz changes the current working directory
	IBK::Path currentWd = IBK::Path::current();
	int res = miniunz(6, argv);
	IBK::Path::setCurrent(currentWd); // reset working directory
	if (res != 0) {
#if defined(_WIN32)
		throw IBK::Exception(IBK::FormatString("Error extracting fmu '%1' into target directory '%2'")
							 .arg(filenameAnsi).arg(extractionPathAnsi), "[FMUManager::unzipFMU]");
#else
		throw IBK::Exception(IBK::FormatString("Error extracting fmu '%1' into target directory '%2'")
							 .arg(pathToFMU).arg(extractionPath), "[FMUManager::unzipFMU]");
#endif
	}
}


std::vector<IBK::Path> FMU::binarySubDirectories() {
	std::vector<IBK::Path> dirs;

	// FMI 1.0/2.0 platform directory (primary, since MasterSim imports FMI 1.0 and 2.0 FMUs).
	// Note: on macOS "darwin64" does not specify the CPU architecture. FMUs supporting Apple Silicon
	//       usually ship a universal (x86_64 + arm64) dylib in this directory.
#if defined(_WIN32)
#  if defined(_WIN64)
	dirs.push_back(IBK::Path("binaries/win64"));
#  else
	dirs.push_back(IBK::Path("binaries/win32"));
#  endif
#elif defined(__APPLE__)
	dirs.push_back(IBK::Path("binaries/darwin64"));
#else
	if (sizeof(void*) == 8)
		dirs.push_back(IBK::Path("binaries/linux64"));
	else
		dirs.push_back(IBK::Path("binaries/linux32"));
#endif

	// Fallback: FMI 3.0 platform tuple <arch>-<sys>, sometimes also used by FMI 2.0 exporters,
	// and the only way to provide architecture-specific binaries (e.g. aarch64-darwin).
#if defined(__aarch64__) || defined(_M_ARM64)
	const char * const arch = "aarch64";
#elif defined(__x86_64__) || defined(_M_X64)
	const char * const arch = "x86_64";
#elif defined(__i386__) || defined(_M_IX86)
	const char * const arch = "x86";
#else
	const char * const arch = nullptr;
#endif

#if defined(_WIN32)
	const char * const sys = "windows";
#elif defined(__APPLE__)
	const char * const sys = "darwin";
#else
	const char * const sys = "linux";
#endif

	if (arch != nullptr)
		dirs.push_back(IBK::Path("binaries") / (std::string(arch) + "-" + sys));

	return dirs;
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
			nullptr,
			error,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0, nullptr );
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
	/// \todo  check, if FreeLibrary() causes an access violation when the library contains OpenMP code
	if (m_dllHandle != 0)
		FreeLibrary( m_dllHandle );
}

void * FMUPrivate::importFunctionAddress(const char* functionName ) {
	void * ptr = reinterpret_cast<void*>( GetProcAddress( m_dllHandle, functionName ) );
	if (ptr == nullptr)
		throw IBK::Exception( IBK::FormatString("Cannot import function '%1' from shared/dynamic library").arg(functionName), "[FMUPrivate::importFunctionAddress]");
	return ptr;
}

void FMUPrivate::loadLibrary(const IBK::Path & sharedLibraryDir) {
	const char * const FUNC_ID = "[FMUPrivate::loadLibrary]";
	IBK::Path sharedLibraryPath = sharedLibraryDir;
	sharedLibraryPath.addExtension(SHARED_LIBRARY_EXTENSION);
	IBK::IBK_Message(IBK::FormatString("Loading DLL '%1'.\n").arg(sharedLibraryPath), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	if (!sharedLibraryPath.exists())
		throw IBK::Exception(IBK::FormatString("DLL '%1' does not exist.").arg(sharedLibraryPath), FUNC_ID);
	// use wide-char version of LoadLibrary
	std::wstring dllPath = sharedLibraryPath.wstrOS();
	m_dllHandle = LoadLibraryExW( dllPath.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);

	if ( m_dllHandle == 0 )
		throw IBK::Exception(IBK::FormatString("%1\nCannot load DLL '%2' (maybe missing dependencies).")
							 .arg(GetLastErrorStdStr()).arg(sharedLibraryPath), FUNC_ID);
}


#else // _WIN32

// Linux/Mac implementation

FMUPrivate::~FMUPrivate() {
	if (m_soHandle != nullptr) {
		dlclose( m_soHandle );
	}
}


void * FMUPrivate::importFunctionAddress(const char* functionName) {
	void * ptr = dlsym( m_soHandle, functionName );
	if (ptr == nullptr) {
		throw IBK::Exception( IBK::FormatString("Cannot import function '%1' from shared/dynamic library")
							  .arg(std::string(functionName)), "[FMUPrivate::importFunctionAddress]");
	}
	return ptr;
}


void FMUPrivate::loadLibrary(const IBK::Path & sharedLibraryDir) {
	const char * const FUNC_ID = "[FMUPrivate::loadLibrary]";
	IBK::Path sharedLibraryPath = sharedLibraryDir;
	sharedLibraryPath.addExtension(SHARED_LIBRARY_EXTENSION);
	IBK::IBK_Message(IBK::FormatString("Loading shared library '%1'.\n").arg(sharedLibraryPath), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	if (!sharedLibraryPath.exists())
		throw IBK::Exception(IBK::FormatString("Shared library '%1' does not exist.").arg(sharedLibraryPath), FUNC_ID);

	/// \bug On Unix/Linux system the absolute file path of a shared library files may be different but, due to symlinks,
	///		 point to the same file. In this case, the library will only be loaded once and two instances of FMU
	///		 will hold the same so-handle. When the destructor of the first FMU is called the library gets released. But
	///		 when dlclose() is called again with the same pointer, an access violation/segfault occurs.
	///		 There should be a sanity check here that whenever a handle is returned that previously had been returned already,
	///		 the import should fail.
#ifdef __APPLE__
	m_soHandle = dlopen( sharedLibraryPath.c_str(), RTLD_LAZY );
#else
	m_soHandle = dlopen( sharedLibraryPath.c_str(), RTLD_LAZY|RTLD_DEEPBIND );
#endif

	if (m_soHandle == nullptr) {
		std::string errMsg = dlerror(); // can only be called once
#ifdef __APPLE__
		// A process can only load dylibs matching its own architecture (Rosetta 2 translates whole
		// processes only). Give the user a hint how to run x86_64-only FMUs on Apple Silicon.
		if (errMsg.find("incompatible architecture") != std::string::npos) {
#  if defined(__aarch64__)
			errMsg += "\nThe FMU does not contain an arm64 (Apple Silicon) binary. To use x86_64-only FMUs, "
					  "run MasterSim under Rosetta 2, e.g. 'arch -x86_64 mastersim <project.msim>'.";
#  else
			errMsg += "\nThe FMU does not contain an x86_64 binary. Use an FMU compiled for x86_64 "
					  "(or as universal binary).";
#  endif
		}
#endif
		throw IBK::Exception(IBK::FormatString("%1\nCannot load shared library '%2' (maybe missing dependencies).")
							 .arg(errMsg).arg(sharedLibraryPath), FUNC_ID);
	}
}


#endif // _WIN32

// ------------------------------------------------------------------------


} // namespace MASTER_SIM
