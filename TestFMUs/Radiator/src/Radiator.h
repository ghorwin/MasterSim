/*
Radiator model with a step thermostat as FMI 2.0 co-simulation FMU
(also supports Model Exchange).

Model: a radiator controlled by a P-controller with a step thermostat
setpoint profile:

	setpoint(t) = setpointDay  for  (t mod dayLength) < dayLength/2
	              setpointNight for  (t mod dayLength) >= dayLength/2
	Q(t) = clamp( Kp * (setpoint(t) - T_room_est(t)), 0, Qmax )

where
	Kp     controller gain                       [W/K] (parameter 'controllerGain')
	Qmax   maximum heating load                  [W]   (parameter 'maxHeatingLoad')
	T_room room temperature                      [K]   (input 'roomTemperature')

FMI 2.0 interpolation features (see the FMI 2.0 specification):

- CoSimulation/canInterpolateInputs = "true":
  the integer parameter 'inputInterpolationMode' selects how the room
  temperature at the end of the current communication step is estimated:

  0 - zero-order hold: the current input value is used as is
  1 - internal history (default): linear extrapolation from the two most
      recent communication points, so that the heating output anticipates
      the temperature trend of the room
  2 - master derivatives: T_est = T(tn) + dT/dt(tn) * h, with the first
      derivative set by the master via fmi2SetRealInputDerivatives().
      Derivatives are only valid for the following fmi2DoStep() and are
      reset to 0 afterwards (zero-order hold if the master does not set them).

- CoSimulation/maxOutputDerivativeOrder = "1":
  fmi2GetRealOutputDerivatives() provides the first time derivative of the
  output heatingLoad, so that a master can interpolate the output
  between communication points.
*/

#ifndef RadiatorH
#define RadiatorH

#include "fmi2common/InstanceData.h"

class Radiator : public InstanceData {
public:
	// FMI variable value references
	static constexpr int VR_roomTemperature     = 1; // input,    [K]
	static constexpr int VR_heatingLoad         = 2; // output,   [W]
	static constexpr int VR_controllerGain      = 3; // parameter,[W/K]
	static constexpr int VR_maxHeatingLoad      = 4; // parameter,[W]
	static constexpr int VR_dayLength           = 5; // parameter,[s]
	static constexpr int VR_setpointDay         = 6; // parameter,[K]
	static constexpr int VR_setpointNight       = 7; // parameter,[K]
	static constexpr int VR_inputInterpolationMode = 8; // parameter,integer

	// values of parameter inputInterpolationMode
	enum InputInterpolationMode {
		IIM_ZERO_ORDER_HOLD = 0,
		IIM_INTERNAL_HISTORY = 1,
		IIM_MASTER_DERIVATIVES = 2
	};

	Radiator();
	virtual ~Radiator();

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
	double m_currentTimePoint;  // current time point [s]
	double m_tPrev;             // time point of the previous communication point [s] (< 0 if no history)
	double m_TPrev;             // room temperature at the previous communication point [K]
	double m_dTdt;              // rate of change of the room temperature used in the last step [K/s]
	double m_inputdTdt;         // input derivative set via fmi2SetRealInputDerivatives(), valid for next step only [K/s]
	double m_heatingLoad;      // current heating load [W]

	// parameters (copied from the variable maps during init())
	double m_controllerGain;   // [W/K]
	double m_maxHeatingLoad;   // [W]
	double m_dayLength;        // [s]
	double m_setpointDay;      // [K]
	double m_setpointNight;    // [K]
	int    m_inputInterpolationMode;

	// setpoint of the step thermostat at time t
	double setpointAt(double t) const;

	// heating load from the P-controller (clamped to [0, Qmax])
	double computeHeatingLoad(double Tset, double Troom) const;
};

#endif // RadiatorH
