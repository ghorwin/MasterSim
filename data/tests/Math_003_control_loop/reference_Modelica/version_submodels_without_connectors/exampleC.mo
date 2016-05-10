within ;
model exampleC
    Real x1=B1.x1;
    Real x2=B1.x2;
    Real x3=B3.x3;
    Real x4=B3.x4;
  Part1 B1;
  Part2 B2;
  Part3 B3;
equation
  B1.x1 = B2.x1;
  B2.x3 = B3.x3;
  B3.x4 = B2.x4;
  B1.x2 = B2.x2;

  annotation (uses(Modelica(version="3.2.1")),
    Icon(coordinateSystem(preserveAspectRatio=true,  extent={{-100,-100},{200,
            100}})),
    Diagram(coordinateSystem(preserveAspectRatio=false,extent={{-100,-100},{200,
            100}}), graphics),
    experiment(StopTime=10),
    experimentSetupOutput);
end exampleC;
