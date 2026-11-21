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

The integer parameter 'inputInterpolationMode' selects how the inputs are
assumed to change within a communication step:

  0 - zero-order hold: inputs are constant over the step
  1 - internal history (default): the value set for the current step is
      interpreted as the value at the END of the step, and the inputs change
      linearly from the value of the previous step to the current value
      (assumes a Gauss-Seidel master that evaluates the producer first)
  2 - master derivatives: the value set for the current step is the value at
      the BEGIN of the step (as defined by the FMI standard), and the first
      derivatives set via fmi2SetRealInputDerivatives() define the slope
      within the step. Derivatives are only valid for the following
      fmi2DoStep() and are reset to 0 afterwards (zero-order hold if the
      master does not set them).
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
	static constexpr int VR_inputInterpolationMode = 6; // parameter,integer

	// values of parameter inputInterpolationMode
	enum InputInterpolationMode {
		IIM_ZERO_ORDER_HOLD = 0,
		IIM_INTERNAL_HISTORY = 1,
		IIM_MASTER_DERIVATIVES = 2
	};

	Room();
	virtual ~Room();

	// InstanceData interface
	void init();
	void updateIfModified(); // Model Exchange
	void integrateTo(double tCommunicationIntervalEnd); // Co-Simulation
	fmi2Status getRealOutputDerivatives(const unsigned int vr[], size_t nvr, const int order[], double value[]);
	fmi2Status setRealInputDerivatives(const unsigned int vr[], size_t nvr, const int order[], const double value[]);
	void computeFMUStateSize();
	void serializeFMUstate(void * FMUstate);
	bool deserializeFMUstate(void * FMUstate);

private:
	// state of the model
	double m_currentTimePoint;     // current time point [s]
	double m_T;                    // room temperature [K] at m_currentTimePoint
	double m_prevHeatingLoad;     // heating load during the previous communication step [W]
	double m_prevAmbientTemperature; // ambient temperature during the previous communication step [K]
	bool   m_haveEndInputs;         // true after the first step, then m_QEnd/m_AEnd hold inputs at m_currentTimePoint
	double m_QEnd;                  // heating load at m_currentTimePoint, as assumed in the last step [W]
	double m_AEnd;                  // ambient temperature at m_currentTimePoint, as assumed in the last step [K]

	// input derivatives set via fmi2SetRealInputDerivatives(), valid for the next step only
	double m_dQdt;                  // [W/s]
	double m_dAdt;                  // [K/s]

	// parameters (copied from the variable maps during init())
	double m_heatCapacity;        // [J/K]
	double m_heatLossCoefficient; // [W/K]
	int    m_inputInterpolationMode;

	// time derivative of the room temperature
	double computeDerivative(double T, double Q, double T_amb) const;

	// exact solution of C dT/dt = Q(t) - k (T - A(t)) over [0, h] with linear
	// input profiles Q(t) = Q0 + slopeQ t and A(t) = A0 + slopeA t
	double integrateLinear(double T0, double Q0, double slopeQ, double A0, double slopeA, double h) const;
};

#endif // RoomH
