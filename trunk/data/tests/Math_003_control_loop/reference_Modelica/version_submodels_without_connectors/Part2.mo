within ;
model Part2
  input Real  x1;
  input Real  x2;
  input Real  x4;
  output Real  x3;
 annotation (Placement(transformation(extent={{72,-10},{92,10}}),
        iconTransformation(extent={{74,-10},{94,10}})));

initial equation
  x4 = 0;
  
equation

 x3 = if noEvent(x1 > 0 and x2 <= 0.01  and x4 < 2.5) then 3
 else if noEvent(x1 <= 0.001 and x2 > 0 and x4 > -2.5) then -3 else 0;

  annotation (uses(Modelica(version="3.2.1")),
                                             Icon(coordinateSystem(
          preserveAspectRatio=true, extent={{-100,-100},{100,100}})),
    Diagram(coordinateSystem(preserveAspectRatio=true,  extent={{-100,-100},{
            100,100}}), graphics));
end Part2;
