#ifndef IBK_FluidPhysicsH
#define IBK_FluidPhysicsH

#include "IBK_MultiSpline.h"

#include <array>

namespace IBK {

/*!	\file IBK_FluidPhysics.h
	\brief Physical constants and functions needed to calculate heat transport and wall friction of fluids in pipes.
	The formulas are mainly taken from the 'VDI Wärmeatlas 11. Auflage 2013 chapt. G1 4'.
*/

/*! Calculates Reynolds number [-] of a moving fluid for a pipe.
	\param v Mean fluid flow velocity [m/s]
	\param kinVis Fluid kinematic viscosity [m2/s]
	\param d Pipe inside diameter [m]
*/
double ReynoldsNumber(double v, double kinVis, double d);

/*! Calculates Prandtl number [-] of a moving fluid.
	\param kinVis fluid kinematic viscosity [m2/s]
	\param cp fluid specific heat capacity [J/kgK]
	\param lambda fluid thermal conductivity [W/mK]
	\param rho fluid mass density [kg/m3]
*/
double PrandtlNumber(double kinVis, double cp, double lambda, double rho);

/*! Calculates nusselt number [-] for laminar fluid flow through a pipe.
	Nusselt number has fixed minimum to laminar
	\param reynolds Reynolds number [-]
	\param prandtl Prandtl number [-]
	\param l Length of the pipe [m]
	\param d Pipe inside diameter [m]
*/
double NusseltNumberLaminar(double reynolds, double prandtl, double l, double d);

/*! Calculates nusselt number [-] for a turbulent fluid through a pipe. Reynolds should not be 0! This function is only
 * called by NusseltNumber() in case Re > RE_LAMINAR and is taken from VDI Wärmeatlas G1 4.1
	\param reynolds Reynolds number [-]
	\param prandtl Prandtl number [-]
	\param l Length of the pipe [m]
	\param d Pipe inside diameter [m]
*/
double NusseltNumberTurbulent(double reynolds, double prandtl, double l, double d);

/*! Calculates nusselt number [-] for a turbulent fluid through a pipe.
	For Reynolds number below RE_LAMINAR NusseltNumberLaminar is used.
	For values higher than RE_TURBULENT NusseltNumberTurbulent is used.
	Between thses two numbers (transition area) a interpolation algorithm according (VDI Wärmeatlas G1 4.2 is used.
	\param reynolds Reynolds number [-]
	\param prandtl Prandtl number [-]
	\param l Length of the pipe [m]
	\param d Pipe inside diameter [m]
*/
double NusseltNumber(double reynolds, double prandtl, double l, double d);

/*! Calculates the darcy friction factor [-] according to swamee-jain equation (approximation of colebrook-white)
	\param reynolds Reynolds number [-]
	\param d Pipe inside diameter [m]
	\param roughness Pipe wall roughness [m]
*/
double FrictionFactorSwamee(double reynolds, double d, double roughness);

/*! Calculates the inner surface transmission coefficient for a pipe.
	\param nusselt Current Nusselt number
	\param lambda Thermal conductivity of the fluid
	\param di Inner diameter of the pipe
*/
double SurfaceTransmission(double nusselt, double lambda, double di);

/*! Class which can deliver fluid properties for a Water-Glycol mixture depending on temperature.*/
class WaterMixProperties {
public:
	/*! Which kind of Glycol is used.
		In case of adding new kinds the parameter array in the cpp must be changed!
	*/
	enum GlycolKind {
		GK_Ethylene,	///< Ethylene glycol
		GK_Propylene,	///< Propylene glycol
		GK_Count
	};

	/*! Constructor with fixed values for Glycol kind and amount.
	 *  \param glycolContent Glycol content in volume fraction (0.2 means 20Vol%).
	 *  \param glycolKind Kind of the used glycol
	*/
	WaterMixProperties(double glycolContent = 0, GlycolKind glycolKind = GK_Ethylene);

	/*! Density of the mixture in kg/m3 depending on given temperature.
		\param temperature Temperature in °C.
	*/
	double density(double temperature);

	/*! Dynamic viscosity of the mixture in Pa*s depending on given temperature.
		\param temperature Temperature in °C.
	*/
	double dynViscosity(double temperature);

	/*! Kinematic viscosity of the mixture in m2/s depending on given temperature.
		\param temperature Temperature in °C.
	*/
	double kinViscosity(double temperature);

	/*! Thermal conductivity of the mixture in W/mK depending on given temperature.
		\param temperature Temperature in °C.
	*/
	double lambda(double temperature);

	/*! Prandtl number of the mixture depending on given temperature.
		\param temperature Temperature in °C.
		Is calculated from dynVis*cp/lambda
	*/
	double prandtl(double temperature);

	/*! Specific heat capacity of the mixture in J/kgK depending on given temperature.
		\param temperature Temperature in °C.
	*/
	double heatCapacity(double temperature);

	/*! Freezing temperature in °C for the given mixture.*/
	double freezeTemperature();

private:
	double								m_glycolContent;		///< Glycol content as volume fraction
	GlycolKind							m_glycolKind;			///< Glycol kind (Ethylene or Propylene)
	std::array<LinearSpline, GK_Count>	m_freezeTemperature;	///< Vector of freezing temperature
	std::array<MultiSpline, GK_Count>	m_densities;			///< Densities in kg/m3
	std::array<MultiSpline, GK_Count>	m_dynViscosity;			///< Dynamic viscosity in Pa*s
	std::array<MultiSpline, GK_Count>	m_kinViscosity;			///< Kinematic viscosity in m2/s
	std::array<MultiSpline, GK_Count>	m_lambda;				///< Thermal conductivity in W/mK
	std::array<MultiSpline, GK_Count>	m_cp;					///< Specific heat capacity in J/kgK
};


} // namespace IBK

#endif // IBK_FluidPhysicsH
