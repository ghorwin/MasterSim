model RoomModel
  parameter Real V=150 "Room air volume in m3";
  parameter Real c=1006 "Specific heat capacity of room air in J/kgK";
  parameter Real CR=400000 "Heat capacity of room other masses in J/K";
  parameter Real rho=1.2 "Air mass density in kg/m3";
  input Real n(start=0.2) "Air change range in 1/s";
  input Real Te(start=273.15) "Ambient temperature in K";
  input Real Qdot(start=150) "Heating load in W";
  output Real T(start=293.15) "Room air temperature in K";
  Real u "Energy density in J/m3";
equation
  u = (V*c*rho+CR)*T "Energy density function";
  der(u) = n*V*rho*(Te-T) + Qdot "Energy balance equation";
end RoomModel;