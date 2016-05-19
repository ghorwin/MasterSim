within ;
model Part2
    Real x2;
  annotation (uses(Modelica(version="3.0")), Icon(coordinateSystem(
          preserveAspectRatio=true, extent={{-100,-100},{100,100}}), graphics={
          Rectangle(extent={{-80,80},{80,-80}}, lineColor={0,0,255}), Text(
          extent={{-52,54},{52,-52}},
          lineColor={0,0,255},
          fillColor={255,0,255},
          fillPattern=FillPattern.Solid,
          textString="Part2")}),
    Diagram(coordinateSystem(preserveAspectRatio=true,  extent={{-100,-100},{
            100,100}}), graphics));
  conin in_x1    annotation (Placement(transformation(extent={{76,52},{96,72}}),
        iconTransformation(
        extent={{-10,-10},{10,10}},
        rotation=270,
        origin={-60,90})));
  conout out_x3  annotation (Placement(transformation(extent={{72,-10},{92,10}}),
        iconTransformation(extent={{74,-10},{94,10}})));
equation
	1 = in_x1 + x2 + out_x3;
	0 = x2 - out_x3;

end Part2;
