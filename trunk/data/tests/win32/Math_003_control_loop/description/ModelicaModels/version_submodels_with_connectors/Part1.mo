within ;
model Part1

  annotation (uses(Modelica(version="3.0")), Icon(coordinateSystem(
          preserveAspectRatio=true, extent={{-100,-100},{100,100}}), graphics={
          Rectangle(extent={{-80,80},{80,-80}}, lineColor={0,0,255}), Text(
          extent={{-50,56},{54,-50}},
          lineColor={0,0,255},
          fillColor={255,0,255},
          fillPattern=FillPattern.Solid,
          textString="Part1")}),
    Diagram(coordinateSystem(preserveAspectRatio=false, extent={{-100,-100},{
            100,100}}), graphics));
  conout out_x2 annotation (Placement(transformation(extent={{72,-10},{92,10}}),
        iconTransformation(extent={{30,-94},{50,-74}})));
  conout out_x1 annotation (Placement(transformation(extent={{72,32},{92,52}}),
        iconTransformation(extent={{-48,-94},{-28,-74}})));
equation

  out_x1.value = if ((time < 1) or (time < 5 and time > 2)) then 0 else 1;
  out_x2.value = if ((time < 3) or (time < 6 and time > 4)) then 0 else 1;

  annotation (Line(
      points={{78,-64},{36,-64},{36,-52},{78,-52},{78,-64}},
      color={0,0,255},
      smooth=Smooth.None));
end Part1;
