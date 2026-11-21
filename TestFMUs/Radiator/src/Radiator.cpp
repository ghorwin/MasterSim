/*
FMI interface for the test FMU 'Radiator'.

A radiator controlled by a P-controller with a step thermostat setpoint
profile. See Radiator.h for a description of the model and the FMI 2.0
interpolation features implemented by this FMU.
*/

#include <cstring>
#include <sstream>
#include <cmath>
#include <stdexcept>

#include "fmi2common/fmi2Functions.h"
#include "fmi2common/fmi2FunctionTypes.h"
#include "Radiator.h"


// *** GUID that uniquely identifies this FMU code
const char * const InstanceData::GUID = "{3b7e9d2a-5c8f-4a1b-b6e4-9d0c7a3f5e82}";

// *** Factory function, creates model specific instance of InstanceData-derived class
InstanceData * InstanceData::create() {
	return new Radiator; // caller takes ownership
}


Radiator::Radiator() :
	InstanceData()
{
	// initialize input variables
	m_realVar[VR_roomTemperature] = 275.15;

	// initialize output variables
	m_realVar[VR_heatingLoad] = 0.0;

	// initialize parameter variables
	m_realVar[VR_controllerGain] = 2000.0;
	m_realVar[VR_maxHeatingLoad] = 10000.0;
	m_realVar[VR_dayLength] = 14400.0;
	m_realVar[VR_setpointDay] = 295.15;  // 22 degC
	m_realVar[VR_setpointNight] = 289.15; // 16 degC
	m_integerVar[VR_inputInterpolationMode] = IIM_INTERNAL_HISTORY;

	// model state
	m_currentTimePoint = 0.0;
	m_tPrev = -1.0; // no history yet
	m_TPrev = 275.15;
	m_dTdt = 0.0;
	m_inputdTdt = 0.0;
	m_heatingLoad = 0.0;

	// parameters (updated in init())
	m_controllerGain = 2000.0;
	m_maxHeatingLoad = 10000.0;
	m_dayLength = 14400.0;
	m_setpointDay = 295.15;
	m_setpointNight = 289.15;
	m_inputInterpolationMode = IIM_INTERNAL_HISTORY;
}


Radiator::~Radiator() {
}


// create a model instance
void Radiator::init() {
	logger(fmi2OK, "progress", "Starting initialization.");

	// read parameters
	m_controllerGain = m_realVar[VR_controllerGain];
	m_maxHeatingLoad = m_realVar[VR_maxHeatingLoad];
	m_dayLength = m_realVar[VR_dayLength];
	m_setpointDay = m_realVar[VR_setpointDay];
	m_setpointNight = m_realVar[VR_setpointNight];
	m_inputInterpolationMode = m_integerVar[VR_inputInterpolationMode];
	if (m_inputInterpolationMode < IIM_ZERO_ORDER_HOLD || m_inputInterpolationMode > IIM_MASTER_DERIVATIVES)
		throw std::runtime_error("Parameter 'inputInterpolationMode' must be 0, 1 or 2.");

	if (m_controllerGain < 0.0)
		throw std::runtime_error("Parameter 'controllerGain' must not be negative.");
	if (m_maxHeatingLoad < 0.0)
		throw std::runtime_error("Parameter 'maxHeatingLoad' must not be negative.");
	if (m_dayLength <= 0.0)
		throw std::runtime_error("Parameter 'dayLength' must be positive.");

	if (!m_modelExchange)
		m_currentTimePoint = m_tStart;

	// no input history yet
	m_tPrev = -1.0;
	m_TPrev = m_realVar[VR_roomTemperature];
	m_dTdt = 0.0;
	m_inputdTdt = 0.0;

	// compute the initial output
	m_heatingLoad = computeHeatingLoad(setpointAt(m_currentTimePoint), m_realVar[VR_roomTemperature]);
	m_realVar[VR_heatingLoad] = m_heatingLoad;

	logger(fmi2OK, "progress", "Initialization complete.");
}


// model exchange: implementation of derivative and output update
void Radiator::updateIfModified() {
	if (!m_externalInputVarsModified)
		return;

	m_heatingLoad = computeHeatingLoad(setpointAt(m_tInput), m_realVar[VR_roomTemperature]);
	m_realVar[VR_heatingLoad] = m_heatingLoad;

	// mark variables as updated
	m_externalInputVarsModified = false;
}


// Co-simulation: time integration
void Radiator::integrateTo(double tCommunicationIntervalEnd) {
	double h = tCommunicationIntervalEnd - m_currentTimePoint;
	if (h <= 0.0)
		throw std::runtime_error("Communication step size must be positive.");

	double Tcur = m_realVar[VR_roomTemperature];
	double Tset = setpointAt(tCommunicationIntervalEnd);

	double Test = Tcur;
	if (m_inputInterpolationMode == IIM_INTERNAL_HISTORY && (m_tPrev >= 0.0)) {
		// the input is assumed to change linearly between the communication
		// points. Estimate the room temperature at the end of the current step
		// by linear extrapolation from the two most recent communication points.
		m_dTdt = (Tcur - m_TPrev) / (m_currentTimePoint - m_tPrev);
		Test = Tcur + m_dTdt * h;
	}
	else if (m_inputInterpolationMode == IIM_MASTER_DERIVATIVES) {
		// FMI 2.0: input and its derivative are set with respect to the begin
		// of the communication step: T(t) = T(tn) + dT/dt * (t - tn)
		m_dTdt = m_inputdTdt;
		Test = Tcur + m_dTdt * h;
	}
	else {
		// zero-order hold: use the current input value as is
		m_dTdt = 0.0;
	}

	m_heatingLoad = computeHeatingLoad(Tset, Test);
	m_realVar[VR_heatingLoad] = m_heatingLoad;

	// remember the current communication point for the history of the next step
	m_tPrev = m_currentTimePoint;
	m_TPrev = Tcur;
	// input derivatives are only valid for this step
	m_inputdTdt = 0.0;
	m_currentTimePoint = tCommunicationIntervalEnd;
}


// FMI 2.0 co-simulation: output derivatives
fmi2Status Radiator::getRealOutputDerivatives(const unsigned int vr[], size_t nvr, const int order[], double value[]) {
	for (size_t i = 0; i < nvr; ++i) {
		if (vr[i] != VR_heatingLoad) {
			logger(fmi2Warning, "logStatusWarning", "Unknown variable in fmi2GetRealOutputDerivatives().");
			return fmi2Warning;
		}
		if (order[i] == 0) {
			value[i] = m_heatingLoad;
		}
		else if (order[i] == 1) {
			// dQ/dt = -Kp * dT_room/dt, except when the output is clamped
			if ((m_heatingLoad > 0.0) && (m_heatingLoad < m_maxHeatingLoad))
				value[i] = -m_controllerGain * m_dTdt;
			else
				value[i] = 0.0;
		}
		else {
			logger(fmi2Warning, "logStatusWarning", "Only output derivatives of order 0 and 1 are supported.");
			return fmi2Warning;
		}
	}
	return fmi2OK;
}


// FMI 2.0 co-simulation: input derivatives (valid for the next fmi2DoStep() only)
fmi2Status Radiator::setRealInputDerivatives(const unsigned int vr[], size_t nvr, const int order[], const double value[]) {
	for (size_t i = 0; i < nvr; ++i) {
		if (order[i] < 1) {
			logger(fmi2Error, "logStatusError", "Invalid derivative order in fmi2SetRealInputDerivatives().");
			return fmi2Error;
		}
		if (vr[i] != VR_roomTemperature) {
			logger(fmi2Error, "logStatusError", "Variable in fmi2SetRealInputDerivatives() is not a continuous real input.");
			return fmi2Error;
		}
		if (order[i] == 1)
			m_inputdTdt = value[i];
		// higher order derivatives are ignored (model uses linear extrapolation)
	}
	return fmi2OK;
}


double Radiator::setpointAt(double t) const {
	// step thermostat: first half of the day at day setpoint, second half at night setpoint
	double tmod = t - std::floor(t / m_dayLength) * m_dayLength;
	return (tmod < 0.5 * m_dayLength) ? m_setpointDay : m_setpointNight;
}


double Radiator::computeHeatingLoad(double Tset, double Troom) const {
	double Q = m_controllerGain * (Tset - Troom);
	if (Q < 0.0)
		Q = 0.0;
	if (Q > m_maxHeatingLoad)
		Q = m_maxHeatingLoad;
	return Q;
}


void Radiator::computeFMUStateSize() {
	// time and model state
	m_fmuStateSize = sizeof(double)*6; // m_currentTimePoint, m_tPrev, m_TPrev, m_dTdt, m_inputdTdt, m_heatingLoad

	// we store all cached variables
	m_fmuStateSize += sizeof(int)*4; // map sizes for sanity checks

	m_fmuStateSize += (sizeof(int) + sizeof(double))*m_realVar.size();
	m_fmuStateSize += (sizeof(int) + sizeof(int))*m_integerVar.size();
	m_fmuStateSize += (sizeof(int) + sizeof(int))*m_boolVar.size(); // booleans are stored as int

	// strings are serialized in checkable format: first length, then zero-terminated string
	for (std::map<int, std::string>::const_iterator it = m_stringVar.begin();
		 it != m_stringVar.end(); ++it)
	{
		m_fmuStateSize += sizeof(int) + sizeof(int) + it->second.size() + 1; // add one char for \0
	}
}


// macro for storing a POD and increasing the pointer to the linear memory array
#define SERIALIZE(type, storageDataPtr, value)\
{\
  *reinterpret_cast<type *>(storageDataPtr) = (value);\
  (storageDataPtr) = reinterpret_cast<char *>(storageDataPtr) + sizeof(type);\
}

// macro for retrieving a POD and increasing the pointer to the linear memory array
#define DESERIALIZE(type, storageDataPtr, value)\
{\
  (value) = *reinterpret_cast<type *>(storageDataPtr);\
  (storageDataPtr) = reinterpret_cast<const char *>(storageDataPtr) + sizeof(type);\
}


template <typename T>
bool deserializeMap(Radiator * obj, const char * & dataPtr, const char * typeID, std::map<int, T> & varMap) {
	// now de-serialize the maps: first the size (for checking), then each key-value pair
	int mapsize;
	DESERIALIZE(const int, dataPtr, mapsize);
	if (mapsize != static_cast<int>(varMap.size())) {
		std::stringstream strm;
		strm << "Bad binary data or invalid/uninitialized model data. " << typeID << "-Map size mismatch.";
		obj->logger(fmi2Error, "deserialization", strm.str());
		return false;
	}
	for (int i = 0; i < mapsize; ++i) {
		int valueRef;
		T val;
		DESERIALIZE(const int, dataPtr, valueRef);
		if (varMap.find(valueRef) == varMap.end()) {
			std::stringstream strm;
			strm << "Bad binary data or invalid/uninitialized model data. " << typeID
				 << "-Variable with value ref " << valueRef
				 << " does not exist in " << typeID << "-variable map.";
			obj->logger(fmi2Error, "deserialization", strm.str());
			return false;
		}
		DESERIALIZE(const T, dataPtr, val);
		varMap[valueRef] = val;
	}
	return true;
}


void Radiator::serializeFMUstate(void * FMUstate) {
	char * dataPtr = reinterpret_cast<char*>(FMUstate);

	// time and model state
	SERIALIZE(double, dataPtr, m_currentTimePoint);
	SERIALIZE(double, dataPtr, m_tPrev);
	SERIALIZE(double, dataPtr, m_TPrev);
	SERIALIZE(double, dataPtr, m_dTdt);
	SERIALIZE(double, dataPtr, m_inputdTdt);
	SERIALIZE(double, dataPtr, m_heatingLoad);

	// write map size for checking
	int mapSize = static_cast<int>(m_realVar.size());
	SERIALIZE(int, dataPtr, mapSize);
	// now serialize all members of the map
	for (std::map<int,double>::const_iterator it = m_realVar.begin(); it != m_realVar.end(); ++it) {
		SERIALIZE(int, dataPtr, it->first);
		SERIALIZE(double, dataPtr, it->second);
	}
	mapSize = static_cast<int>(m_integerVar.size());
	SERIALIZE(int, dataPtr, mapSize);
	for (std::map<int,int>::const_iterator it = m_integerVar.begin(); it != m_integerVar.end(); ++it) {
		SERIALIZE(int, dataPtr, it->first);
		SERIALIZE(int, dataPtr, it->second);
	}
	mapSize = static_cast<int>(m_boolVar.size());
	SERIALIZE(int, dataPtr, mapSize);
	for (std::map<int,int>::const_iterator it = m_boolVar.begin(); it != m_boolVar.end(); ++it) {
		SERIALIZE(int, dataPtr, it->first);
		SERIALIZE(int, dataPtr, it->second);
	}
	mapSize = static_cast<int>(m_stringVar.size());
	SERIALIZE(int, dataPtr, mapSize);
	for (std::map<int, std::string>::const_iterator it = m_stringVar.begin();
		 it != m_stringVar.end(); ++it)
	{
		SERIALIZE(int, dataPtr, it->first);		// map key
		SERIALIZE(int, dataPtr, static_cast<int>(it->second.size()));	// string size
		std::memcpy(dataPtr, it->second.c_str(), it->second.size()+1); // also copy the trailing \0
		dataPtr += it->second.size()+1;
	}
}


bool Radiator::deserializeFMUstate(void * FMUstate) {
	const char * dataPtr = reinterpret_cast<const char*>(FMUstate);

	// time and model state
	DESERIALIZE(const double, dataPtr, m_currentTimePoint);
	DESERIALIZE(const double, dataPtr, m_tPrev);
	DESERIALIZE(const double, dataPtr, m_TPrev);
	DESERIALIZE(const double, dataPtr, m_dTdt);
	DESERIALIZE(const double, dataPtr, m_inputdTdt);
	DESERIALIZE(const double, dataPtr, m_heatingLoad);
	m_realVar[VR_heatingLoad] = m_heatingLoad;

	if (!deserializeMap(this, dataPtr, "real", m_realVar))
		return false;
	if (!deserializeMap(this, dataPtr, "integer", m_integerVar))
		return false;
	if (!deserializeMap(this, dataPtr, "boolean", m_boolVar))
		return false;

	// special handling for deserialization of string map
	int mapsize;
	DESERIALIZE(const int, dataPtr, mapsize);
	if (mapsize != static_cast<int>(m_stringVar.size())) {
		logger(fmi2Error, "deserialization", "Bad binary data or invalid/uninitialized model data. string-variable map size mismatch.");
		return false;
	}
	for (int i = 0; i < mapsize; ++i) {
		int valueRef;
		DESERIALIZE(const int, dataPtr, valueRef);
		if (m_stringVar.find(valueRef) == m_stringVar.end()) {
			std::stringstream strm;
			strm << "Bad binary data. string-variable with value ref " << valueRef
				 << " does not exist in real variable map.";
			logger(fmi2Error, "deserialization", strm.str());
			return false;
		}
		// get length of string
		int strLen;
		DESERIALIZE(const int, dataPtr, strLen);
		// create a string of requested length
		std::string s(static_cast<size_t>(strLen), ' ');
		// copy contents of string
		std::memcpy(&s[0], dataPtr, static_cast<size_t>(strLen)); // do not copy the trailing \0
		dataPtr += strLen;
		// check that next character is a \0
		if (*dataPtr != '\0') {
			std::stringstream strm;
			strm << "Bad binary data. string-variable with value ref " << valueRef
				 << " does not have a trailing \0.";
			logger(fmi2Error, "deserialization", strm.str());
			return false;
		}
		++dataPtr;
		// replace value in map
		m_stringVar[valueRef] = s;
	}

	return true;
}
