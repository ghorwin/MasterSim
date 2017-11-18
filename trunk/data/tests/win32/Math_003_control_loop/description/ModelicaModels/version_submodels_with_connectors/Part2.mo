within ;
model Part2
    Real x1=in_x1.value;
    Real x2=in_x2.value;
    Real x4=in_x4.value;
    Real x3=out_x3.value;
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
  conin in_x2    annotation (Placement(transformation(extent={{40,78},{60,98}}),
        iconTransformation(extent={{-10,-10},{10,10}},
        rotation=270,
        origin={60,90})));
  conin in_x4    annotation (Placement(transformation(extent={{-94,-50},{-74,-30}}),
        iconTransformation(extent={{-10,-10},{10,10}},
        rotation=90,
        origin={38,-90})));
  conin in_x1    annotation (Placement(transformation(extent={{76,52},{96,72}}),
        iconTransformation(
        extent={{-10,-10},{10,10}},
        rotation=270,
        origin={-60,90})));
  conout out_x3  annotation (Placement(transformation(extent={{72,-10},{92,10}}),
        iconTransformation(extent={{74,-10},{94,10}})));
equation

  x3 = if noEvent(x1 > 0 and x2 <= 0.01  and x4 < 2.5) then 3
  else if noEvent(x1 <= 0.001 and x2 > 0 and x4 > -2.5) then -3 else 0;

end Part2;
