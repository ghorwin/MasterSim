within ;
model Part1

  output Real x2;
  output Real x1;
equation

  x1 = if ((time < 1) or (time < 5 and time > 2)) then 0 else 1;
  x2 = if ((time < 3) or (time < 6 and time > 4)) then 0 else 1;

  annotation (uses(Modelica(version="3.2.1")),
                                             Icon(coordinateSystem(
          preserveAspectRatio=true, extent={{-100,-100},{100,100}})),
    Diagram(coordinateSystem(preserveAspectRatio=false, extent={{-100,-100},{
            100,100}}), graphics),
              Line(
      points={{78,-64},{36,-64},{36,-52},{78,-52},{78,-64}},
      color={0,0,255},
      smooth=Smooth.None));
end Part1;
