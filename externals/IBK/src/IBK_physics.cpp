#include "IBK_physics.h"

namespace IBK {

const double PI = 3.141592653589793238;
const double BOLTZMANN = 5.67e-08;
const double FARADAY = 96485.3415;
const double R_IDEAL_GAS = 8.314472;
const double DEG2RAD = 0.01745329252;
const double R_VAPOR = 461.89;
const double R_AIR = 287.1;

const double RHO_W = 1000;
const double RHO_AIR = 1.205;
const double RHO_ICE = 916.7;
const double T_DEFAULT = 293.15;
const double T_REF_23 = 296.15;
const double C_WATER = 4.18e3;
const double C_ICE = 2.108e3;
const double C_VAPOR = 2.05e3;
const double C_AIR = 1.006e3;
const double H_EVAP = 3.08e6;
const double H_FREEZE = -2.46e5;
const double KELVIN_FACTOR = 1.0/(1000.0 * 462.0 * T_DEFAULT);
const double GASPRESS_REF = 101325;
const double GRAVITY = 9.807;
const double MIN_RH = 1e-10;
const double MIN_PC_T = -23.02585093 * RHO_W * R_VAPOR;
const double DV_AIR = 2.662e-5;
const double SIGMA_W = 7.6E-2;

/*! barrier constants */
const double MAX_RESISTANCE = 1e20;


} //namespace IBK
