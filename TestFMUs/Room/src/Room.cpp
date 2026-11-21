/*
FMI interface for the test FMU 'Room'.

A simple room model that receives a heating load and an ambient temperature
and returns the room temperature:

	C dT/dt = Q(t) - k (T - T_amb(t))

FMI 2.0 interpolation features (see Room.h for details):
- canInterpolateInputs="true": during fmi2DoStep() the inputs are assumed to
  change linearly over the communication step, from their value in the
  previous step to the value set for the current step. The room temperature
  is computed with the exact (closed-form) solution of the ODE for a linear
  input profile.
- maxOutputDerivativeOrder="1": fmi2GetRealOutputDerivatives() provides the
  first time derivative of the output roomTemperature.

The integer parameter 'inputInterpolationMode' selects zero-order hold (0),
interpolation from the internal input history (1, default) or the use of
input derivatives set by the master via fmi2SetRealInputDerivatives() (2),
see Room.h.
*/

#include <cstring>
#include <sstream>
#include <cmath>
#include <stdexcept>

#include "fmi2common/fmi2Functions.h"
#include "fmi2common/fmi2FunctionTypes.h"
#include "Room.h"


// *** GUID that uniquely identifies this FMU code
const char * const InstanceData::GUID = "{8f2a1c4d-7b6e-4d3a-9c1f-5e8b2a6d4c01}";

// *** Factory function, creates model specific instance of InstanceData-derived class
InstanceData * InstanceData::create() {
	return new Room; // caller takes ownership
}


Room::Room() :
	InstanceData()
{
	// initialize input variables
	m_realVar[VR_heatingLoad] = 0.0;
	m_realVar[VR_ambientTemperature] = 275.15;

	// initialize output variables
	m_realVar[VR_roomTemperature] = 275.15;

	// initialize parameter variables
	m_realVar[VR_heatCapacity] = 50000.0;
	m_realVar[VR_heatLossCoefficient] = 250.0;
	m_integerVar[VR_inputInterpolationMode] = IIM_INTERNAL_HISTORY;

	// model state
	m_currentTimePoint = 0.0;
	m_T = 275.15;
	m_prevHeatingLoad = 0.0;
	m_prevAmbientTemperature = 275.15;
	m_haveEndInputs = false;
	m_QEnd = 0.0;
	m_AEnd = 275.15;
	m_dQdt = 0.0;
	m_dAdt = 0.0;

	// parameters (updated in init())
	m_heatCapacity = 50000.0;
	m_heatLossCoefficient = 250.0;
	m_inputInterpolationMode = IIM_INTERNAL_HISTORY;
}


Room::~Room() {
}


// create a model instance
void Room::init() {
	logger(fmi2OK, "progress", "Starting initialization.");

	// read parameters
	m_heatCapacity = m_realVar[VR_heatCapacity];
	m_heatLossCoefficient = m_realVar[VR_heatLossCoefficient];
	m_inputInterpolationMode = m_integerVar[VR_inputInterpolationMode];
	if (m_inputInterpolationMode < IIM_ZERO_ORDER_HOLD || m_inputInterpolationMode > IIM_MASTER_DERIVATIVES)
		throw std::runtime_error("Parameter 'inputInterpolationMode' must be 0, 1 or 2.");

	if (m_heatCapacity <= 0.0)
		throw std::runtime_error("Parameter 'heatCapacity' must be positive.");
	if (m_heatLossCoefficient < 0.0)
		throw std::runtime_error("Parameter 'heatLossCoefficient' must not be negative.");

	// the room temperature at the start of the simulation is taken from the
	// variable map (set by the FMU environment before fmi2Initialize())
	m_T = m_realVar[VR_roomTemperature];

	if (!m_modelExchange)
		m_currentTimePoint = m_tStart;

	// there is no input history yet, so the first communication step
	// is assumed to have constant input values
	m_prevHeatingLoad = m_realVar[VR_heatingLoad];
	m_prevAmbientTemperature = m_realVar[VR_ambientTemperature];
	m_haveEndInputs = false;
	m_dQdt = 0.0;
	m_dAdt = 0.0;

	logger(fmi2OK, "progress", "Initialization complete.");
}


// model exchange: implementation of derivative and output update
void Room::updateIfModified() {
	if (!m_externalInputVarsModified)
		return;

	m_T = m_realVar[VR_roomTemperature];

	// mark variables as updated
	m_externalInputVarsModified = false;
}


// Co-simulation: time integration
void Room::integrateTo(double tCommunicationIntervalEnd) {
	double h = tCommunicationIntervalEnd - m_currentTimePoint;
	if (h <= 0.0)
		throw std::runtime_error("Communication step size must be positive.");

	double Qcur = m_realVar[VR_heatingLoad];
	double Acur = m_realVar[VR_ambientTemperature];

	switch (m_inputInterpolationMode) {
		case IIM_INTERNAL_HISTORY :
			// in the first step there is no input history yet (the values remembered in init()
			// were taken before the master set the inputs in initialization mode)
			if (!m_haveEndInputs) {
				m_prevHeatingLoad = Qcur;
				m_prevAmbientTemperature = Acur;
			}
			// the inputs are assumed to change linearly between the communication
			// points, from their previous value to the current one (current value
			// is interpreted as value at the end of the step)
			m_T = integrateLinear(m_T,
								  m_prevHeatingLoad, (Qcur - m_prevHeatingLoad) / h,
								  m_prevAmbientTemperature, (Acur - m_prevAmbientTemperature) / h,
								  h);
			m_QEnd = Qcur;
			m_AEnd = Acur;
			break;

		case IIM_MASTER_DERIVATIVES :
			// FMI 2.0: inputs and their derivatives are set with respect to the
			// begin of the communication step: u(t) = u(tn) + du/dt * (t - tn)
			m_T = integrateLinear(m_T, Qcur, m_dQdt, Acur, m_dAdt, h);
			m_QEnd = Qcur + m_dQdt*h;
			m_AEnd = Acur + m_dAdt*h;
			break;

		default : // IIM_ZERO_ORDER_HOLD
			// the inputs are assumed to stay at their current value
			m_T = integrateLinear(m_T, Qcur, 0.0, Acur, 0.0, h);
			m_QEnd = Qcur;
			m_AEnd = Acur;
	}
	m_haveEndInputs = true;

	// input derivatives are only valid for this step
	m_dQdt = 0.0;
	m_dAdt = 0.0;

	m_realVar[VR_roomTemperature] = m_T;

	// remember the current input values as the starting values of the next step
	m_prevHeatingLoad = Qcur;
	m_prevAmbientTemperature = Acur;
	m_currentTimePoint = tCommunicationIntervalEnd;
}


// FMI 2.0 co-simulation: output derivatives
fmi2Status Room::getRealOutputDerivatives(const unsigned int vr[], size_t nvr, const int order[], double value[]) {
	for (size_t i = 0; i < nvr; ++i) {
		if (vr[i] != VR_roomTemperature) {
			logger(fmi2Warning, "logStatusWarning", "Unknown variable in fmi2GetRealOutputDerivatives().");
			return fmi2Warning;
		}
		if (order[i] == 0) {
			value[i] = m_T;
		}
		else if (order[i] == 1) {
			// derivative at the current communication point, using the input values that the
			// model assumed at the end of the last step (or the current inputs before the first step)
			if (m_haveEndInputs)
				value[i] = computeDerivative(m_T, m_QEnd, m_AEnd);
			else
				value[i] = computeDerivative(m_T, m_realVar[VR_heatingLoad], m_realVar[VR_ambientTemperature]);
		}
		else {
			logger(fmi2Warning, "logStatusWarning", "Only output derivatives of order 0 and 1 are supported.");
			return fmi2Warning;
		}
	}
	return fmi2OK;
}


// FMI 2.0 co-simulation: input derivatives (valid for the next fmi2DoStep() only)
fmi2Status Room::setRealInputDerivatives(const unsigned int vr[], size_t nvr, const int order[], const double value[]) {
	for (size_t i = 0; i < nvr; ++i) {
		if (order[i] < 1) {
			logger(fmi2Error, "logStatusError", "Invalid derivative order in fmi2SetRealInputDerivatives().");
			return fmi2Error;
		}
		if (order[i] > 1)
			continue; // higher order derivatives are ignored (model uses linear input profiles)
		switch (vr[i]) {
			case VR_heatingLoad			: m_dQdt = value[i]; break;
			case VR_ambientTemperature	: m_dAdt = value[i]; break;
			default :
				logger(fmi2Error, "logStatusError", "Variable in fmi2SetRealInputDerivatives() is not a continuous real input.");
				return fmi2Error;
		}
	}
	return fmi2OK;
}


double Room::computeDerivative(double T, double Q, double T_amb) const {
	return (Q - m_heatLossCoefficient * (T - T_amb)) / m_heatCapacity;
}


double Room::integrateLinear(double T0, double Q0, double slopeQ, double A0, double slopeA, double h) const {
	// exact solution of
	//   C dT/dtau = Q0 + slopeQ*tau - k * (T - A0 - slopeA*tau)
	// i.e.
	//   dT/dtau = -lambda * T + a0 + a1 * tau,  tau in [0, h]
	//
	//   T(h) = T0*e^(-x) + a0*h*(E/x) + a1*(h/lambda)*(1 - E/x)
	// with x = lambda*h, E/x = (1 - e^(-x))/x
	//
	// The form with (E/x) is numerically stable for small x (E/x -> 1,
	// (1 - E/x) -> x/2, so a1*(h/lambda)*(1-E/x) -> a1*h^2/2).
	double lambda = m_heatLossCoefficient / m_heatCapacity;
	double a0 = (Q0 + m_heatLossCoefficient * A0) / m_heatCapacity;
	double a1 = (slopeQ + m_heatLossCoefficient * slopeA) / m_heatCapacity;

	double x = lambda * h;
	double EoverX;
	if (std::fabs(x) < 1e-8) {
		// Taylor series: (1 - e^-x)/x = 1 - x/2 + x^2/6 - x^3/24
		EoverX = 1.0 - x*(0.5 - x*(1.0/6.0 - x/24.0));
	}
	else {
		EoverX = (-std::expm1(-x)) / x;
	}

	double result = T0 * std::exp(-x) + a0 * h * EoverX;
	if (std::fabs(lambda) > 1e-30) {
		result += a1 * (h / lambda) * (1.0 - EoverX);
	}
	else {
		// no heat loss: pure polynomial
		result += 0.5 * a1 * h * h;
	}
	return result;
}


void Room::computeFMUStateSize() {
	// time and model state
	m_fmuStateSize = sizeof(double)*8; // m_currentTimePoint, m_T, m_prevHeatingLoad, m_prevAmbientTemperature, m_QEnd, m_AEnd, m_dQdt, m_dAdt
	m_fmuStateSize += sizeof(int); // m_haveEndInputs

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
bool deserializeMap(Room * obj, const char * & dataPtr, const char * typeID, std::map<int, T> & varMap) {
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


void Room::serializeFMUstate(void * FMUstate) {
	char * dataPtr = reinterpret_cast<char*>(FMUstate);

	// time and model state
	SERIALIZE(double, dataPtr, m_currentTimePoint);
	SERIALIZE(double, dataPtr, m_T);
	SERIALIZE(double, dataPtr, m_prevHeatingLoad);
	SERIALIZE(double, dataPtr, m_prevAmbientTemperature);
	SERIALIZE(double, dataPtr, m_QEnd);
	SERIALIZE(double, dataPtr, m_AEnd);
	SERIALIZE(double, dataPtr, m_dQdt);
	SERIALIZE(double, dataPtr, m_dAdt);
	SERIALIZE(int, dataPtr, m_haveEndInputs ? 1 : 0);

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


bool Room::deserializeFMUstate(void * FMUstate) {
	const char * dataPtr = reinterpret_cast<const char*>(FMUstate);

	// time and model state
	DESERIALIZE(const double, dataPtr, m_currentTimePoint);
	DESERIALIZE(const double, dataPtr, m_T);
	DESERIALIZE(const double, dataPtr, m_prevHeatingLoad);
	DESERIALIZE(const double, dataPtr, m_prevAmbientTemperature);
	DESERIALIZE(const double, dataPtr, m_QEnd);
	DESERIALIZE(const double, dataPtr, m_AEnd);
	DESERIALIZE(const double, dataPtr, m_dQdt);
	DESERIALIZE(const double, dataPtr, m_dAdt);
	int haveEndInputs;
	DESERIALIZE(const int, dataPtr, haveEndInputs);
	m_haveEndInputs = (haveEndInputs != 0);
	m_realVar[VR_roomTemperature] = m_T;

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
