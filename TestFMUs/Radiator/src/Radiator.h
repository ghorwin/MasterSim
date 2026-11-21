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
  the FMU keeps a short history of the input roomTemperature. With
  'useInputInterpolation' enabled (default), the room temperature at the
  end of the current communication step is estimated by linear
  extrapolation from the two most recent communication points, so that the
  heating output anticipates the temperature trend of the room. With
  'useInputInterpolation' disabled, the current input value is used as is
  (zero-order hold). This demonstrates the effect of input interpolation
  in co-simulation test cases.

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
	static constexpr int VR_useInputInterpolation = 8; // parameter,boolean

	Radiator();
	virtual ~Radiator();

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
	double m_currentTimePoint;  // current time point [s]
	double m_tPrev;             // time point of the previous communication point [s] (< 0 if no history)
	double m_TPrev;             // room temperature at the previous communication point [K]
	double m_dTdt;              // rate of change of the room temperature from the history [K/s]
	double m_heatingLoad;      // current heating load [W]

	// parameters (copied from the variable maps during init())
	double m_controllerGain;   // [W/K]
	double m_maxHeatingLoad;   // [W]
	double m_dayLength;        // [s]
	double m_setpointDay;      // [K]
	double m_setpointNight;    // [K]
	bool   m_useInputInterpolation;

	// setpoint of the step thermostat at time t
	double setpointAt(double t) const;

	// heating load from the P-controller (clamped to [0, Qmax])
	double computeHeatingLoad(double Tset, double Troom) const;
};

#endif // RadiatorH
