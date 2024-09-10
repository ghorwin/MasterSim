model MoistureCtrl
  Modelica.Blocks.Interfaces.RealInput phi annotation(
    Placement(transformation(origin = {-104, 60}, extent = {{-20, -20}, {20, 20}}), iconTransformation(origin = {-100, 72}, extent = {{-20, -20}, {20, 20}})));
  Modelica.Blocks.Interfaces.RealOutput mDotHumidity annotation(
    Placement(transformation(origin = {106, 76}, extent = {{-10, -10}, {10, 10}}), iconTransformation(origin = {100, 76}, extent = {{-10, -10}, {10, 10}})));
  Modelica.Blocks.Interfaces.RealInput phiSet annotation(
    Placement(transformation(origin = {-104, 20}, extent = {{-20, -20}, {20, 20}}), iconTransformation(origin = {-100, 72}, extent = {{-20, -20}, {20, 20}})));
  Modelica.Blocks.Interfaces.RealInput phiPositive annotation(
    Placement(transformation(origin = {-104, -14}, extent = {{-20, -20}, {20, 20}}), iconTransformation(origin = {-100, 72}, extent = {{-20, -20}, {20, 20}})));
  Modelica.Blocks.Interfaces.RealInput phiNegativ annotation(
    Placement(transformation(origin = {-104, -46}, extent = {{-20, -20}, {20, 20}}), iconTransformation(origin = {-100, 72}, extent = {{-20, -20}, {20, 20}})));
  Modelica.Blocks.Interfaces.RealInput mDotPositiv annotation(
    Placement(transformation(origin = {-104, -78}, extent = {{-20, -20}, {20, 20}}), iconTransformation(origin = {-100, 72}, extent = {{-20, -20}, {20, 20}})));
  Modelica.Blocks.Interfaces.RealInput mDotNegativ annotation(
    Placement(transformation(origin = {-104, -104}, extent = {{-20, -20}, {20, 20}}), iconTransformation(origin = {-100, 72}, extent = {{-20, -20}, {20, 20}})));

equation

  if (phi > phiSet + phiPositive) then
    mDotHumidity = mDotPositiv;
  elseif (phi < phiSet - phiNegativ) then
    mDotHumidity = mDotNegativ;
  else
    mDotHumidity = 0;
  end if

annotation(
    uses(Modelica(version = "4.0.0")));
end MoistureCtrl;