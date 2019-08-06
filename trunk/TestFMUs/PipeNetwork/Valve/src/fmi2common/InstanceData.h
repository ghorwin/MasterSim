/*

Generic FMI Interface Implementation

This file is part of FMICodeGenerator (https://github.com/ghorwin/FMICodeGenerator)

BSD 3-Clause License

Copyright (c) 2018, Andreas Nicolai
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef InstanceDataH
#define InstanceDataH

#include <vector>
#include <map>
#include <set>
#include <string>

#include "fmi2FunctionTypes.h"

/*! This class wraps data needed for FMUs and implements common functionality.

	In order to use this in your own InstanceData structure, you must inherit this
	file and implement the following functions:

	\code
	class MyFMIClass : public InstanceDataCommon {
	public:
		// Initializes InstanceData
		void init();

		// Re-implement if you want ModelExchange support
		virtual void updateIfModified();
		// Re-implement if you want CoSim support
		virtual void integrateTo(double tCommunicationIntervalEnd);
	};
	\endcode

	Also, you must define the constant with some GUID and implement the static function create().
	\code
	const char * const InstanceData::GUID = "{471a3b52-4923-44d8-ab4a-fcdb813c7322}";

	InstanceData * InstanceData::create() {
		return new MyFMIClass;
	}
	\endcode
	in your MyFMIClass.cpp file.
*/
class InstanceData {
public:
	/*! Global unique ID that identifies this FMU.
		Must match the GUID in the ModelDescription file.
		\note This GUID is model-specific, so you must define this static symbol
			  in the cpp file of your derived class.
	*/
	static const char * const GUID;
	/*! Factory function, needs to be implemented in user code.
		\note Only the model-specific implementation knows the concrete type of the class
			derived from abstract InterfaceData class, and so this function must be
			implemented in the model-specific code.
	*/
	static InstanceData * create();


	/*! Initializes empty instance.
		\note You must initialize all input and output variables here, since input
			variable can be set and output variables can be requested, even before
			entering initialization mode (i.e. before a call to init()).
	*/
	InstanceData();

	/*! Destructor, resource cleanup. */
	virtual ~InstanceData();

	/*! Re-implement this function in derived classes to perform initialization
		of the model during the initialization phase.
	*/
	virtual void init() {}

	/*! This function triggers a state-update of the embedded model whenever our cached input
		data differs from the input data in the model.
		Re-implement in models that support model exchange.
		\note You should check the state of m_externalInputVarsModified and only
			update the results of this flag is true. Afterwards set this flag to false.
	*/
	virtual void updateIfModified() {}

	/*! Called from fmi2DoStep().
		Re-implement in models that support co-simulation.
	*/
	virtual void integrateTo(double tCommunicationIntervalEnd) { (void)tCommunicationIntervalEnd; }

	/*! Send a logging message to FMU environment if logger is present.*/
	void logger(fmi2Status state, fmi2String category, fmi2String msg);

	/*! Send a logging message to FMU environment if logger is present.
		This function copies the error message (which may be a temporary string object) into the
		persistent member variable m_lastError and passes a pointer to this string through the
		logger function.
	*/
	void logger(fmi2Status state, fmi2String category, const std::string & msg) {
		m_lastError = msg;
		logger(state, category, m_lastError.c_str());
	}

	/*! Sets a new input parameter of type double. */
	void setReal(int varID, double value);

	/*! Sets a new input parameter of type int. */
	void setInt(int varID, int value);

	/*! Sets a new input parameter of type string. */
	void setString(int varID, fmi2String value);

	/*! Sets a new input parameter of type bool. */
	void setBool(int varID, bool value);

	/*! Retrieves an output parameter of type double. */
	void getReal(int varID, double & value);

	/*! Retrieves an output parameter of type int. */
	void getInt(int varID, int & value);

	/*! Retrieves an output parameter of type string. */
	void getString(int varID, fmi2String & value);

	/*! Retrieves an output parameter of type bool. */
	void getBool(int varID, bool & value);

	/*! Called from fmi2CompletedIntegratorStep(): only ModelExchange. */
	void completedIntegratorStep();

	/*! Called from completedIntegratorStep(): only ModelExchange.
		Re-implement with your own code.
	*/
	virtual void completedIntegratorStep(double t_stepEnd, double * yInput) { (void)t_stepEnd; (void)yInput; }

	/*! Re-implement for getFMUState()/setFMUState() support.
		This function computes the size needed for full serizalization of
		the FMU and stores the size in m_fmuStateSize.
		\note The size includes the leading 8byte for the 64bit integer size
		of the memory array (for testing purposes).
		If serialization is not supported, the function will set an fmu size of 0.
	*/
	virtual void computeFMUStateSize() { m_fmuStateSize = 0; } // default implementation sets zero size = no serialization

	/*! Re-implement for getFMUState() support.
		Copies the internal state of the FMU to the memory array pointed to by FMUstate.
		Memory array always has size m_fmuStateSize.
	*/
	virtual void serializeFMUstate(void * FMUstate) { (void)FMUstate; }

	/*! Re-implement for setFMUState() support.
		Copies the content of the memory array pointed to by FMUstate to the internal state of the FMU.
		Memory array always has size m_fmuStateSize.

		\return Returns false if checks during deserialization fail.
	*/
	virtual bool deserializeFMUstate(void * FMUstate) { (void)FMUstate; return true; }

	/*! Called from either doStep() or terminate() in CoSimulation mode whenever
		a communication interval has been completed and all related buffers can be cleared/output files can be
		written.
	*/
	virtual void clearBuffers() {}

	/*! Stores the FMU callback functions for later use.
		It is usable between fmi2Instantiate and fmi2Terminate.*/
	const fmi2CallbackFunctions*	m_callbackFunctions;

	/*! True if in initialization mode. */
	bool							m_initializationMode;

	/*! Name of the instance inside the FMU environment.*/
	std::string						m_instanceName;

	/*! Resource root path as set via fmi2Instantiate(). */
	std::string						m_resourceLocation;

	/*! Logging enabled flag as set via fmi2Instantiate(). */
	bool							m_loggingOn;

	/*! Logging categories supported by the master. */
	std::vector<std::string>		m_loggingCategories;

	/*! If true, this is a ModelExchange FMU. */
	bool							m_modelExchange;

	std::map<int,int>				m_boolVar;
	std::map<int,double>			m_realVar;
	std::map<int,int>				m_integerVar;
	std::map<int,std::string>		m_stringVar;

	/*! Time point in [s] received by last call to fmi2SetTime(). */
	double							m_tInput;

	/*! Model state vector as received by last call to fmi2SetContinuousStates(). */
	std::vector<double>				m_yInput;

	/*! Model derivatives vector as updated by last call to updateIfModified(). */
	std::vector<double>				m_ydot;

	/*! Signals that one of the real parameter inputs have been changed.
		This flag is reset whenever updateIfModified() has been called.
		The flag is set in any of the setXXXParameter() functions.
	*/
	bool							m_externalInputVarsModified;

	/*! Holds the size of the FMU when serialized in memory.
		This value does not change after full initialization of the solver so it can be cached.
		Initially it will be zero so functions can check if initialization is properly done.
	*/
	size_t							m_fmuStateSize;
	/*! Holds pointers to all currently stored FMU states.
		Pointers get added in function fmi2GetFMUstate(), and removed in fmi2FreeFMUstate().
		Unreleased memory gets deallocated in destructor.
	*/
	std::set<void*>					m_fmuStates;

	/*! Persistent string pointing to last error message.
		\warning DO not change/resize string manually, only through logger() function.
	*/
	std::string						m_lastError;

}; // class InstanceData

#endif // InstanceDataH
