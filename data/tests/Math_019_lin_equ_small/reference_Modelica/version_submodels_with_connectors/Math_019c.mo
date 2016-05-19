within ;
model Math_019c
  annotation (uses(Modelica(version="3.0")),
    Icon(coordinateSystem(preserveAspectRatio=true,  extent={{-100,-100},{200,
            100}}), graphics={Text(
          extent={{-100,40},{100,-20}},
          lineColor={0,0,255},
          fillColor={255,0,255},
          fillPattern=FillPattern.Solid,
          textString="exampleTemplate")}),
    Diagram(coordinateSystem(preserveAspectRatio=false,extent={{-100,-100},{200,
            100}}), graphics),
    experiment(StopTime=1),
    experimentSetupOutput);
  Part1 B1 annotation (Placement(transformation(extent={{-46,26},{14,86}})));
  Part2 B2 annotation (Placement(transformation(extent={{-46,-66},{14,-6}})));
end Math_019c;
