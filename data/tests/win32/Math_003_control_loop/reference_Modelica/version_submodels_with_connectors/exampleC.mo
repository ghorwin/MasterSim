within ;
model exampleC
    Real x1=B1.out_x1.value;
    Real x2=B1.out_x2.value;
    Real x3=B3.in_x3.value;
    Real x4=B3.out_x4.value;
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
    experiment(StopTime=10),
    experimentSetupOutput);
  Part1 B1 annotation (Placement(transformation(extent={{-46,26},{14,86}})));
  Part2 B2 annotation (Placement(transformation(extent={{-46,-66},{14,-6}})));
  Part3 B3 annotation (Placement(transformation(extent={{52,-28},{134,54}})));
equation
  connect(B1.out_x1, B2.in_x1) annotation (Line(
      points={{-27.4,30.8},{-27.4,12.4},{-34,12.4},{-34,-9}},
      color={0,0,255},
      smooth=Smooth.None));
  connect(B2.out_x3, B3.in_x3) annotation (Line(
      points={{9.2,-36},{30,-36},{30,13},{56.1,13}},
      color={0,0,255},
      smooth=Smooth.None));
  connect(B3.out_x4, B2.in_x4) annotation (Line(
      points={{76.6,-21.44},{76.6,-80},{-4.6,-80},{-4.6,-63}},
      color={0,0,255},
      smooth=Smooth.None));
  connect(B1.out_x2, B2.in_x2) annotation (Line(
      points={{-4,30.8},{-4,12},{2,12},{2,-9}},
      color={0,0,255},
      smooth=Smooth.None));
end exampleC;
