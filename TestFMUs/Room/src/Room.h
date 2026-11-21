/*
Simple room model as FMI 2.0 co-simulation FMU (also supports Model Exchange).

Model: energy balance of a room with heat loss to the environment

	C dT/dt = Q(t) - k (T - T_amb(t))

where
	C   heat capacity of the room        [J/K]   (parameter 'heatCapacity')
	k   heat loss coefficient           [W/K]   (parameter 'heatLossCoefficient')
	Q   heating load                    [W]     (input 'heatingLoad')
	T   room temperature                [K]     (output 'roomTemperature')
	T_amb ambient temperature           [K]     (input 'ambientTemperature')

FMI 2.0 interpolation features (see the FMI 2.0 specification):

- CoSimulation/canInterpolateInputs = "true":
  during fmi2DoStep() the inputs are assumed to change linearly over the
  communication step, from their value in the previous step to the value
  set for the current step. The room temperature is then computed with the
  exact (closed-form) solution of the first order ODE for a linear input
  profile, which makes the result much more accurate than a simple
  zero-order-hold (constant input) step.

- CoSimulation/maxOutputDerivativeOrder = "1":
  fmi2GetRealOutputDerivatives() provides the first time derivative of the
  output roomTemperature, so that a master can interpolate the output
  between communication points.

The parameter 'useInputInterpolation' (boolean) switches the internal input
interpolation on (default) or off, to demonstrate the effect of the feature
in co-simulation test cases.
*/

#ifndef RoomH
#define RoomH

#include "fmi2common/InstanceData.h"

class Room : public InstanceData {
public:
	// FMI variable value references
	static constexpr int VR_heatingLoad           = 1; // input,    [W]
	static constexpr int VR_ambientTemperature    = 2; // input,    [K]
	static constexpr int VR_roomTemperature       = 3; // output,   [K]
	static constexpr int VR_heatCapacity          = 4; // parameter,[J/K]
	static constexpr int VR_heatLossCoefficient   = 5; // parameter,[W/K]
	static constexpr int VR_useInputInterpolation = 6; // parameter,boolean

	Room();
	virtual ~Room();

	// InstanceData interface
	void init();
	void updateIfModified(); // Model Exchange
	void integrateTo(double tCommunicationIntervalEnd); // Co-Simulation
	fmi2Status getRealOutputDerivatives(const unsigned int vr[], size_t nvr, const int order[], double value[]);
	void computeFMUStateSize();
	void serializeFMUstate(void * FMUstate);
	bool deserializeFMUstate(void * FMUstate);

private:
	// state of the model
	double m_currentTimePoint;     // current time point [s]
	double m_T;                    // room temperature [K] at m_currentTimePoint
	double m_prevHeatingLoad;     // heating load during the previous communication step [W]
	double m_prevAmbientTemperature; // ambient temperature during the previous communication step [K]

	// parameters (copied from the variable maps during init())
	double m_heatCapacity;        // [J/K]
	double m_heatLossCoefficient; // [W/K]
	bool   m_useInputInterpolation;

	// time derivative of the room temperature
	double computeDerivative(double T, double Q, double T_amb) const;

	// exact solution of C dT/dt = Q(t) - k (T - A(t)) over [0, h] with linear
	// input profiles Q(t) = Q0 + slopeQ t and A(t) = A0 + slopeA t
	double integrateLinear(double T0, double Q0, double slopeQ, double A0, double slopeA, double h) const;
};

#endif // RoomH
