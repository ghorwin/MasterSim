model LotkaVolterra
  parameter Real A=0.1 "Reproduction rate of prey";
  parameter Real B=0.02 "Mortality rate of predator per prey";
  parameter Real C=0.4 "Mortality rate of predator";
  parameter Real D=0.02 "Reproduction rate of predator per prey";
  parameter Real x0=10 "Initial prey population";
  parameter Real y0=10 "Initial predator population";
  Real x(start=x0) "Prey population";
  Real y(start=y0) "Predator population";
initial equation
  x = x0;
  y = y0;
equation
  der(x) = x*(A-B*y);
  der(y) = y*(D*x-C);
end LotkaVolterra;
